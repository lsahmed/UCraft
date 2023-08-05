#ifndef _GAME_H
#define _GAME_H
#include "config.h"
#include <stdint.h>
#include <unistd.h>

typedef struct player_t player_t;

// server stuff
struct gameData_t
{
    uint8_t blocks[(2 * GAME_RADIUS * 2 * GAME_RADIUS + 7) / 8]; // bitset
    int32_t centerX, centerY, centerZ;
    size_t initalPlayers;
    size_t players;
    size_t startDelay; // in ticks
    uint8_t isStarted : 1;
};
typedef struct gameData_t gameData_t;
// per player stuff
struct gamePlayerData_t
{
    int32_t bx, bz; // previous block
    size_t timeout;
    size_t ongroundtimeout;
    uint8_t isPlaying : 1;
    uint8_t isSpectating : 1;
};
typedef struct gamePlayerData_t gamePlayerData_t;

void gamePreload();
void gameChunkLoaded(player_t *currentPlayer);
void gameGlobalPlayerMoved(player_t *currentPlayer);
void gamePlayerLeft(player_t *currentPlayer);
void gameGlobalTick();
void gamePlayerGlobalTick(player_t *currentPlayer);
void gamePlayerLocalTick(player_t *currentPlayer);

#endif