#include <math.h>
#include <stdarg.h>

#include "game.h"
#include "log.h"
#include "s2c.h"
#include "wrapper.h"
#include "UCraft.h"
#include "blocks/blocks.h"

static gameData_t gameData;
// write a message to the chat
void printChatFormatted(char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    PlayS2Cunsignedchatmessage(buf, strnlen(buf, sizeof(buf)));
    va_end(args);
}
static uint8_t isPlayerInArena(player_t *currentPlayer)
{
    int32_t x = currentPlayer->x;
    int32_t z = currentPlayer->z;
    const int32_t radiusSquared = GAME_RADIUS * GAME_RADIUS;
    int distanceSquared = (x - gameData.centerX) * (x - gameData.centerX) + (z - gameData.centerZ) * (z - gameData.centerZ);
    return distanceSquared < radiusSquared;
}
static void setBlock(int32_t x, int32_t z, uint8_t value)
{
    size_t index = (x - gameData.centerX + GAME_RADIUS) + (z - gameData.centerZ + GAME_RADIUS) * GAME_RADIUS * 2;
    if (index / 8 < sizeof(gameData.blocks))
    {
        if (value)
        {
            gameData.blocks[index / 8] |= (1 << (index % 8));
        }
        else
        {
            gameData.blocks[index / 8] &= ~(1 << (index % 8));
        }
    }
}
static uint8_t getBlock(int32_t x, int32_t z)
{
    size_t index = (x - gameData.centerX + GAME_RADIUS) + (z - gameData.centerZ + GAME_RADIUS) * GAME_RADIUS * 2;
    if (index / 8 < sizeof(gameData.blocks))
    {
        return (gameData.blocks[index / 8] & (1 << (index % 8))) != 0;
    }
    return 0;
}
static void drawArena(player_t *currentPlayer, int32_t centerX, int32_t centerY, int32_t centerZ)
{
    int32_t x = 0;
    int32_t z = 0;
    gameData.centerX = centerX;
    gameData.centerY = centerY;
    gameData.centerZ = centerZ;
    const int32_t radiusSquared = GAME_RADIUS * GAME_RADIUS;
    for (z = centerZ - GAME_RADIUS; z <= centerZ + GAME_RADIUS; z++)
    {
        for (x = centerX - GAME_RADIUS; x <= centerX + GAME_RADIUS; x++)
        {
            int distanceSquared = (x - centerX) * (x - centerX) + (z - centerZ) * (z - centerZ);
            // construct the player area
            if (distanceSquared < radiusSquared)
            {
                if (getBlock(x, z))
                {
                    if ((distanceSquared % 5) == 0)
                    {
                        PlayS2Cblock(currentPlayer, MINECRAFT_SAND, x, centerY, z);
                    }
                    else
                    {
                        PlayS2Cblock(currentPlayer, MINECRAFT_RED_SAND, x, centerY, z);
                    }
                }
                else
                {
                    PlayS2Cblock(currentPlayer, MINECRAFT_AIR, x, centerY, z);
                }
            }
            // construct the border
            else if (distanceSquared - radiusSquared < 2 * GAME_RADIUS && distanceSquared - radiusSquared >= 0)
            {
                for (int i = -1; i < 3; i++)
                {
                    PlayS2Cblock(currentPlayer, MINECRAFT_SANDSTONE, x, centerY + i, z);
                }
            }
        }
    }
}
// ready every player who is spectating
static void readySpectators()
{
    player_t *currentPlayer = playerGetHead();
    size_t count = 0;
    while (currentPlayer != NULL)
    {
        if (currentPlayer->gamePlayerData.isSpectating)
        {
            currentPlayer->gamePlayerData.isSpectating = 0;
            currentPlayer->gamePlayerData.isPlaying = 1;
            currentPlayer->gamePlayerData.bx = 0;
            currentPlayer->gamePlayerData.bz = 0;
            currentPlayer->ability = ABILILTIES_CLEAR;
            currentPlayer->gamemode = 2;
            PlayS2Ctablist(currentPlayer, TABLIST_ACTION_GAMEMODE, currentPlayer->player_id);
            count++;
        }
        currentPlayer = currentPlayer->next;
    }
    gameData.initalPlayers = count;
    gameData.players = count;
}
// update player count
static void updatePlayerCount()
{
    player_t *currentPlayer = playerGetHead();
    size_t count = 0;
    while (currentPlayer != NULL)
    {
        if (currentPlayer->gamePlayerData.isPlaying)
        {
            count++;
        }
        currentPlayer = currentPlayer->next;
    }
    gameData.players = count;
}
static void startGame(size_t playerCount)
{
    if (playerCount <= 1)
    {
        if ((main_tick % 500) == 0)
        {
            printChatFormatted("Not enough players, waiting for players");
        }
        gameData.startDelay = 1000;
        return;
    }
    if (gameData.isStarted == 0)
    {
        if (gameData.startDelay == 0)
        {
            gameData.startDelay = 1000;
            gameData.isStarted = 1;
            return;
        }
        if (gameData.startDelay == 500)
        {
            readySpectators();
            // this will brodcast to all the players
            PlayS2Cpositionrotation(playerGetHead(), gameData.centerX, gameData.centerY + 2, gameData.centerZ);
        }
        if ((gameData.startDelay % 100) == 0)
        {
            printChatFormatted("Starting game in %lds", gameData.startDelay / 100);
        }
        gameData.startDelay--;
    }
    else
    {
        // check for a deadlock
        if ((gameData.players == 0) && gameData.initalPlayers)
        {
            gameData.isStarted = 0;
            gameData.initalPlayers = 0;
            gameData.players = 0;
            gameData.initalPlayers = 0;
        }
    }
}
// fired when the server is about to start
void gamePreload()
{
    memset(gameData.blocks, 0xff, sizeof(gameData.blocks));
    gameData.startDelay = 1000;
}
// fired when a player joins the server and the chunks are loaded
void gameChunkLoaded(player_t *currentPlayer)
{
    currentPlayer->gamePlayerData.isSpectating = 1;
    currentPlayer->ability = ABILILTIES_CAN_FLY | ABILILTIES_FLYING;
    drawArena(currentPlayer, 0, 0, 0);
}
// fired when a player leaves
void gamePlayerLeft(player_t *currentPlayer)
{
    memset(&currentPlayer->gamePlayerData, 0, sizeof(currentPlayer->gamePlayerData));
    updatePlayerCount();
}
// fired every tick in the global context
// NOTE: this will run even if there are no players so be careful when sending packets
void gameGlobalTick()
{
    size_t playerCount = 0;
    if ((playerCount = playerGetActiveCount()) == 0)
    {
        return;
    }
    startGame(playerCount);
}
// fired every tick in the global context for the current player
void gamePlayerGlobalTick(player_t *currentPlayer)
{
    // check the player if they go too far from the arena
    int32_t maxDistance = GAME_RADIUS + 10;
    if (currentPlayer->x > maxDistance || currentPlayer->x < -maxDistance || currentPlayer->y > maxDistance || currentPlayer->y < -maxDistance || currentPlayer->z > maxDistance || currentPlayer->z < -maxDistance)
    {
        currentPlayer->gamePlayerData.isPlaying = 0;
        currentPlayer->gamePlayerData.isSpectating = 1;
        currentPlayer->gamePlayerData.bx = 0;
        currentPlayer->gamePlayerData.bz = 0;
        currentPlayer->gamemode = 3;
        PlayS2Ctablist(currentPlayer, TABLIST_ACTION_GAMEMODE, currentPlayer->player_id);
        PlayS2Cteleport(currentPlayer, gameData.centerX, gameData.centerY + 4, gameData.centerZ);
        currentPlayer->teleport = 1;
        return;
    }
    // Game is started
    if (gameData.isStarted)
    {
        // check the player count in game
        if (gameData.players == 1 && gameData.initalPlayers > 0)
        {

            if (currentPlayer->gamePlayerData.isPlaying)
            {
                printChatFormatted("\u00A7a%s \u00A7ehas won the game!", currentPlayer->playername);
                memset(gameData.blocks, 0xff, sizeof(gameData.blocks));
                drawArena(currentPlayer, 0, 0, 0);
                gameData.isStarted = 0;
                gameData.initalPlayers = 0;
                gameData.players = 0;
                gameData.initalPlayers = 0;
                currentPlayer->gamePlayerData.isPlaying = 0;
                currentPlayer->gamePlayerData.isSpectating = 1;
                currentPlayer->gamePlayerData.bx = 0;
                currentPlayer->gamePlayerData.bz = 0;
            }
            return;
        }
        if (currentPlayer->gamePlayerData.isPlaying)
        {

            // check if the player is in the arena
            if (isPlayerInArena(currentPlayer) && (currentPlayer->y >= gameData.centerY + 1) && (currentPlayer->y < gameData.centerY + 3))
            {
                int32_t x = floor(currentPlayer->x);
                int32_t z = floor(currentPlayer->z);
                if (x == currentPlayer->gamePlayerData.bx && z == currentPlayer->gamePlayerData.bz)
                {
                    currentPlayer->gamePlayerData.ongroundtimeout++;
                }
                //player is in some corner, so get rid of the blocks
                if (currentPlayer->gamePlayerData.ongroundtimeout > 70)
                {
                    for (int32_t i = -1; i < 2; i++)
                    {
                        for (int32_t j = -1; j < 2; j++)
                        {
                            if (getBlock(x + i, z + j))
                            {
                                setBlock(x + i, z + j, 0);
                                PlayS2Cblock(currentPlayer, MINECRAFT_AIR, x + i, gameData.centerY, z + j);
                            }
                        }
                    }
                    currentPlayer->gamePlayerData.ongroundtimeout = 0;  
                }
                if (getBlock(x, z))
                {
                    if (currentPlayer->gamePlayerData.timeout > 40)
                    {
                        setBlock(x, z, 0);
                        PlayS2Cblock(currentPlayer, MINECRAFT_AIR, x, gameData.centerY, z);
                        currentPlayer->gamePlayerData.timeout = 0;
                    }
                    if (currentPlayer->gamePlayerData.bx != x || currentPlayer->gamePlayerData.bz != z)
                    {
                        setBlock(currentPlayer->gamePlayerData.bx, currentPlayer->gamePlayerData.bz, 0);
                        PlayS2Cblock(currentPlayer, MINECRAFT_AIR, currentPlayer->gamePlayerData.bx, gameData.centerY, currentPlayer->gamePlayerData.bz);
                        currentPlayer->gamePlayerData.bx = x;
                        currentPlayer->gamePlayerData.bz = z;
                        currentPlayer->gamePlayerData.timeout = 0;
                        currentPlayer->gamePlayerData.ongroundtimeout = 0;
                    }
                    currentPlayer->gamePlayerData.timeout++;
                }
                else
                {
                    // check if the player is unscyned
                    if (currentPlayer->onground)
                    {
                        PlayS2Cblock(currentPlayer, MINECRAFT_AIR, x, gameData.centerY, z);
                    }
                }
            }
            // check if the player fell down
            if (currentPlayer->y < gameData.centerY - 4)
            {
                currentPlayer->gamePlayerData.isPlaying = 0;
                currentPlayer->gamePlayerData.isSpectating = 1;
                currentPlayer->gamePlayerData.bx = 0;
                currentPlayer->gamePlayerData.bz = 0;
                currentPlayer->ability = ABILILTIES_CAN_FLY | ABILILTIES_FLYING;
                currentPlayer->gamemode = 3;
                PlayS2Ctablist(currentPlayer, TABLIST_ACTION_GAMEMODE, currentPlayer->player_id);
                updatePlayerCount();
                printChatFormatted("\u00A7a%s \u00A7ffell down (%ld/%ld)", currentPlayer->playername, gameData.players, gameData.initalPlayers);
                PlayS2Cteleport(currentPlayer, gameData.centerX, gameData.centerY + 2, gameData.centerZ);
                currentPlayer->teleport = 1;
            }
        }
    }
}

// fired in local context
void gamePlayerLocalTick(player_t *currentPlayer)
{
}