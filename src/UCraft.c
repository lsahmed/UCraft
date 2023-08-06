
#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "wrapper.h"
#include "c2s.h"
#include "config.h"
#include "player.h"
#include "s2c.h"
#include "socketio.h"
#include "util.h"
#include "log.h"
#include "enums.h"
#include "encryption.h"
#include "https.h"

size_t main_tick;

static void c2sHandler(readPacketVars_t *readPacketValue)
{
    player_t *currentPlayer = readPacketValue->player;
    while (readAllowed())
    {
        if (currentPlayer->remove_player)
        {
            return;
        }
        readPacketValue->pktbytes = -1;
        readPacketValue->pktbytes = readVarInt();
        if (readPacketValue->pktbytes > readPacketValue->buffersize || readPacketValue->pktbytes > READBUFSIZE)
        {
            printl(LOG_WARN, "c2sHandler overflow! %ld/%ld\n", readPacketValue->pktbytes, readPacketValue->buffersize);
            currentPlayer->remove_player = 1;
            return;
        }
        if (currentPlayer->compression_event)
        {
            uint8_t compsize = readVarInt();
            if (compsize > 0)
            {
                printl(LOG_WARN, "UNIMPLEMENTED compressed packet size:%d\n", compsize);
                if (readPacketValue->pktbytes)
                {
                    while (readPacketValue->pktbytes)
                    {
                        // printf("%02x ", readByte());
                        readByte();
                    }
                }
                continue;
            }
        }

        uint8_t cmd = readByte();
        switch (currentPlayer->handshake_status)
        {
        case 0:
            if (cmd == 0)
            {
                int32_t protocol_version = readVarInt();
                if (protocol_version != PROTOCOL_VERSION && protocol_version != -1)
                {
                    printl(LOG_WARN, "client not supported! version:%d\n", protocol_version);
                    currentPlayer->remove_player = 1;
                }
                uint8_t buf[255];
                readString((char *)buf, 255);
                readShort();
                currentPlayer->handshake_status = readVarInt();
            }
            break;
        case 1: // status state
            switch (cmd)
            {
            case 0x00: // Request
                currentPlayer->playerlist_event = 1;
                break;
            case 0x01:
                char scratch[8]; // Ping
                readBuffer(scratch, 8);
                currentPlayer->ping_event = 1;
                break;
            default:
                break;
            }
            break;
        case 2: // login state
            switch (cmd)
            {
            case 0:
                if (playerGetActiveCount() >= MAX_PLAYERS)
                {
                    currentPlayer->remove_player = 1;
                    continue;
                }
                readString((char *)currentPlayer->playername, sizeof(((player_t *)0)->playername));
                // check the player name
                if (playerCheckName(currentPlayer))
                {
                    strncpy((char *)currentPlayer->playername, "stinky_player", sizeof(((player_t *)0)->playername));
                    currentPlayer->remove_player = 1;
                    continue;
                }
                // check if any other players exist with the same name
                if (playerCheckDuplicate(currentPlayer))
                {
                    strncpy((char *)currentPlayer->playername, "stinky_player", sizeof(((player_t *)0)->playername));
                    currentPlayer->remove_player = 1;
                    continue;
                }
                printl(LOG_INFO, "Player login name: %s\n", currentPlayer->playername);
#ifdef ONLINE_MODE
                currentPlayer->encryption_event = 1;
#else
                currentPlayer->login_event = 1;
#endif /*ONLINE_MODE*/
                break;
            case 1: // encryption response
#ifdef ONLINE_MODE
                LoginC2S_encryption_response();
#ifdef ONLINE_MODE_AUTH
                if (currentPlayer->encryption_verified)
                {
                    if (httpsGetData()->currentPlayer == NULL)
                    {
                        httpsGetPlayerInfo(currentPlayer);
                    }
                    else
                    {
                        currentPlayer->https_wait_event = 1;
                    }
                }
#else
                currentPlayer->login_event = 1;
#endif /*ONLINE_MODE_AUTH*/
                currentPlayer->encryption_recv_event = 1;
#endif /*ONLINE_MODE*/
                break;
            default:
                break;
            }
            break;
        case 3: // play state
            if (cmd < PLAYC2S_MAPPING_LEN)
            {
                void (**packet_handler)(player_t *) = c2s_1_19_4_1_19_4;
                if (packet_handler[cmd] != NULL)
                {
                    if (*packet_handler[cmd] != NULL)
                    {
                        (*packet_handler[cmd])(currentPlayer);
                    }
                }
            }
            break;
        default:
            // invalid state
            printl(LOG_WARN, "Player(%d) invalid state! %d\n", currentPlayer->player_id, currentPlayer->handshake_status);
            currentPlayer->remove_player = 1;
            break;
        }
        // empty the packet buffer
        if (readPacketValue->pktbytes)
        {
            //  printl(LOG_INFO,"discarding packet %d with %d bytes remaining\n", cmd,
            // readPacketValue->pktbytes);
            while (readPacketValue->pktbytes)
            {
                // printf("%02x ", readByte());
                readByte();
            }
        }
    }
}

static void s2cHandler()
{
    for (player_t *currentPlayer = playerGetHead(); currentPlayer != NULL; currentPlayer = currentPlayer->next)
    {
        if (currentPlayer == NULL)
        {
            break;
        }
        sendStartPlayer(currentPlayer);
        if (currentPlayer->ping_event)
        {
            StatusS2Cpong(currentPlayer);
            currentPlayer->remove_player = 1;
            currentPlayer->ping_event = 0;
        }
        if (currentPlayer->playerlist_event)
        {
            StatusS2Cresponse(currentPlayer);
            currentPlayer->playerlist_event = 0;
        }
#ifdef ONLINE_MODE_AUTH
        if (currentPlayer->https_wait_event)
        {
            if (httpsGetData()->currentPlayer == NULL)
            {
                httpsGetPlayerInfo(currentPlayer);
                currentPlayer->https_wait_event = 0;
            }
        }
        if (currentPlayer->https_rtr_event)
        {
            currentPlayer->https_rtr_event = httpsRtr(currentPlayer);
        }
        if (currentPlayer->https_rts_event)
        {
            currentPlayer->https_rts_event = httpsRts(currentPlayer);
        }
#endif /*ONLINE_MODE_AUTH*/
        if (currentPlayer->spawn_event)
        {
            PlayS2Cjoingame(currentPlayer);
            PlayS2Cheartbeat(currentPlayer);
            PlayS2Ctablist(currentPlayer, TABLIST_ACTION_ADDPLAYER | TABLIST_ACTION_LISTED | TABLIST_ACTION_LATENCY, currentPlayer->player_id);
            PlayS2Centitydata(currentPlayer, PLAYER_SKIN_PARTS_FLAGS, ENTITY_DATA_BYTE, currentPlayer->skin_parts); // enable from cape to hat

            // show player to other clients
            for (player_t *p = playerGetHead(); p != NULL; p = p->next)
            {
                if (p != currentPlayer && p->active)
                {
                    PlayS2Ctablist(p, TABLIST_ACTION_ADDPLAYER | TABLIST_ACTION_LISTED, p->player_id);
                    PlayS2Centityplayer(p);
                    PlayS2Centitydata(p, PLAYER_SKIN_PARTS_FLAGS, ENTITY_DATA_BYTE, p->skin_parts); // enable from cape to hat
                }
            }
            // set the minimum chunk coordinates
            currentPlayer->chunk_x = -CHUNK_SIZE;
            currentPlayer->chunk_z = -CHUNK_SIZE;

            currentPlayer->chunk_next_event = 1;
            currentPlayer->logged_on = 1;
            currentPlayer->spawn_event = 0;
        }
        if (currentPlayer->chunk_next_event)
        {
            // dont send multiple chunks at the same time since the client will reject it
            // TODO: add compression
            PlayS2Cchunk(currentPlayer, currentPlayer->chunk_x, currentPlayer->chunk_z);
            if (currentPlayer->chunk_x < CHUNK_SIZE)
            {
                currentPlayer->chunk_x++;
            }
            else
            {
                currentPlayer->chunk_x = -CHUNK_SIZE;
                currentPlayer->chunk_z++;
            }
            if (currentPlayer->chunk_z > CHUNK_SIZE)
            {
                currentPlayer->chunk_x = 0;
                currentPlayer->chunk_z = 0;
                currentPlayer->global_buffer_start_index = sendGetGlobalBufferIndex();
                currentPlayer->chunk_next_event = 0;
                currentPlayer->chunk_loaded_event = 1;
                currentPlayer->ingame = 1;
            }
        }
#ifdef ONLINE_MODE
        if (currentPlayer->encryption_event)
        {
            LoginS2Cencryptionrequest(currentPlayer);
            currentPlayer->encryption_event = 0;
        }
#endif /*ONLINE_MODE*/
        if (currentPlayer->login_event)
        {
#ifdef COMPRESSION
            LoginS2Ccompression(currentPlayer);
#endif /*COMPRESSION*/
            LoginS2Csuccess(currentPlayer);
#ifdef ONLINE_MODE_AUTH
            httpsFreePlayer(currentPlayer);
#endif /*ONLINE_MODE_AUTH*/
            currentPlayer->active = 1;
            currentPlayer->spawn_event = 1;
            currentPlayer->login_event = 0;
            currentPlayer->handshake_status = 3;
        }
        if (currentPlayer->ingame && currentPlayer->active)
        {
            // printl(LOG_INFO,"player %d ingame\n",currentPlayer->player_id);
            sendRevertFromGlobalBuffer();
            if ((main_tick % 500) == 0)
            {
                if (!currentPlayer->heartbeat)
                {
                    PlayS2Ckick(currentPlayer, "{\"text\":\"Failed heartbeat! {try not doing that}\"}");
                    printl(LOG_INFO, "Failed heartbeat for %s\n", currentPlayer->playername);
                    currentPlayer->remove_player = 1;
                }
                PlayS2Cheartbeat(currentPlayer);
                currentPlayer->heartbeat = 0;
            }
            if (currentPlayer->chunk_loaded_event)
            {
                PlayS2Cpositionrotation(currentPlayer, SPAWN_X, SPAWN_Y, SPAWN_Z);
                PlayS2Ccompassposition(currentPlayer, SPAWN_X, SPAWN_Y, SPAWN_Z);
                gameChunkLoaded(currentPlayer);
                currentPlayer->chunk_loaded_event = 0;
            }
            // check if the conditions for a chunk update are met
            if (currentPlayer->chunk_x != currentPlayer->chunk_px || currentPlayer->chunk_z != currentPlayer->chunk_pz)
            {
                PlayS2Cchunkcenter(currentPlayer, currentPlayer->chunk_x, currentPlayer->chunk_z);
                currentPlayer->chunk_px = currentPlayer->chunk_x;
                currentPlayer->chunk_pz = currentPlayer->chunk_z;
            }
            if (currentPlayer->teleport)
            {
                // not a huge fan of this since the client does not respect the teleport packet in the local context
                PlayS2Cpositionrotation(currentPlayer, currentPlayer->x, currentPlayer->y, currentPlayer->z);
                currentPlayer->teleport = 0;
            }
            if (currentPlayer->ability)
            {
                if (currentPlayer->ability & ABILILTIES_CLEAR)
                {
                    PlayS2Cplayerabilities(currentPlayer, 0);
                }
                else
                {
                    PlayS2Cplayerabilities(currentPlayer, currentPlayer->ability & 0xf);
                }
                currentPlayer->ability = 0;
            }
            gamePlayerLocalTick(currentPlayer);
            sendGlobalBuffer(currentPlayer);
        }
        if (currentPlayer->packet_dispatch_flag)
        {
            if (currentPlayer->packet_timeout > SEND_PACKET_TIMEOUT)
            {
                currentPlayer->remove_player = 1;
            }
            else
            {
                size_t remaining = currentPlayer->packet_len - currentPlayer->packet_sent;
                if (remaining == 0)
                {
                    if (currentPlayer->packet == NULL)
                    {
                        printl(LOG_ERROR, "WTF, idk why this is null\n");
                        currentPlayer->remove_player = 1;
                        return;
                    }
                    U_free(currentPlayer->packet);
                    currentPlayer->packet_len = 0;
                    currentPlayer->packet_sent = 0;
                    currentPlayer->packet = NULL;
                    currentPlayer->packet_dispatch_flag = 0;
                }
                else
                {
                    size_t totalSent = sendData(&currentPlayer->packet[currentPlayer->packet_sent], remaining);
                    currentPlayer->packet_sent += totalSent;
                }
            }
            currentPlayer->packet_timeout++;
        }
        else
        {
            currentPlayer->packet_timeout = 0;
            sendDispatch();
        }
        currentPlayer->global_buffer_start_index = 0;
    }
    sendclearGlobalBuffer();

    for (player_t *currentPlayer = playerGetHead(); currentPlayer != NULL; currentPlayer = currentPlayer->next)
    {
        if (currentPlayer->ingame)
        {
            sendStartPlayer(currentPlayer);
            // Global context, send to all clients
            sendSwitchToGlobalBuffer();
            // add current player to the the tablist as well
            if (currentPlayer->logged_on)
            {
                PlayS2Ctablist(currentPlayer, TABLIST_ACTION_ADDPLAYER | TABLIST_ACTION_LISTED, currentPlayer->player_id);
                PlayS2Centityplayer(currentPlayer);
                PlayS2Centitydata(currentPlayer, PLAYER_SKIN_PARTS_FLAGS, ENTITY_DATA_BYTE, currentPlayer->skin_parts); // enable from cape to hat
                currentPlayer->global_buffer_start_index = sendGetGlobalBufferIndex();
                currentPlayer->send_chat_login_event = 1;
                currentPlayer->logged_on = 0;
            }
            if (currentPlayer->position_event)
            {
                if (currentPlayer->yaw > 180)
                {
                    currentPlayer->yaw -= 360;
                }
                else if (currentPlayer->yaw < -180)
                {
                    currentPlayer->yaw += 360;
                }
                currentPlayer->nyaw = (uint8_t)((int8_t)(currentPlayer->yaw * (float)(256.00F / 360.00F)));
                currentPlayer->npitch = (uint8_t)((int8_t)(currentPlayer->pitch * (float)(256.00F / 360.00F)));
                // check if the player has moved since the last packet
                if (currentPlayer->x != currentPlayer->px || currentPlayer->y != currentPlayer->py || currentPlayer->z != currentPlayer->pz)
                {
                    PlayS2Cteleport(currentPlayer, currentPlayer->x, currentPlayer->y, currentPlayer->z);
                    PlayS2Cheadrotation(currentPlayer);
                    // set the chunk that the player is in
                    currentPlayer->chunk_x = (int32_t)floor(currentPlayer->x / 16.00);
                    currentPlayer->chunk_z = (int32_t)floor(currentPlayer->z / 16.00);
                    // set the previous value to current
                    currentPlayer->px = currentPlayer->x;
                    currentPlayer->py = currentPlayer->y;
                    currentPlayer->pz = currentPlayer->z;
                    currentPlayer->pyaw = currentPlayer->yaw;
                    currentPlayer->ppitch = currentPlayer->pitch;
                }
                if (currentPlayer->pyaw != currentPlayer->yaw || currentPlayer->pitch != currentPlayer->ppitch)
                {
                    PlayS2Crotation(currentPlayer);
                    PlayS2Cheadrotation(currentPlayer);
                    // set the previous value to current
                    currentPlayer->pyaw = currentPlayer->yaw;
                    currentPlayer->pitch = currentPlayer->ppitch;
                }
                currentPlayer->global_buffer_start_index = sendGetGlobalBufferIndex();
                currentPlayer->position_event = 0;
            }
            if (currentPlayer->swing_arm_event)
            {
                switch (currentPlayer->swing_arm_animation)
                {
                case 0: // main hand
                    PlayS2Centityanimation(currentPlayer, 0);
                    break;
                case 1: // off hand
                    PlayS2Centityanimation(currentPlayer, 3);
                    break;
                default:
                    break;
                }
                currentPlayer->global_buffer_start_index = sendGetGlobalBufferIndex();
                currentPlayer->swing_arm_event = 0;
            }
            if (currentPlayer->entity_action_event)
            {
                switch (currentPlayer->entity_action_id)
                {
                case 0: // start sneaking
                    PlayS2Centitydata(currentPlayer, ENTITY_POSE, ENTITY_DATA_POSE, STATE_SNEAKING);
                    break;
                case 1: // normal standing
                    PlayS2Centitydata(currentPlayer, ENTITY_POSE, ENTITY_DATA_POSE, STATE_STANDING);
                default:
                    break;
                }
                currentPlayer->global_buffer_start_index = sendGetGlobalBufferIndex();
                currentPlayer->entity_action_event = 0;
            }
            if (currentPlayer->chat_event)
            {
                if (currentPlayer->chat_ptr)
                {
                    if (chat_inuse)
                    {
                        PlayS2Cunsignedchatmessage(currentPlayer->chat_ptr, currentPlayer->chat_len);
                        printl(LOG_INFO, "%s\n", currentPlayer->chat_ptr);
                    }
                    chat_inuse = 0;
                }
                currentPlayer->chat_event = 0;
            }
            if (currentPlayer->send_chat_login_event)
            {
                char join_msg[64];
                snprintf(join_msg, sizeof(join_msg), "\\u00A7e%s has joined the game", currentPlayer->playername);
                PlayS2Cunsignedchatmessage(join_msg, strnlen(join_msg, sizeof(join_msg)));
                currentPlayer->send_chat_login_event = 0;
            }
            if (currentPlayer->settings_changed_event)
            {
                PlayS2Centitydata(currentPlayer, PLAYER_SKIN_PARTS_FLAGS, ENTITY_DATA_BYTE, currentPlayer->skin_parts);
                currentPlayer->settings_changed_event = 0;
            }
            gamePlayerGlobalTick(currentPlayer);
            sendRevertFromGlobalBuffer();
        }
    }
    sendSwitchToGlobalBuffer();
    // special case for player disconnections
    for (playerd_t *disconnected = playerPopDisconnected(); disconnected != NULL; disconnected = playerPopDisconnected())
    {
        if (playerGetActiveCount() > 0)
        {
            char left_msg[64];
            snprintf(left_msg, sizeof(left_msg), "\\u00A7e%s has left the game", disconnected->name);
            PlayS2Cunsignedchatmessage(left_msg, strnlen(left_msg, sizeof(left_msg)));
            PlayS2Ctablistremove(disconnected->id);
            PlayS2Centitydestroy(disconnected->id);
        }
        U_free(disconnected->name);
        U_free(disconnected);
    }
    gameGlobalTick();
    sendRevertFromGlobalBuffer();
}
int UCraftStart(uint8_t *cleanup_flag)
{
    if (cleanup_flag == NULL)
    {
        printl(LOG_ERROR, "Cleanup flag is NULL\n");
        return 0;
    }
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    fd_set set;
    struct timeval timeout;
    int rv;
    int max_sock = 0;
#ifdef ONLINE_MODE
    if (encryptionBegin())
    {
        printl(LOG_ERROR, "Failed to begin encryption 'module'\n");
        encryptionCleanup();
        return 1;
    }
#endif /*ONLINE_MODE*/
    if ((server_fd = U_socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printl(LOG_ERROR, "Cannot create socket fd:%d\n", server_fd);
        return 1;
    }
    if (U_setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printl(LOG_ERROR, "setsockopt error\n");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (U_bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printl(LOG_ERROR, "bind error");
        return 1;
    }
    if (U_listen(server_fd, MAX_PLAYERS + 1) < 0)
    {
        printl(LOG_ERROR, "listen error");
        return 1;
    }
    printl(LOG_INFO, "Listening on *:%d\n", PORT);
    FD_ZERO(playerGetSet());
    FD_SET(server_fd, playerGetSet());
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    max_sock = server_fd;
    printl(LOG_INFO, "UCraft server started!\n");
    gamePreload();
    while (1)
    {
        if (*cleanup_flag)
        {
            printl(LOG_INFO, "Cleaning up!\n");
            break;
        }
        memcpy(&set, playerGetSet(), sizeof(fd_set));
        rv = U_select(max_sock + 1, &set, NULL, NULL, &timeout);
        if (rv == -1)
        {
            printl(LOG_ERROR, "select error");
            return 1;
        }
        else
        {
            if (FD_ISSET(server_fd, &set))
            {
                int new_socket = U_accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                //set the socket non blocking
                if(U_fctl(new_socket, F_SETFL, U_fctl(new_socket, F_GETFL, 0) | O_NONBLOCK)){
                    printl(LOG_ERROR, "Failed to set socket non blocking\n");
                    return 1;
                }
                if (new_socket > max_sock)
                {
                    max_sock = new_socket;
                }
                if (!playerAdd(new_socket))
                {
                    printl(LOG_WARN, "player not added\n");
                    FD_CLR(new_socket, playerGetSet());
                    U_shutdown(new_socket, SHUT_RDWR);
                    U_close(new_socket);
                    continue;
                }
                struct sockaddr_in addr;
                socklen_t addr_size = sizeof(addr);
                U_getpeername(new_socket, (struct sockaddr *)&addr, (socklen_t *)&addr_size);
                printl(LOG_INFO, "Incomming connection from IP: %s\n", U_inet_ntoa(addr.sin_addr));
            }
            for (player_t *player = playerGetHead(); player != NULL;)
            {
                if (player->remove_player)
                {
                    // printl(LOG_INFO, "removing player due to flag\n");
                    playerRemove(player);
                    player = NULL;
                    if (playerGetHead() == NULL)
                    {
                        break;
                    }
                    continue;
                }
                if (FD_ISSET(player->player_fd, &set))
                {
                    readPacketVars_t *readPacketValue = readValues();
                    readPacketValue->bufferpos = 0;
                    ssize_t read_size = U_read(player->player_fd, readPacketValue->buffer, READBUFSIZE);
                    if (read_size <= 0 || player->player_fd < 0)
                    {
                        // printl(LOG_WARN,"Connection closed %d fd:%d\n", player->handshake_status,
                        //  player->player_fd);
                        playerRemove(player);
                        player = NULL;
                        if (playerGetHead() == NULL)
                        {
                            break;
                        }
                        continue;
                    }
                    if (read_size > 0 && read_size < READBUFSIZE)
                    {
                        readPacketValue->buffersize = read_size;
                    }
#ifdef ONLINE_MODE
                    if (player->encryption_recv_event)
                    {
                        int ret = mbedtls_aes_crypt_cfb8(&player->aes_ctx, MBEDTLS_AES_DECRYPT, readPacketValue->buffersize, player->iv_decrypt, readPacketValue->buffer, readPacketValue->buffer);
                        if (ret != 0)
                        {
                            printl(LOG_ERROR, "aes decrypt failed %d\n", ret);
                            player->remove_player = 1;
                            continue;
                        }
                    }
#endif /*ONLINE_MODE*/
                    readStart(player);
                    c2sHandler(readPacketValue);
                }
                player = player->next;
            }
        }
        s2cHandler();
        // TODO: this is not how you tick
        U_usleep(10000); // need to tick 20 times a second
        main_tick++;
    }
    // cleanup
    socketioCleanup();
#ifdef ONLINE_MODE
    encryptionCleanup();
#endif /*ONLINE_MODE*/
    playerCleanup();
#ifdef ONLINE_MODE_AUTH
    httpsCleanup();
#endif /*ONLINE_MODE_AUTH*/
    U_close(server_fd);
    return 0;
}