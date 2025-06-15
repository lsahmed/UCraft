#ifndef _S2C_H
#define _S2C_H
#include "socketio.h"
#include "enums.h"
#include "blocks/blocks.h"

#define S2C_PLAY_BUNDLE_DELIMITER 0x00
#define S2C_PLAY_ADD_ENTITY 0x01
#define S2C_PLAY_ANIMATE 0x02
#define S2C_PLAY_AWARD_STATS 0x03
#define S2C_PLAY_BLOCK_CHANGED_ACK 0x04
#define S2C_PLAY_BLOCK_DESTRUCTION 0x05
#define S2C_PLAY_BLOCK_ENTITY_DATA 0x06
#define S2C_PLAY_BLOCK_EVENT 0x07
#define S2C_PLAY_BLOCK_UPDATE 0x08
#define S2C_PLAY_BOSS_EVENT 0x09
#define S2C_PLAY_CHANGE_DIFFICULTY 0x0A
#define S2C_PLAY_CHUNK_BATCH_FINISHED 0x0B
#define S2C_PLAY_CHUNK_BATCH_START 0x0C
#define S2C_PLAY_CHUNKS_BIOMES 0x0D
#define S2C_PLAY_CLEAR_TITLES 0x0E
#define S2C_PLAY_COMMAND_SUGGESTIONS 0x0F
#define S2C_PLAY_COMMANDS 0x10
#define S2C_PLAY_CONTAINER_CLOSE 0x11
#define S2C_PLAY_CONTAINER_SET_CONTENT 0x12
#define S2C_PLAY_CONTAINER_SET_DATA 0x13
#define S2C_PLAY_CONTAINER_SET_SLOT 0x14
#define S2C_PLAY_COOKIE_REQUEST 0x15
#define S2C_PLAY_COOLDOWN 0x16
#define S2C_PLAY_CUSTOM_CHAT_COMPLETIONS 0x17
#define S2C_PLAY_CUSTOM_PAYLOAD 0x18
#define S2C_PLAY_DAMAGE_EVENT 0x19
#define S2C_PLAY_DEBUG_SAMPLE 0x1A
#define S2C_PLAY_DELETE_CHAT 0x1B
#define S2C_PLAY_DISCONNECT 0x1C
#define S2C_PLAY_DISGUISED_CHAT 0x1D
#define S2C_PLAY_ENTITY_EVENT 0x1E
#define S2C_PLAY_ENTITY_POSITION_SYNC 0x1F
#define S2C_PLAY_EXPLODE 0x20
#define S2C_PLAY_FORGET_LEVEL_CHUNK 0x21
#define S2C_PLAY_GAME_EVENT 0x22
#define S2C_PLAY_HORSE_SCREEN_OPEN 0x23
#define S2C_PLAY_HURT_ANIMATION 0x24
#define S2C_PLAY_INITIALIZE_BORDER 0x25
#define S2C_PLAY_KEEP_ALIVE 0x26
#define S2C_PLAY_LEVEL_CHUNK_WITH_LIGHT 0x27
#define S2C_PLAY_LEVEL_EVENT 0x28
#define S2C_PLAY_LEVEL_PARTICLES 0x29
#define S2C_PLAY_LIGHT_UPDATE 0x2A
#define S2C_PLAY_LOGIN 0x2B
#define S2C_PLAY_MAP_ITEM_DATA 0x2C
#define S2C_PLAY_MERCHANT_OFFERS 0x2D
#define S2C_PLAY_MOVE_ENTITY_POS 0x2E
#define S2C_PLAY_MOVE_ENTITY_POS_ROT 0x2F
#define S2C_PLAY_MOVE_MINECART_ALONG_TRACK 0x30
#define S2C_PLAY_MOVE_ENTITY_ROT 0x31
#define S2C_PLAY_MOVE_VEHICLE 0x32
#define S2C_PLAY_OPEN_BOOK 0x33
#define S2C_PLAY_OPEN_SCREEN 0x34
#define S2C_PLAY_OPEN_SIGN_EDITOR 0x35
#define S2C_PLAY_PING 0x36
#define S2C_PLAY_PONG_RESPONSE 0x37
#define S2C_PLAY_PLACE_GHOST_RECIPE 0x38
#define S2C_PLAY_PLAYER_ABILITIES 0x39
#define S2C_PLAY_PLAYER_CHAT 0x3A
#define S2C_PLAY_PLAYER_COMBAT_END 0x3B
#define S2C_PLAY_PLAYER_COMBAT_ENTER 0x3C
#define S2C_PLAY_PLAYER_COMBAT_KILL 0x3D
#define S2C_PLAY_PLAYER_INFO_REMOVE 0x3E
#define S2C_PLAY_PLAYER_INFO_UPDATE 0x3F
#define S2C_PLAY_PLAYER_LOOK_AT 0x40
#define S2C_PLAY_PLAYER_POSITION 0x41
#define S2C_PLAY_PLAYER_ROTATION 0x42
#define S2C_PLAY_RECIPE_BOOK_ADD 0x43
#define S2C_PLAY_RECIPE_BOOK_REMOVE 0x44
#define S2C_PLAY_RECIPE_BOOK_SETTINGS 0x45
#define S2C_PLAY_REMOVE_ENTITIES 0x46
#define S2C_PLAY_REMOVE_MOB_EFFECT 0x47
#define S2C_PLAY_RESET_SCORE 0x48
#define S2C_PLAY_RESOURCE_PACK_POP 0x49
#define S2C_PLAY_RESOURCE_PACK_PUSH 0x4A
#define S2C_PLAY_RESPAWN 0x4B
#define S2C_PLAY_ROTATE_HEAD 0x4C
#define S2C_PLAY_SECTION_BLOCKS_UPDATE 0x4D
#define S2C_PLAY_SELECT_ADVANCEMENTS_TAB 0x4E
#define S2C_PLAY_SERVER_DATA 0x4F
#define S2C_PLAY_SET_ACTION_BAR_TEXT 0x50
#define S2C_PLAY_SET_BORDER_CENTER 0x51
#define S2C_PLAY_SET_BORDER_LERP_SIZE 0x52
#define S2C_PLAY_SET_BORDER_SIZE 0x53
#define S2C_PLAY_SET_BORDER_WARNING_DELAY 0x54
#define S2C_PLAY_SET_BORDER_WARNING_DISTANCE 0x55
#define S2C_PLAY_SET_CAMERA 0x56
#define S2C_PLAY_SET_CHUNK_CACHE_CENTER 0x57
#define S2C_PLAY_SET_CHUNK_CACHE_RADIUS 0x58
#define S2C_PLAY_SET_CURSOR_ITEM 0x59
#define S2C_PLAY_SET_DEFAULT_SPAWN_POSITION 0x5A
#define S2C_PLAY_SET_DISPLAY_OBJECTIVE 0x5B
#define S2C_PLAY_SET_ENTITY_DATA 0x5C
#define S2C_PLAY_SET_ENTITY_LINK 0x5D
#define S2C_PLAY_SET_ENTITY_MOTION 0x5E
#define S2C_PLAY_SET_EQUIPMENT 0x5F
#define S2C_PLAY_SET_EXPERIENCE 0x60
#define S2C_PLAY_SET_HEALTH 0x61
#define S2C_PLAY_SET_HELD_SLOT 0x62
#define S2C_PLAY_SET_OBJECTIVE 0x63
#define S2C_PLAY_SET_PASSENGERS 0x64
#define S2C_PLAY_SET_PLAYER_INVENTORY 0x65
#define S2C_PLAY_SET_PLAYER_TEAM 0x66
#define S2C_PLAY_SET_SCORE 0x67
#define S2C_PLAY_SET_SIMULATION_DISTANCE 0x68
#define S2C_PLAY_SET_SUBTITLE_TEXT 0x69
#define S2C_PLAY_SET_TIME 0x6A
#define S2C_PLAY_SET_TITLE_TEXT 0x6B
#define S2C_PLAY_SET_TITLES_ANIMATION 0x6C
#define S2C_PLAY_SOUND_ENTITY 0x6D
#define S2C_PLAY_SOUND 0x6E
#define S2C_PLAY_START_CONFIGURATION 0x6F
#define S2C_PLAY_STOP_SOUND 0x70
#define S2C_PLAY_STORE_COOKIE 0x71
#define S2C_PLAY_SYSTEM_CHAT 0x72
#define S2C_PLAY_TAB_LIST 0x73
#define S2C_PLAY_TAG_QUERY 0x74
#define S2C_PLAY_TAKE_ITEM_ENTITY 0x75
#define S2C_PLAY_TELEPORT_ENTITY 0x76
#define S2C_PLAY_TEST_INSTANCE_BLOCK_STATUS 0x77
#define S2C_PLAY_TICKING_STATE 0x78
#define S2C_PLAY_TICKING_STEP 0x79
#define S2C_PLAY_TRANSFER 0x7A
#define S2C_PLAY_UPDATE_ADVANCEMENTS 0x7B
#define S2C_PLAY_UPDATE_ATTRIBUTES 0x7C
#define S2C_PLAY_UPDATE_MOB_EFFECT 0x7D
#define S2C_PLAY_UPDATE_RECIPES 0x7E
#define S2C_PLAY_UPDATE_TAGS 0x7F
#define S2C_PLAY_PROJECTILE_POWER 0x80
#define S2C_PLAY_CUSTOM_REPORT_DETAILS 0x81
#define S2C_PLAY_SERVER_LINKS 0x82

#define S2C_PLAY_MAPPING_LEN 0x83

#define S2C_CONFIGURATION_COOKIE_REQUEST 0x00
#define S2C_CONFIGURATION_CUSTOM_PAYLOAD 0x01
#define S2C_CONFIGURATION_DISCONNECT 0x02
#define S2C_CONFIGURATION_FINISH_CONFIGURATION 0x03
#define S2C_CONFIGURATION_KEEP_ALIVE 0x04
#define S2C_CONFIGURATION_PING 0x05
#define S2C_CONFIGURATION_RESET_CHAT 0x06
#define S2C_CONFIGURATION_REGISTRY_DATA 0x07
#define S2C_CONFIGURATION_RESOURCE_PACK_POP 0x08
#define S2C_CONFIGURATION_RESOURCE_PACK_PUSH 0x09
#define S2C_CONFIGURATION_STORE_COOKIE 0x0A
#define S2C_CONFIGURATION_TRANSFER 0x0B
#define S2C_CONFIGURATION_UPDATE_ENABLED_FEATURES 0x0C
#define S2C_CONFIGURATION_UPDATE_TAGS 0x0D
#define S2C_CONFIGURATION_SELECT_KNOWN_PACKS 0x0E
#define S2C_CONFIGURATION_CUSTOM_REPORT_DETAILS 0x0F
#define S2C_CONFIGURATION_SERVER_LINKS 0x10

#define S2C_CONFIGURATION_MAPPING_LEN 0x11

void StatusS2Cresponse(player_t *currentPlayer);
void StatusS2Cpong(player_t *currentPlayer);
void LoginS2Cdisconnect(player_t *currentPlayer, char *reason);
void LoginS2Cencryptionrequest(player_t *currentPlayer);
void LoginS2Ccompression(player_t *currentPlayer);
void LoginS2Csuccess(player_t *currentPlayer);
void PlayS2Clogin(player_t *currentPlayer);
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
void PlayS2Crotation(player_t *currentPlayer);
void PlayS2Cteleport(player_t *currentPlayer, double x, double y, double z);
void PlayS2Cheadrotation(player_t *currentPlayer);
void PlayS2Centityanimation(player_t *currentPlayer, uint8_t animation);
void PlayS2Ckick(player_t *currentPlayer, char *str);

void PlayS2Csysmessage(char *message,
                       size_t len);
void PlayS2Centitydestroy(int32_t eid);
void PlayS2Cblock(blocksDefaultState blockstate, int32_t x, int32_t y, int32_t z);
void PlayS2Cblockbreak(player_t *currentPlayer, int32_t sequence);
void PlayS2Cbossbar(player_t *currentPlayer, uint16_t uuid, int32_t action,
                    char *title, size_t len, float health);
void PlayS2Centitydata(player_t *currentPlayer, uint8_t entity, EntityDataMetadata type, State state);
void PlayS2Ccompassposition(player_t *currentPlayer, int32_t x, int32_t y, int32_t z);
void PlayS2Cdisconnect(player_t *currentPlayer, char *reason);
void ConfigurationS2Cfeatures();
void ConfigurationS2Cknownpacks();
void ConfigurationS2Cregistry();
void ConfigurationS2Cready();
void ConfigurationS2Cdisconnect(player_t *currentPlayer, char *reason);

#endif