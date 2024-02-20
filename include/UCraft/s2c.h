#ifndef _S2C_H
#define _S2C_H
#include "socketio.h"
#include "enums.h"
#include "blocks/blocks.h"

#define S2C_PLAY_BUNDLE 0
#define S2C_PLAY_ENTITY_OBJECT_SPAWN 1
#define S2C_PLAY_ENTITY_EXPERIENCE_ORB 2
#define S2C_PLAY_ENTITY_ANIMATION 3
#define S2C_PLAY_STATISTICS 4
#define S2C_PLAY_BLOCK_BREAK 5
#define S2C_PLAY_BLOCK_BREAK_ANIMATION 6
#define S2C_PLAY_BLOCK_DATA 7
#define S2C_PLAY_BLOCK_ACTION 8
#define S2C_PLAY_BLOCK 9
#define S2C_PLAY_BOSSBAR 10
#define S2C_PLAY_DIFFICULTY 11
#define S2C_PLAY_CHUNK_BATCH_DONE 12
#define S2C_PLAY_CHUNK_BATCH_START 13
#define S2C_PLAY_CHUNK_BIOME 14
#define S2C_PLAY_CLEAR_TITLE 15
#define S2C_PLAY_COMMAND_SUGGESTIONS 16
#define S2C_PLAY_COMMANDS 17
#define S2C_PLAY_CLOSE_CONTAINER 18
#define S2C_PLAY_CONTAINER_ITEMS 19
#define S2C_PLAY_CONTAINER_PROPERTIES 20
#define S2C_PLAY_CONTAINER_ITEM 21
#define S2C_PLAY_ITEM_COOLDOWN 22
#define S2C_PLAY_CHAT_SUGGESTIONS 23
#define S2C_PLAY_CHANNEL 24
#define S2C_PLAY_ENTITY_DAMAGE 25
#define S2C_PLAY_HIDE_MESSAGE 26
#define S2C_PLAY_KICK 27
#define S2C_PLAY_UNSIGNED_CHAT_MESSAGE 28
#define S2C_PLAY_ENTITY_EVENT 29
#define S2C_PLAY_EXPLOSION 30
#define S2C_PLAY_UNLOAD_CHUNK 31
#define S2C_PLAY_GAME_EVENT 32
#define S2C_PLAY_OPEN_ENTITY_CONTAINER 33
#define S2C_PLAY_DAMAGE_TILT 34
#define S2C_PLAY_INITIALIZE_WORLD_BORDER 35
#define S2C_PLAY_HEARTBEAT 36
#define S2C_PLAY_CHUNK 37
#define S2C_PLAY_WORLD_EVENT 38
#define S2C_PLAY_PARTICLE 39
#define S2C_PLAY_CHUNK_LIGHT 40
#define S2C_PLAY_INITIALIZE 41
#define S2C_PLAY_MAP 42
#define S2C_PLAY_VILLAGER_TRADES 43
#define S2C_PLAY_RELATIVE_MOVE 44
#define S2C_PLAY_MOVEMENT_ROTATION 45
#define S2C_PLAY_ROTATION 46
#define S2C_PLAY_MOVE_VEHICLE 47
#define S2C_PLAY_BOOK 48
#define S2C_PLAY_OPEN_CONTAINER 49
#define S2C_PLAY_SIGN_EDITOR 50
#define S2C_PLAY_PING 51
#define S2C_PLAY_PONG 52
#define S2C_PLAY_CRAFTING_RECIPE 53
#define S2C_PLAY_PLAYER_ABILITIES 54
#define S2C_PLAY_SIGNED_CHAT_MESSAGE 55
#define S2C_PLAY_END_COMBAT_EVENT 56
#define S2C_PLAY_ENTER_COMBAT_EVENT 57
#define S2C_PLAY_KILL_COMBAT_EVENT 58
#define S2C_PLAY_TAB_LIST_REMOVE 59
#define S2C_PLAY_TAB_LIST 60
#define S2C_PLAY_PLAYER_FACE 61
#define S2C_PLAY_POSITION_ROTATION 62
#define S2C_PLAY_UNLOCK_RECIPES 63
#define S2C_PLAY_ENTITY_DESTROY 64
#define S2C_PLAY_ENTITY_REMOVE_EFFECT 65
#define S2C_PLAY_REMOVE_SCOREBOARD_SCORE 66
#define S2C_PLAY_REMOVE_RESOURCEPACK 67
#define S2C_PLAY_RESOURCEPACK 68
#define S2C_PLAY_RESPAWN 69
#define S2C_PLAY_HEAD_ROTATION 70
#define S2C_PLAY_BLOCKS 71
#define S2C_PLAY_ADVANCEMENT_TAB 72
#define S2C_PLAY_PLAY_STATUS 73
#define S2C_PLAY_HOTBAR_TEXT 74
#define S2C_PLAY_CENTER_WORLD_BORDER 75
#define S2C_PLAY_INTERPOLATE_WORLD_BORDER 76
#define S2C_PLAY_SIZE_WORLD_BORDER 77
#define S2C_PLAY_WARN_TIME_WORLD_BORDER 78
#define S2C_PLAY_WARN_BLOCKS_WORLD_BORDER 79
#define S2C_PLAY_CAMERA 80
#define S2C_PLAY_HOTBAR_SLOT 81
#define S2C_PLAY_CHUNK_CENTER 82
#define S2C_PLAY_VIEW_DISTANCE 83
#define S2C_PLAY_COMPASS_POSITION 84
#define S2C_PLAY_OBJECTIVE_POSITION 85
#define S2C_PLAY_ENTITY_DATA 86
#define S2C_PLAY_ENTITY_ATTACH 87
#define S2C_PLAY_VELOCITY 88
#define S2C_PLAY_ENTITY_EQUIPMENT 89
#define S2C_PLAY_EXPERIENCE 90
#define S2C_PLAY_HEALTH 91
#define S2C_PLAY_OBJECTIVE 92
#define S2C_PLAY_ENTITY_PASSENGER 93
#define S2C_PLAY_TEAMS 94
#define S2C_PLAY_PUT_SCOREBOARD_SCORE 95
#define S2C_PLAY_SIMULATION_DISTANCE 96
#define S2C_PLAY_SUBTITLE 97
#define S2C_PLAY_TIME 98
#define S2C_PLAY_TITLE_TEXT 99
#define S2C_PLAY_TITLE_TIMES 100
#define S2C_PLAY_ENTITY_SOUND 101
#define S2C_PLAY_SOUND_EVENT 102
#define S2C_PLAY_RECONFIGURE 103
#define S2C_PLAY_STOP_SOUND 104
#define S2C_PLAY_CHAT_MESSAGE 105
#define S2C_PLAY_TAB_LIST_TEXT 106
#define S2C_PLAY_NBT_RESPONSE 107
#define S2C_PLAY_ENTITY_COLLECT 108
#define S2C_PLAY_TELEPORT 109
#define S2C_PLAY_TICK_RATE 110
#define S2C_PLAY_TICK_STEP 111
#define S2C_PLAY_ADVANCEMENTS 112
#define S2C_PLAY_ENTITY_ATTRIBUTES 113
#define S2C_PLAY_ENTITY_EFFECT 114
#define S2C_PLAY_RECIPES 115
#define S2C_PLAY_TAGS 116
#define S2C_PLAY_SCOREBOARD_SCORE 117
#define S2C_PLAY_ENTITY_PLAYER 118
#define S2C_PLAY_FEATURES 119
#define S2C_PLAY_CHAT_PREVIEW 120
#define S2C_PLAY_NAMED_SOUND 121
#define S2C_PLAY_MESSAGE_HEADER 122
#define S2C_PLAY_TEMPORARY_CHAT_PREVIEW 123
#define S2C_PLAY_ENTITY_MOB_SPAWN 124
#define S2C_PLAY_ENTITY_PAINTING 125
#define S2C_PLAY_VIBRATION 126
#define S2C_PLAY_LEGACY_BLOCK_BREAK 127
#define S2C_PLAY_CONTAINER_ACTION 128
#define S2C_PLAY_EMPTY_MOVE 129
#define S2C_PLAY_COMBAT_EVENT 130
#define S2C_PLAY_WORLD_BORDER 131
#define S2C_PLAY_TITLE 132
#define S2C_PLAY_GLOBAL_ENTITY_SPAWN 133
#define S2C_PLAY_ENTITY_SLEEP 134
#define S2C_PLAY_SIGN_TEXT 135
#define S2C_PLAY_CHUNKS 136
#define S2C_PLAY_COMPRESSION 137
#define S2C_PLAY_LEGACY_MAP 138
#define S2C_PLAY_LEGACY_TAB_LIST 139

#define S2C_PLAY_MAPPING_LEN 140

#define S2C_CONFIGURATION_CHANNEL 0
#define S2C_CONFIGURATION_KICK 1
#define S2C_CONFIGURATION_READY 2
#define S2C_CONFIGURATION_HEARTBEAT 3
#define S2C_CONFIGURATION_PING 4
#define S2C_CONFIGURATION_REGISTRIES 5
#define S2C_CONFIGURATION_REMOVE_RESOURCEPACK 6
#define S2C_CONFIGURATION_RESOURCEPACK 7
#define S2C_CONFIGURATION_FEATURES 8
#define S2C_CONFIGURATION_TAGS 9

#define S2C_CONFIGURATION_MAPPING_LEN 10

extern const uint8_t s2c_1_19_4_1_20_1[];

void StatusS2Cresponse(player_t *currentPlayer);
void StatusS2Cpong(player_t *currentPlayer);
void LoginS2Cencryptionrequest(player_t *currentPlayer);
void LoginS2Ccompression(player_t *currentPlayer);
void LoginS2Csuccess(player_t *currentPlayer);
void PlayS2Cjoingame(player_t *currentPlayer);
void PlayS2Ctablist(player_t *currentPlayer, TabListAction action, uint16_t eid);
void PlayS2Cgameevent(GameEvent event, float value);
void PlayS2Cplayerabilities(player_t *currentPlayer, PlayerAbilities abilities);
void PlayS2Ctablistremove(uint16_t eid);
void PlayS2Cspawnentity(player_t *currentPlayer, EntityMetadataType type);
void PlayS2Cpositionrotation(player_t *currentPlayer, double x, double y,
                             double z);
void PlayS2Cchunkcenter(player_t *currentPlayer, int32_t x, int32_t z);
void PlayS2Cchunk(player_t *currentPlayer, int32_t x, int32_t z);
void PlayS2Cheartbeat(player_t *currentPlayer);
void PlayS2Crelativemove(player_t *currentPlayer, int16_t diffx, int16_t diffy,
                         int16_t diffz);
void PlayS2Cmovementrotation(player_t *currentPlayer, int16_t diffx,
                             int16_t diffy, int16_t diffz);
void PlayS2Crotation(player_t *currentPlayer);
void PlayS2Cteleport(player_t *currentPlayer, double x, double y, double z);
void PlayS2Cheadrotation(player_t *currentPlayer);
void PlayS2Centityanimation(player_t *currentPlayer, uint8_t animation);
void PlayS2Ckick(player_t *currentPlayer, char *str);
void PlayS2Csignedchatmessage(player_t *currentPlayer, char *message,
                              size_t len);
void PlayS2Cunsignedchatmessage(char *message,
                                size_t len);
void PlayS2Centitydestroy(int32_t eid);
void PlayS2Cblock(player_t *currentPlayer, blocksDefaultState blockstate, int32_t x, int32_t y, int32_t z);
void PlayS2Cblockbreak(player_t *currentPlayer, int32_t sequence);
void PlayS2Cbossbar(player_t *currentPlayer, uint16_t uuid, int32_t action,
                    char *title, size_t len, float health);
void PlayS2Centitydata(player_t *currentPlayer, uint8_t entity, EntityDataMetadata type, State state);
void PlayS2Ccompassposition(player_t *currentPlayer, int32_t x, int32_t y, int32_t z);
void ConfigurationS2Cregistry();
void ConfigurationS2Cready();
#endif