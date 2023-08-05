#ifndef _PLAYER_H
#define _PLAYER_H
#include "config.h"
#include "encryption.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "game.h"
typedef struct gamePlayerData_t gamePlayerData_t;
typedef struct player_t player_t;
struct player_t
{
  // General
  char playername[17];
  int32_t player_id; // negative player id is possible
  int player_fd;
  // Protocol
  int32_t handshake_status;
#ifdef ONLINE_MODE
  char verify_token[4];
  uint8_t iv_encrypt[16];
  uint8_t iv_decrypt[16];
  mbedtls_aes_context aes_ctx;
#ifdef ONLINE_MODE_AUTH
  char *texture_value;
  size_t texture_value_len;
  char *texture_signature;
  size_t texture_signature_len;
  uint8_t https_rts_event : 1; // ready to send
  uint8_t https_rtr_event : 1; // ready to read
  uint8_t https_wait_event : 1;
#endif /*ONLINE_MODE_AUTH*/
#endif /*ONLINE_MODE*/
// Generic Events
#ifdef ONLINE_MODE
  uint8_t encryption_event : 1;
  uint8_t encryption_recv_event : 1;
  uint8_t encryption_verified : 1;
#endif /*ONLINE_MODE*/
  uint8_t gamemode : 2;
  uint8_t teleport : 1;
  uint8_t playerlist_event : 1;
  uint8_t ping_event : 1;
  uint8_t compression_event : 1;
  uint8_t login_event : 1;
  uint8_t spawn_event : 1;
  uint8_t chunk_next_event : 1;
  uint8_t chunk_loaded_event : 1;
  uint8_t ingame : 1;
  uint8_t logged_on : 1;
  uint8_t active : 1;
  uint8_t onground : 1;
  uint8_t heartbeat : 1;
  uint8_t remove_player : 1;
  uint8_t send_chat_login_event : 1;
  uint8_t settings_changed_event : 1;
  // Player Movement & action
  uint8_t position_event : 1;
  uint8_t rotation_event : 1;
  uint8_t swing_arm_event : 1;
  uint8_t entity_action_event : 1;
  // Packet
  uint8_t packet_dispatch_flag : 1;
  size_t packet_len;
  size_t packet_sent;
  size_t packet_timeout;
  uint8_t *packet;
  // Chunk
  int32_t chunk_x, chunk_z, chunk_px, chunk_pz;
  // Chat
  uint8_t chat_event : 1;
  // Player skin
  uint8_t skin_parts;
  // Movement
  double x, y, z, px, py, pz;
  float yaw, pitch, pyaw, ppitch;
  uint8_t nyaw, npitch, npyaw, nppitch; // 1 to 255 'steps
  // Arm swing
  int32_t swing_arm_animation;
  // Sneaking
  int32_t entity_action_id;
  // Chat
  size_t chat_len;
  char *chat_ptr;
  int64_t chat_timestamp;
  // Misc
  uint8_t ability;
  size_t global_buffer_start_index;
  size_t loaded_chunks;
  // Game
  gamePlayerData_t gamePlayerData;
  // Private
  struct player_t *next;
};

struct playerd_t
{
  char *name;
  int32_t id;
  struct playerd_t *next;
};
typedef struct playerd_t playerd_t;

fd_set *playerGetSet();
void playerPushDisconnected(int32_t playerid, char *playername, size_t len);
playerd_t *playerPopDisconnected();
player_t *playerGetHead();
player_t *playerGetId(int32_t player_id);
player_t *playerAdd(uint32_t player_fd);
int playerCheckName(player_t *player);
int playerCheckDuplicate(player_t *player);
uint8_t playerRemove(player_t *player);
size_t playerGetCount();
size_t playerGetActiveCount();
void playerCleanup();
#endif
