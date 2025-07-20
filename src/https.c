#include "https.h"
#ifdef ONLINE_MODE_AUTH
#include <errno.h>
#include "encryption.h"
#include "log.h"
#include "util.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha1.h"
#include "mbedtls/ssl.h"
#include "mbedtls/debug.h"
#include "lwjson/lwjson.h"
#include "wrapper.h"
static httpsData_t httpsData;
#ifdef MBEDTLS_DEBUG_C
static void sslDebug(void *ctx, int level, const char *file, int line, const char *str)
{
    printl(LOG_INFO, "%s:%04d: %s", file, line, str);
}
#endif /*MBEDTLS_DEBUG_C*/
//TODO: Add checks if the socket is nonblocking or not as the implementation for net_would_block is missing
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int ret;
    int fd = ((mbedtls_net_context *)ctx)->fd;
    ret = (int)U_recv(fd, buf, len, MSG_NOSIGNAL);
    if (ret < 0)
    {
        if (errno == EPIPE || errno == ECONNRESET)
        {
            return (MBEDTLS_ERR_NET_CONN_RESET);
        }

        if (errno == EINTR)
        {
            return (MBEDTLS_ERR_SSL_WANT_READ);
        }

        return (MBEDTLS_ERR_SSL_WANT_READ);
    }
    return ret;
}
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret;
    int fd = ((mbedtls_net_context *)ctx)->fd;
    ret = (int)U_send(fd, buf, len, MSG_NOSIGNAL);
    if (ret < 0)
    {
        if (errno == EPIPE || errno == ECONNRESET)
        {
            return (MBEDTLS_ERR_NET_CONN_RESET);
        }

        if (errno == EINTR)
        {
            return (MBEDTLS_ERR_SSL_WANT_WRITE);
        }
        return (MBEDTLS_ERR_SSL_WANT_WRITE);
    }
    return ret;
}

httpsData_t *httpsGetData()
{
    return &httpsData;
}
int httpsConnect(player_t *currentPlayer, const char *hostname, const char *port)
{
    if (currentPlayer == NULL)
    {
        printl(LOG_ERROR, "how??? currentPlayer is NULL\n");
        return 1;
    }
    if (httpsData.currentPlayer != NULL)
    {
        printl(LOG_ERROR, "in use\n");
        return 1;
    }
    httpsData.currentPlayer = currentPlayer;
    int ret;
    mbedtls_ssl_init(&httpsData.ssl);
    mbedtls_ssl_config_init(&httpsData.conf);
    // connect to the server manually
    httpsData.net.fd = U_socket(AF_INET, SOCK_STREAM, 0);
    if (httpsData.net.fd < 0)
    {
        printl(LOG_ERROR, "U_socket returned %d\n", httpsData.net.fd);
        return 1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    struct hostent *hostinfo = U_gethostbyname(hostname);
    if (!hostinfo)
    {
        printl(LOG_ERROR, "U_gethostbyname Failed\r\n");
        return 1;
    }
    server.sin_addr = *((struct in_addr *)hostinfo->h_addr);
    ret = U_connect(httpsData.net.fd, (struct sockaddr *)&server, sizeof(server));
    if (ret != 0)
    {
        printl(LOG_ERROR, "U_connect returned %d errno: %d\n", ret,errno);
        return 1;
    }
    ret = U_setsocknonblock(httpsData.net.fd);
    if (ret != 0)
    {
        printl(LOG_ERROR, "U_setsocknonblock returned %d\n", ret);
        return 1;
    }
    ret = mbedtls_ssl_config_defaults(&httpsData.conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0)
    {
        printl(LOG_ERROR, "mbedtls_ssl_config_defaults returned %d\n", ret);
        return 1;
    }
    // TODO:is a certificate required? ofc not bro we're just a client, jk will fix it later
    mbedtls_ssl_conf_authmode(&httpsData.conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&httpsData.conf, mbedtls_ctr_drbg_random, &encryptionGetData()->ctr_drbg);

    ret = mbedtls_ssl_setup(&httpsData.ssl, &httpsData.conf);
    if (ret != 0)
    {
        printl(LOG_ERROR, "mbedtls_ssl_setup returned %d\n", ret);
        return 1;
    }
    ret = mbedtls_ssl_set_hostname(&httpsData.ssl, hostname);
    if (ret != 0)
    {
        printl(LOG_ERROR, "mbedtls_ssl_set_hostname returned %d\n", ret);
        return 1;
    }
#ifdef MBEDTLS_DEBUG_C
    mbedtls_ssl_conf_dbg(&httpsData.conf, sslDebug, NULL);
    mbedtls_debug_set_threshold(4);
#endif /*MBEDTLS_DEBUG_C*/
    mbedtls_ssl_set_bio(&httpsData.ssl, &httpsData.net, mbedtls_net_send, mbedtls_net_recv, NULL);
    return 0;
}
void httpsGetPlayerInfo(player_t *currentPlayer)
{
    if (currentPlayer == NULL)
    {
        printl(LOG_ERROR, "how??? currentPlayer is NULL\n");
        return;
    }
    if (httpsData.currentPlayer != NULL)
    {
        printl(LOG_ERROR, "in use\n");
        return;
    }
    // generate the login hash
    uint8_t hash[20];
    uint8_t publickey[256];
    uint8_t server_hash[45];
    mbedtls_sha1_context sha1;
    mbedtls_sha1_init(&sha1);
    mbedtls_sha1_starts(&sha1);
    memset(hash, 0, sizeof(hash));
    mbedtls_sha1_update(&sha1, hash, 20);
    mbedtls_sha1_update(&sha1, currentPlayer->iv_encrypt, 16);
    int ret = mbedtls_pk_write_pubkey_der(&encryptionGetData()->key, publickey, sizeof(publickey));
    if (ret < 0)
    {
        printl(LOG_ERROR, "mbedtls_pk_write_pubkey_der returned %d\n", ret);
        currentPlayer->remove_player_event = 1;
        return;
    }
    mbedtls_sha1_update(&sha1, &publickey[sizeof(publickey) - ret], ret);
    mbedtls_sha1_finish(&sha1, hash);
    mbedtls_sha1_free(&sha1);
    encryptionHexDigest(server_hash, hash, sizeof(server_hash));
    ret = httpsConnect(currentPlayer, AUTH_HOST, AUTH_HOST_PORT);
    if (ret != 0)
    {
        printl(LOG_ERROR, "httpsConnect returned %d\n", ret);
        httpsFreePlayer(currentPlayer);
        currentPlayer->remove_player_event = 1;
        return;
    }
    // form the request
    httpsGetData()->len = snprintf(httpsGetData()->buffer, sizeof(((httpsData_t *)0)->buffer), "GET /session/minecraft/hasJoined?username=%s&serverId=%s HTTP/1.1\r\nHost: %s\r\nAccept: application/json\r\nContent-Type: application/json\r\n\r\n", currentPlayer->name, server_hash, AUTH_HOST);
    // set the rts flag and dispatch it whenever it can be sent
    currentPlayer->https_rts_event = 1;
}
int httpsRtr(player_t *currentPlayer)
{
    if (currentPlayer == NULL)
    {
        printl(LOG_ERROR, "how??? currentPlayer is NULL\n");
        return 0;
    }
    if (httpsData.currentPlayer != currentPlayer)
    {
        printl(LOG_ERROR, "how??? currentPlayer is not the same\n");
        return 0;
    }
    if (httpsData.timeout > AUTH_TIMEOUT)
    {
        printl(LOG_ERROR, "httpsRtr timeout\n");
        strncpy((char *)currentPlayer->disconnect_reason, "Authentication Timeout", sizeof(((player_t *)0)->disconnect_reason));
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    int ret = mbedtls_ssl_read(&httpsData.ssl, (unsigned char *)&httpsData.buffer[httpsData.offset], sizeof(((httpsData_t *)0)->buffer) - httpsData.offset);
    httpsData.timeout++;
    if (ret < 0)
    {
        switch (ret)
        {
        case MBEDTLS_ERR_SSL_WANT_READ:
            break;
        case MBEDTLS_ERR_SSL_WANT_WRITE:
            break;
        default:
            printl(LOG_WARN, "mbedtls_ssl_read returned %d\n", ret);
            currentPlayer->remove_player_event = 1;
            return 0;
            break;
        }
        return 1;
    }
    httpsData.offset += ret;
    httpsData.timeout = 0;
    // check if header is received
    char *header_end = strstr(httpsData.buffer, "\r\n\r\n");
    if (header_end == NULL)
    {
        return 1;
    }
    // check for HTTP 204
    if (strstr(httpsData.buffer, "204 No Content\r\n"))
    {
        printl(LOG_INFO, "HTTP 204 received\n");
        strncpy((char *)currentPlayer->disconnect_reason, "Invalid session (Try restarting your game)", sizeof(((player_t *)0)->disconnect_reason));
        currentPlayer->remove_player_event = 1;
    }
    // apparently its okay for the case to be different WHY
    char *content_length_header = strstr(httpsData.buffer, "Content-Length:");
    if (content_length_header == NULL)
    {
        content_length_header = strstr(httpsData.buffer, "content-length:");
        if (content_length_header == NULL)
        {
            return 1;
        }
    }
    size_t content_length = atoi(content_length_header + strlen("Content-Length:"));
    // check the content length
    if (content_length >= sizeof(((httpsData_t *)0)->buffer))
    {
        printl(LOG_WARN, "https_rtr_event content length too big\n");
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    // check if all the data has been received
    size_t len = (size_t)(header_end - httpsData.buffer) + 4; // len + 4 for the \r\n\r\n
    if ((httpsData.offset - len) != content_length)
    {
        return 1;
    }
    httpsData.len = content_length;
    httpsData.offset -= content_length;
    // add null terminator
    if (httpsData.offset + httpsData.len + 1 >= sizeof(((httpsData_t *)0)->buffer))
    {
        printl(LOG_WARN, "https_rtr_event buffer overflow\n");
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    httpsData.buffer[httpsData.offset + httpsData.len + 1] = '\0';
    lwjson_token_t tokens[10];
    lwjson_t lwjson;

    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    ret = lwjson_parse(&lwjson, &httpsData.buffer[httpsData.offset]);
    if (ret > 0)
    {
        printl(LOG_WARN, "lwjson_parse returned %d\n", ret);
        lwjson_free(&lwjson);
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    // parse the json
    const lwjson_token_t *t;
    if ((t = lwjson_find(&lwjson, "name")) == NULL)
    {
        lwjson_free(&lwjson);
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    if (t->type == LWJSON_TYPE_STRING)
    {
        if (t->u.str.token_value_len < sizeof(((player_t *)0)->name))
        {
            memset(currentPlayer->name, 0, sizeof(((player_t *)0)->name));
            memcpy(currentPlayer->name, t->u.str.token_value, t->u.str.token_value_len);
        }
    }
    // sanity check for the player name
    if (playerCheckName(currentPlayer))
    {
        strncpy((char *)currentPlayer->name, "stinky_player", sizeof(((player_t *)0)->name));
        lwjson_free(&lwjson);
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    // another name sanity check
    if (playerCheckDuplicate(currentPlayer))
    {
        strncpy((char *)currentPlayer->name, "stinky_player", sizeof(((player_t *)0)->name));
        lwjson_free(&lwjson);
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    if ((t = lwjson_find(&lwjson, "properties")) == NULL)
    {
        lwjson_free(&lwjson);
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    const lwjson_token_t *tkn = lwjson_get_first_child(t);
    if (tkn != NULL)
    {
        if (tkn->type == LWJSON_TYPE_OBJECT)
        {
            uint8_t texture_flag = 0;
            for (const lwjson_token_t *obj = lwjson_get_first_child(tkn); obj != NULL; obj = obj->next)
            {
                if (obj->type == LWJSON_TYPE_STRING)
                {
                    if (strncmp(obj->token_name, "name", obj->token_name_len) == 0)
                    {
                        if (strncmp(obj->u.str.token_value, "textures", obj->u.str.token_value_len) == 0)
                        {
                            texture_flag = 1;
                        }
                    }
                    if (texture_flag)
                    {
                        if (strncmp(obj->token_name, "value", obj->token_name_len) == 0)
                        {
                            if (currentPlayer->texture_value == NULL)
                            {
                                currentPlayer->texture_value = U_calloc(1, obj->u.str.token_value_len + 1);
                                if (currentPlayer->texture_value == NULL)
                                {
                                    printl(LOG_ERROR, "U_calloc returned NULL\n");
                                    lwjson_free(&lwjson);
                                    currentPlayer->remove_player_event = 1;
                                    return 0;
                                }
                                memcpy(currentPlayer->texture_value, obj->u.str.token_value, obj->u.str.token_value_len);
                                currentPlayer->texture_value_len = obj->u.str.token_value_len;
                            }
                        }
                        if (strncmp(obj->token_name, "signature", obj->token_name_len) == 0)
                        {
                            if (currentPlayer->texture_signature == NULL)
                            {
                                currentPlayer->texture_signature = U_calloc(1, obj->u.str.token_value_len + 1);
                                if (currentPlayer->texture_signature == NULL)
                                {
                                    printl(LOG_ERROR, "U_calloc returned NULL\n");
                                    lwjson_free(&lwjson);
                                    currentPlayer->remove_player_event = 1;
                                    return 0;
                                }
                                memcpy(currentPlayer->texture_signature, obj->u.str.token_value, obj->u.str.token_value_len);
                                currentPlayer->texture_signature_len = obj->u.str.token_value_len;
                                texture_flag = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    lwjson_free(&lwjson);
    // finally let the player join
    currentPlayer->login_event = 1;
    return 0;
}
int httpsRts(player_t *currentPlayer)
{
    if (currentPlayer == NULL)
    {
        printl(LOG_ERROR, "how??? currentPlayer is NULL\n");
        return 0;
    }
    if (httpsData.currentPlayer != currentPlayer)
    {
        printl(LOG_ERROR, "how??? currentPlayer is not the same\n");
        return 0;
    }
    if (httpsData.timeout > AUTH_TIMEOUT)
    {
        printl(LOG_ERROR, "httpsRtr timeout\n");
        strncpy((char *)currentPlayer->disconnect_reason, "Authentication Timeout", sizeof(((player_t *)0)->disconnect_reason));
        currentPlayer->remove_player_event = 1;
        return 0;
    }
    int ret = 0;
    if (httpsData.len == 0)
    {
        memset(httpsData.buffer, 0, sizeof(((httpsData_t *)0)->buffer));
        httpsData.offset = 0;
        httpsData.len = 0;
        currentPlayer->https_rtr_event = 1;
        return 0;
    }
    ret = mbedtls_ssl_write(&httpsData.ssl, (unsigned char *)&httpsData.buffer[httpsData.offset], httpsData.len);
    httpsData.timeout++;
    if (ret < 0)
    {
        switch (ret)
        {
        case MBEDTLS_ERR_SSL_WANT_READ:
            ret = mbedtls_ssl_handshake(&httpsData.ssl);
            if (ret < 0)
            {
                if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
                    ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
                    ret != MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS)
                {
                    printl(LOG_WARN, "mbedtls_ssl_handshake returned -0x%x\n", -ret);
                    currentPlayer->remove_player_event = 1;
                    return 0;
                }
            }
            break;
        case MBEDTLS_ERR_SSL_WANT_WRITE:
            break;
        default:
            printl(LOG_WARN, "mbedtls_ssl_write returned %d\n", ret);
            currentPlayer->remove_player_event = 1;
            return 0;
            break;
        }
        return 1;
    }
    httpsData.offset += ret;
    httpsData.len -= ret;
    httpsData.timeout = 0;
    return 1;
}
void httpsFreePlayer(player_t *currentPlayer)
{
    if (currentPlayer == NULL)
    {
        printl(LOG_ERROR, "how??? currentPlayer is NULL\n");
        return;
    }
    if (httpsData.currentPlayer == NULL)
    {
        return;
    }
    if (currentPlayer != httpsData.currentPlayer)
    {
        printl(LOG_ERROR, "how??? currentPlayer is not the same\n");
        return;
    }
    // send SSL/TLS closure notification
    int ret = 0;
    do
    {
        ret = mbedtls_ssl_close_notify(&httpsData.ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    U_close(httpsData.net.fd);
    mbedtls_ssl_free(&httpsData.ssl);
    mbedtls_ssl_config_free(&httpsData.conf);
    memset(&httpsData, 0, sizeof(httpsData_t));
    httpsData.currentPlayer = NULL;
}
void httpsCleanup()
{
    mbedtls_ssl_free(&httpsData.ssl);
    mbedtls_ssl_config_free(&httpsData.conf);
    memset(&httpsData, 0, sizeof(httpsData_t));
    httpsData.currentPlayer = NULL;
}
#endif /*ONLINE_MODE_AUTH*/