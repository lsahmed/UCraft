#ifndef _C2S_H
#define _C2S_H
#include "socketio.h"
#include <stdint.h>

extern void (*c2s_play_1_20_4_1_20_4[])(player_t *currentPlayer);
extern void (*c2s_configuration_1_20_4_1_20_4[])(player_t *currentPlayer);

extern uint8_t chat_inuse;

#define C2S_PLAY_MAPPING_LEN 60
#define C2S_CONFIGURATION_MAPPING_LEN 6

#endif