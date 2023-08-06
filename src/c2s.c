#include "c2s.h"

static void PlayC2S_confirm_teleport(player_t *currentPlayer) {}
static void PlayC2S_block_nbt(player_t *currentPlayer) {}
static void PlayC2S_difficulty(player_t *currentPlayer) {}
static void PlayC2S_message_acknowledgement(player_t *currentPlayer) {}
static void PlayC2S_command(player_t *currentPlayer) {}
static char chat_buffer[300];
uint8_t chat_inuse = 0;
static void PlayC2S_signed_chat_message(player_t *currentPlayer)
{
  if (!chat_inuse)
  {
    int32_t length = readVarInt();
    if (length <= 255)
    {
      memset(chat_buffer, 0, sizeof(chat_buffer));
      // add the decorators, very ugly but trying to avoid using snprintf here
      chat_buffer[0] = '<';
      char *name = strncpy(&chat_buffer[1], currentPlayer->playername, sizeof(((player_t *)0)->playername));
      char *footer = &name[strnlen(name, sizeof(((player_t *)0)->playername))];
      *footer++ = '>';
      *footer++ = ' ';
      for (int i = 0; i < length; i++)
      {
        readBuffer(&footer[i], 1);
        if (footer[i] < ' ')
        {
          chat_inuse = 0;
          return;
        }
        if (footer[i] == '\"')
        {
          chat_inuse = 0;
          return;
        }
        // remove '_' since its valid
        if ((footer[i] >= '[' && footer[i] <= '`') && footer[i] != '_')
        {
          chat_inuse = 0;
          return;
        }
        if (footer[i] >= '{')
        {
          chat_inuse = 0;
          return;
        }
        if (footer[i] == 0)
        {
          i--;
          break;
        }
      }
      currentPlayer->chat_timestamp = readLong();
      currentPlayer->chat_event = 1;
      currentPlayer->chat_len = strnlen(chat_buffer, sizeof(chat_buffer)); // do not count the null char
      currentPlayer->chat_ptr = chat_buffer;
      chat_inuse = 1;
    }
  }
}
static void PlayC2S_client_action(player_t *currentPlayer) {}
static void PlayC2S_settings(player_t *currentPlayer)
{
  char buf[17];
  readString(buf, sizeof(buf));           // Locale
  readByte();                             // View Distance
  readVarInt();                           // Chat Mode
  readByte();                             // Chat Colors
  currentPlayer->skin_parts = readByte(); // Displayed Skin Parts
  readVarInt();                           // Main Hand
  readByte();                             // Disable Text Filter
  readByte();                             // Allow server listing
  currentPlayer->settings_changed_event = 1;
}
static void PlayC2S_command_suggestions(player_t *currentPlayer) {}
static void PlayC2S_container_button(player_t *currentPlayer) {}
static void PlayC2S_container_click(player_t *currentPlayer) {}
static void PlayC2S_close_container(player_t *currentPlayer) {}
static void PlayC2S_channel(player_t *currentPlayer) {}
// static void PlayC2S_plugin(player_t *currentPlayer) {}
static void PlayC2S_book(player_t *currentPlayer) {}
static void PlayC2S_entity_nbt(player_t *currentPlayer) {}
static void PlayC2S_entity_interact(player_t *currentPlayer) {}
static void PlayC2S_generate_structure(player_t *currentPlayer) {}
static void PlayC2S_heartbeat(player_t *currentPlayer)
{
  currentPlayer->heartbeat = 1;
}
static void PlayC2S_lock_difficulty(player_t *currentPlayer) {}
static void PlayC2S_position(player_t *currentPlayer)
{
  currentPlayer->x = readDouble();
  currentPlayer->y = readDouble();
  currentPlayer->z = readDouble();
  currentPlayer->onground = readByte();
  currentPlayer->position_event = 1;
}
static void PlayC2S_position_rotation(player_t *currentPlayer)
{
  currentPlayer->x = readDouble();
  currentPlayer->y = readDouble();
  currentPlayer->z = readDouble();
  currentPlayer->yaw = readFloat();
  currentPlayer->pitch = readFloat();
  currentPlayer->onground = readByte();
  currentPlayer->position_event = 1;
}
static void PlayC2S_rotation(player_t *currentPlayer)
{
  currentPlayer->yaw = readFloat();
  currentPlayer->pitch = readFloat();
  currentPlayer->onground = readByte();
  currentPlayer->position_event = 1;
}
static void PlayC2S_ground_change(player_t *currentPlayer) {}
static void PlayC2S_move_vehicle(player_t *currentPlayer) {}
static void PlayC2S_steer_boat(player_t *currentPlayer) {}
static void PlayC2S_item_pick(player_t *currentPlayer) {}
static void PlayC2S_crafting_recipe(player_t *currentPlayer) {}
static void PlayC2S_toggle_fly(player_t *currentPlayer) {}
static void PlayC2S_player_action(player_t *currentPlayer) {}
static void PlayC2S_entity_action(player_t *currentPlayer)
{
  int32_t eid = readVarInt();
  if (eid == currentPlayer->player_id)
  {
    currentPlayer->entity_action_id = readVarInt();
    readVarInt();
    currentPlayer->entity_action_event = 1;
  }
}
static void PlayC2S_vehicle_input(player_t *currentPlayer) {}
// static void PlayC2S_steer_vehicle(player_t *currentPlayer) {}
static void PlayC2S_pong(player_t *currentPlayer) {}
static void PlayC2S_session_data(player_t *currentPlayer) {}
static void PlayC2S_displayed_recipe(player_t *currentPlayer) {}
static void PlayC2S_recipe_book(player_t *currentPlayer) {}
static void PlayC2S_anvil_item_name(player_t *currentPlayer) {}
static void PlayC2S_resourcepack(player_t *currentPlayer) {}
static void PlayC2S_advancement_tab(player_t *currentPlayer) {}
static void PlayC2S_trade(player_t *currentPlayer) {}
static void PlayC2S_beacon_effect(player_t *currentPlayer) {}
static void PlayC2S_hotbar_slot(player_t *currentPlayer) {}
static void PlayC2S_command_block(player_t *currentPlayer) {}
static void PlayC2S_minecart_command_block(player_t *currentPlayer) {}
static void PlayC2S_item_stack_create(player_t *currentPlayer) {}
static void PlayC2S_jigsaw_block(player_t *currentPlayer) {}
static void PlayC2S_structure_block(player_t *currentPlayer) {}
static void PlayC2S_sign_text(player_t *currentPlayer) {}
static void PlayC2S_swing_arm(player_t *currentPlayer)
{
  currentPlayer->swing_arm_animation = readVarInt();
  currentPlayer->swing_arm_event = 1;
}
static void PlayC2S_entity_spectate(player_t *currentPlayer) {}
static void PlayC2S_block_interact(player_t *currentPlayer) {}
static void PlayC2S_use_item(player_t *currentPlayer) {}
// static void PlayC2S_legacy_message_acknowledgement(player_t *currentPlayer){}
// static void PlayC2S_chat_preview(player_t *currentPlayer) {}
// static void PlayC2S_chat_message(player_t *currentPlayer){}
// static void PlayC2S_container_action(player_t *currentPlayer){}
// static void PlayC2S_crafting_grid(player_t *currentPlayer){}
void (*c2s_1_19_4_1_20_1[])(player_t *currentPlayer) = {
    &PlayC2S_confirm_teleport,
    &PlayC2S_block_nbt,
    &PlayC2S_difficulty,
    &PlayC2S_message_acknowledgement,
    &PlayC2S_command,
    &PlayC2S_signed_chat_message,
    &PlayC2S_session_data,
    &PlayC2S_client_action,
    &PlayC2S_settings,
    &PlayC2S_command_suggestions,
    &PlayC2S_container_button,
    &PlayC2S_container_click,
    &PlayC2S_close_container,
    &PlayC2S_channel,
    &PlayC2S_book,
    &PlayC2S_entity_nbt,
    &PlayC2S_entity_interact,
    &PlayC2S_generate_structure,
    &PlayC2S_heartbeat,
    &PlayC2S_lock_difficulty,
    &PlayC2S_position,
    &PlayC2S_position_rotation,
    &PlayC2S_rotation,
    &PlayC2S_ground_change,
    &PlayC2S_move_vehicle,
    &PlayC2S_steer_boat,
    &PlayC2S_item_pick,
    &PlayC2S_crafting_recipe,
    &PlayC2S_toggle_fly,
    &PlayC2S_player_action,
    &PlayC2S_entity_action,
    &PlayC2S_vehicle_input,
    &PlayC2S_pong,
    &PlayC2S_displayed_recipe,
    &PlayC2S_recipe_book,
    &PlayC2S_anvil_item_name,
    &PlayC2S_resourcepack,
    &PlayC2S_advancement_tab,
    &PlayC2S_trade,
    &PlayC2S_beacon_effect,
    &PlayC2S_hotbar_slot,
    &PlayC2S_command_block,
    &PlayC2S_minecart_command_block,
    &PlayC2S_item_stack_create,
    &PlayC2S_jigsaw_block,
    &PlayC2S_structure_block,
    &PlayC2S_sign_text,
    &PlayC2S_swing_arm,
    &PlayC2S_entity_spectate,
    &PlayC2S_block_interact,
    &PlayC2S_use_item,
    0,
    0,
    0,
    0,
    0};