#ifndef _C2S_H
#define _C2S_H
#include "socketio.h"
#include <stdint.h>

extern void (*c2s_1_19_4_1_19_4[])(player_t *currentPlayer);
extern uint8_t chat_inuse;

#define PLAYC2S_MAPPING_LEN 56
#endif