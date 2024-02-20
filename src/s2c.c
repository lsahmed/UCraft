#include <stdint.h>
#include <stdio.h>

#include "s2c.h"
#include "enums.h"
#include "log.h"
#include "encryption.h"
#include "wrapper.h"
#include "util.h"
#ifdef ONLINE_MODE
#include "mbedtls/base64.h"
#endif /*ONLINE_MODE*/

extern int main_tick;

// Status packets
void StatusS2Cresponse(player_t *currentPlayer)
{
  char buffer[300];
  char scratch[24];
  static const char json1[] = "{\"description\":\"" MOTD "\", \"players\":{\"max\":";
  static const char json2[] = ",\"online\":";
  static const char json3[] = "},\"version\":{\"name\":\"" LONG_PROTOCOL_VERSION "\",\"protocol\":";
  static const char json4[] = "}}";
  sendStart();
  sendByte(0);
  sendSwitchToLocalBuffer(buffer, sizeof(buffer));
  sendBuffer(json1, strlen(json1));
  snprintf(scratch, sizeof(scratch), "%d", MAX_PLAYERS);
  sendBuffer(scratch, strnlen(scratch, sizeof(scratch)));
  sendBuffer(json2, strlen(json2));
  snprintf(scratch, sizeof(scratch), "%ld", playerGetActiveCount());
  sendBuffer(scratch, strnlen(scratch, sizeof(scratch)));
  sendBuffer(json3, strlen(json3));
  snprintf(scratch, sizeof(scratch), "%d", PROTOCOL_VERSION);
  sendBuffer(scratch, strnlen(scratch, sizeof(scratch)));
  sendBuffer(json4, strlen(json4));
  int size = sendRevertFromLocalBuffer();
  sendString(buffer, size);
  sendDone();
}
void StatusS2Cpong(player_t *currentPlayer)
{

  sendStart();
  sendByte(0x01);
  sendBuffer(currentPlayer->playername, 8);
  sendDone();
}
// Login packets
#ifdef ONLINE_MODE
void LoginS2Cencryptionrequest(player_t *currentPlayer)
{
  uint8_t key_buf[256];
  char serverid[20];
  char token[4];
  sendStart();
  sendByte(0x01);
  // send the server id {scratch buffer}
  memset(serverid, 0, sizeof(serverid));
  sendString((char *)serverid, 20);
  int ret = mbedtls_pk_write_pubkey_der(&encryptionGetData()->key, key_buf, sizeof(key_buf));
  if (ret < 0)
  {
    printl(LOG_ERROR, "mbedtls_pk_write_pubkey_der returned %d\n", ret);
    return;
  }
  // send the public key
  sendVarInt(ret);
  sendBuffer((char *)&key_buf[sizeof(key_buf) - ret], ret);
  // generate a random token
  ret = mbedtls_ctr_drbg_random(&encryptionGetData()->ctr_drbg, (unsigned char *)&token, sizeof(token));
  if (ret < 0)
  {
    printl(LOG_ERROR, "mbedtls_ctr_drbg_random returned %d\n", ret);
    return;
  }
  memcpy(currentPlayer->verify_token, token, sizeof(token));
  sendVarInt(4);
  sendBuffer(token, 4);
  sendDone();
}
#endif /*ONLINE_MODE*/
void LoginS2Ccompression(player_t *currentPlayer)
{
  sendStart();
  sendByte(0x3);
  sendVarInt(COMPRESSION_THRESHOLD);
  sendDone();
  currentPlayer->compression_event = 1;
}
void LoginS2Csuccess(player_t *currentPlayer)
{
  sendStart();
  sendByte(0x02);
  sendUUID(currentPlayer->player_id); // UUID
  // sendBuffer((char*)currentPlayer->uuid, 16);
  sendString(currentPlayer->playername, -1); // Username
#ifdef ONLINE_MODE_AUTH
  if (currentPlayer->texture_value && currentPlayer->texture_signature)
  {
    sendVarInt(1);                                                              // Number Of Properties
    sendString("textures", -1);                                                 // Property Name
    sendString(currentPlayer->texture_value, currentPlayer->texture_value_len); // Value
    sendByte(1);                                                                // Is Signed
    sendString(currentPlayer->texture_signature, currentPlayer->texture_signature_len);
  }
  else
  {
    sendVarInt(0); // Number Of Properties
  }
#else
  sendVarInt(0); // Number Of Properties
#endif /*ONLINE_MODE_AUTH*/
  sendDone();
}
// Play packets

void PlayS2Cjoingame(player_t *currentPlayer)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_INITIALIZE);
  sendInt(currentPlayer->player_id);
  sendByte(0);                           // Is hardcore
  sendVarInt(1);                         // World Count
  sendString("world", -1);               // Dimension Names
  sendVarInt(MAX_PLAYERS);               // Max Players
  sendVarInt(VIEWDISTANCE);              // viewdistance
  sendVarInt(VIEWDISTANCE);              // simulationdistance
  sendByte(0);                           // Reduced Debug Info
  sendByte(1);                           // Enable respawn screen
  sendByte(0);                           // Do limited crafting
  sendString("minecraft:overworld", -1); // Dimension Type
  sendString("minecraft:overworld", -1); // Dimension Name
  sendLong(0x482304890);                 //{hashedSeed} random bytes
  sendByte(GAMEMODE);                    // gamemode
  currentPlayer->gamemode = GAMEMODE;
  sendByte(-1);  // previous gamemode
  sendByte(0);   // Is Debug
  sendByte(0);   // Is Flat
  sendByte(0);   // Has death location
  sendVarInt(0); // Portal cooldown
  sendDone();
}
void PlayS2Ctablist(player_t *currentPlayer, TabListAction action, uint16_t eid)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_TAB_LIST);
  // TODO: handle more actions
  sendByte(action);
  sendVarInt(1); // Number Of Actions
  sendUUID(eid);
  if (action & TABLIST_ACTION_ADDPLAYER)
  {
    sendString(currentPlayer->playername, -1);
#ifdef ONLINE_MODE_AUTH
    if (currentPlayer->texture_value && currentPlayer->texture_signature)
    {
      sendVarInt(1);                                                              // Number Of Properties
      sendString("textures", -1);                                                 // Property Name
      sendString(currentPlayer->texture_value, currentPlayer->texture_value_len); // Value
      sendByte(1);                                                                // Is Signed
      sendString(currentPlayer->texture_signature, currentPlayer->texture_signature_len);
    }
    else
    {
      sendVarInt(0); // Number Of Properties
    }
#else
    sendVarInt(0); // Number Of Properties
#endif /*ONLINE_MODE_AUTH*/
  }
  if (action & TABLIST_ACTION_GAMEMODE)
  {
    sendVarInt(currentPlayer->gamemode);
  }
  if (action & TABLIST_ACTION_LISTED)
  {
    sendByte(1);
  }
  if (action & TABLIST_ACTION_LATENCY)
  {
    sendVarInt(0);
  }
  if (action & TABLIST_ACTION_NAME)
  {
    printl(LOG_WARN, "tablist action name is not implemented!\n");
    sendByte(0x0);
  }
  sendDone();
}
void PlayS2Cgameevent(GameEvent event, float value)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_GAME_EVENT);
  sendByte(event);
  sendFloat(value);
  sendDone();
}
void PlayS2Cplayerabilities(player_t *currentPlayer, PlayerAbilities abilities)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_PLAYER_ABILITIES);
  sendByte(abilities);
  sendFloat(0.05); // Flying Speed
  sendFloat(0.1);  // FOV Modifier
  sendDone();
}
void PlayS2Ctablistremove(uint16_t eid)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_TAB_LIST_REMOVE);
  sendVarInt(1);
  sendUUID(eid);
  sendDone();
}
void PlayS2Cspawnentity(player_t *currentPlayer, EntityMetadataType type)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_ENTITY_OBJECT_SPAWN);
  sendVarInt(currentPlayer->player_id);
  sendUUID(currentPlayer->player_id);
  sendVarInt(type);
  sendDouble(currentPlayer->x);
  sendDouble(currentPlayer->y);
  sendDouble(currentPlayer->z);
  sendByte(currentPlayer->npitch);
  sendByte(currentPlayer->nyaw);
  sendByte(currentPlayer->nyaw);
  sendVarInt(0);
  sendShort(0);
  sendShort(0);
  sendShort(0);
  sendDone();
}
void PlayS2Cpositionrotation(player_t *currentPlayer, double x, double y, double z)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_POSITION_ROTATION);
  sendDouble(x);
  sendDouble(y);
  sendDouble(z);
  sendFloat(currentPlayer->yaw);
  sendFloat(currentPlayer->pitch);
  sendByte(0);   // xyz absolute
  sendVarInt(0); // teleportId
  sendDone();
}
void PlayS2Cchunkcenter(player_t *currentPlayer, int32_t x, int32_t z)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_CHUNK_CENTER);
  sendVarInt(x);
  sendVarInt(z);
  sendDone();
}
void PlayS2Cchunk(player_t *currentPlayer, int32_t x, int32_t z)
{
  // Thanks to bixilon(https://gitlab.bixilon.de/) for this minimal chunk
  // generation!
  char block_entity[256];
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_CHUNK);
  sendInt(x);
  sendInt(z);
  // NBT start
  sendByte(0x0a);
  sendByte(0);
  //  NBT end
  // switch to another buffer so the size can be appended later on
  sendSwitchToLocalBuffer(block_entity, sizeof(block_entity));
  // this part of the chunk depends on the height and logical_height from the
  // codec since section = logical_height/16 BUT for logical_height > 64
  // something changes and the client expects the value to be <= 32 hence the
  // chosen value
  for (int i = 0; i < 32; i++)
  {
    sendShort(0);  // block count
    sendByte(0);   // blocks singular
    sendByte(0);   // block id
    sendByte(0);   // biome singular
    sendVarInt(0); // biome id
  }
  sendByte(0);
  sendByte(0);
  sendByte(0);
  sendVarInt(0);
  size_t len = sendRevertFromLocalBuffer();
  sendVarInt(len);
  sendBuffer(block_entity, len);
  sendVarInt(0); // block entiies
  for (int i = 0; i < 4; i++)
  { // Sky Light Mask,...
    sendVarInt(1);
    sendLong(0);
  }
  sendVarInt(0); // Sky Light array count
  sendVarInt(0); // Block Light array count
  sendDone();
}
void PlayS2Cheartbeat(player_t *currentPlayer)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_HEARTBEAT);
  sendLong(main_tick);
  sendDone();
}
void PlayS2Crelativemove(player_t *currentPlayer, int16_t diffx, int16_t diffy, int16_t diffz)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_RELATIVE_MOVE);
  sendVarInt(currentPlayer->player_id);
  sendShort(diffx);
  sendShort(diffy);
  sendShort(diffz);
  sendByte(currentPlayer->onground);
  sendDone();
}
void PlayS2Cmovementrotation(player_t *currentPlayer, int16_t diffx, int16_t diffy, int16_t diffz)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_MOVEMENT_ROTATION);
  sendVarInt(currentPlayer->player_id);
  sendShort(diffx);
  sendShort(diffy);
  sendShort(diffz);
  sendByte(currentPlayer->nyaw);
  sendByte(currentPlayer->npitch);
  sendByte(currentPlayer->onground);
  sendDone();
}
void PlayS2Crotation(player_t *currentPlayer)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_ROTATION);
  sendVarInt(currentPlayer->player_id);
  sendByte(currentPlayer->nyaw);
  sendByte(currentPlayer->npitch);
  sendByte(currentPlayer->onground);
  sendDone();
}
void PlayS2Cteleport(player_t *currentPlayer, double x, double y, double z)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_TELEPORT);
  sendVarInt(currentPlayer->player_id);
  sendDouble(x);
  sendDouble(y);
  sendDouble(z);
  sendByte(currentPlayer->nyaw);
  sendByte(currentPlayer->npitch);
  sendByte(currentPlayer->onground);
  sendDone();
  currentPlayer->x = x;
  currentPlayer->y = y;
  currentPlayer->z = z;
}
void PlayS2Cheadrotation(player_t *currentPlayer)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_HEAD_ROTATION);
  sendVarInt(currentPlayer->player_id);
  sendByte(currentPlayer->nyaw);
  sendDone();
}
void PlayS2Centityanimation(player_t *currentPlayer, uint8_t animation)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_ENTITY_ANIMATION);
  sendVarInt(currentPlayer->player_id);
  sendByte(animation);
  sendDone();
}
void PlayS2Ckick(player_t *currentPlayer, char *str)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_KICK);
  sendString(str, -1);
  sendDone();
}
void PlayS2Csignedchatmessage(player_t *currentPlayer, char *message, size_t len)
{

  char buf[300];
  static const char json1[] = "{\"text\":\"";
  static const char json2[] = "\"}";
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_SIGNED_CHAT_MESSAGE);
  sendSwitchToLocalBuffer(buf, sizeof(buf));
  sendBuffer(json1, strlen(json1));
  sendBuffer(message, len);
  sendBuffer(json2, strlen(json2));
  size_t size = sendRevertFromLocalBuffer();
  sendString(buf, size);
  sendByte(0);
  sendVarInt(0);
  sendUUID(currentPlayer->player_id);
  sendSwitchToLocalBuffer(buf, sizeof(buf));
  sendBuffer(json1, strlen(json1));
  sendBuffer(currentPlayer->playername, strnlen(currentPlayer->playername, sizeof(currentPlayer->playername)));
  sendBuffer(json2, strlen(json2));
  size = sendRevertFromLocalBuffer();
  sendString(buf, size);
  sendByte(0);
  sendLong(currentPlayer->chat_timestamp);
  sendLong(0);
  sendVarInt(0); // Signature Length
  sendDone();
}
void PlayS2Cunsignedchatmessage(char *message, size_t len)
{
  static const unsigned char NBT_text[9] = {
      0x0A, 0x08, 0x00, 0x04, 0x74, 0x65, 0x78, 0x74, 0x00};
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_CHAT_MESSAGE);
  sendBuffer((char *)NBT_text, 9);
  sendString(message, len);
  sendByte(0);
  sendByte(0);
  sendDone();
}
void PlayS2Centitydestroy(int32_t eid)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_ENTITY_DESTROY);
  sendVarInt(1);
  sendVarInt(eid);
  sendDone();
}
void PlayS2Cblock(player_t *currentPlayer, blocksDefaultState blockstate, int32_t x, int32_t y, int32_t z)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_BLOCK);
  sendPosition(x, y, z);
  sendVarInt(blockstate);
  sendDone();
}
// Acknowledge Block Change
void PlayS2Cblockbreak(player_t *currentPlayer, int32_t sequence)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_BLOCK_BREAK);
  sendVarInt(sequence);
  sendDone();
}
// TODO: make this packet more cleaner
void PlayS2Cbossbar(player_t *currentPlayer, uint16_t uuid, int32_t action, char *title, size_t len, float health)
{
  char buf[300];
  size_t size;
  static const char json1[] = "{\"text\":\"";
  static const char json2[] = "\"}";
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_BOSSBAR);
  sendUUID(uuid);
  sendVarInt(action);
  switch (action)
  {
  case 0:
    sendSwitchToLocalBuffer(buf, sizeof(buf));
    sendBuffer(json1, strlen(json1));
    if (len < sizeof(buf) - 13)
    {
      if (title)
      {
        sendBuffer(title, len);
      }
    }
    sendBuffer(json2, strlen(json2));
    size = sendRevertFromLocalBuffer();
    sendString(buf, size);
    sendFloat(health);
    sendVarInt(4);
    sendVarInt(0); // 20 notches
    sendByte(0);
    break;
  case 2:
    sendFloat(health);
    break;
  case 3:
    sendSwitchToLocalBuffer(buf, sizeof(buf));
    sendBuffer(json1, strlen(json1));
    if (len < sizeof(buf) - 13)
    {
      if (title)
      {
        sendBuffer(title, len);
      }
    }
    sendBuffer(json2, strlen(json2));
    size = sendRevertFromLocalBuffer();
    sendString(buf, size);
  default:
    break;
  }
  sendDone();
}
void PlayS2Centitydata(player_t *currentPlayer, uint8_t entity, EntityDataMetadata type, State state)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_ENTITY_DATA);
  sendVarInt(currentPlayer->player_id);
  sendByte(entity); // Entity base class "Pose" field ENTITY_POSE
  sendByte(type);   // Type field
  sendVarInt(state);
  sendByte(0xff);
  sendDone();
}
void PlayS2Ccompassposition(player_t *currentPlayer, int32_t x, int32_t y, int32_t z)
{
  sendStart();
  sendPlayPacketHeader(S2C_PLAY_COMPASS_POSITION);
  sendPosition(x, y, z);
  sendFloat(0);
  sendDone();
}
void ConfigurationS2Cregistry()
{
  extern const uint8_t __codec_nbt[];
  extern const size_t __codec_nbt_len;
  sendStart();
  sendConfigurationPacketHeader(S2C_CONFIGURATION_REGISTRIES);
  sendBuffer((char *)__codec_nbt, __codec_nbt_len);
  sendDone();
}
void ConfigurationS2Cready()
{
  sendStart();
  sendConfigurationPacketHeader(S2C_CONFIGURATION_READY);
  sendDone();
}