#include "wrapper.h"
#include "player.h"
#include "https.h"
#include "log.h"

static player_t *playerListHead = NULL;          // connected players list
static playerd_t *playerDisconnectedHead = NULL; // disconnected players list

static fd_set masterset;

fd_set *playerGetSet() { return &masterset; }
void playerPushDisconnected(int32_t playerid, char *playername, size_t len)
{
  if (playername == NULL)
  {
    return;
  }
  playerd_t *element = U_calloc(1, sizeof(playerd_t));
  if (element == NULL)
  {
    return;
  }
  char *name = U_calloc(1, len);
  if (name == NULL)
  {
    U_free(element);
    return;
  }
  strncpy(name, playername, len);
  element->name = name;
  element->id = playerid;
  element->next = playerDisconnectedHead;
  playerDisconnectedHead = element;
}
playerd_t *playerPopDisconnected()
{
  if (playerDisconnectedHead == NULL)
  {
    return NULL;
  }
  playerd_t *temp = playerDisconnectedHead;
  playerDisconnectedHead = playerDisconnectedHead->next;
  return temp;
}
player_t *playerGetHead() { return playerListHead; }
size_t playerGetCount()
{
  size_t currentPlayers = 0;
  player_t *player = playerListHead;
  while (player != NULL)
  {
    currentPlayers++;
    player = player->next;
  }
  return currentPlayers;
}
size_t playerGetActiveCount()
{
  size_t currentActivePlayers = 0;
  player_t *player = playerListHead;
  while (player != NULL)
  {
    if (player->active)
    {
      currentActivePlayers++;
    }
    player = player->next;
  }
  return currentActivePlayers;
}

player_t *playerGetId(int32_t player_id)
{
  player_t *player = playerListHead;
  while (player != NULL)
  {
    if (player->player_id == player_id)
    {
      return player;
    }
    player = player->next;
  }
  return NULL;
}
player_t *playerAdd(uint32_t player_fd)
{
  player_t *player = U_calloc(1, sizeof(player_t));
  if (player == NULL)
  {
    return NULL;
  }
  player->player_fd = player_fd;
  FD_SET(player_fd, &masterset);
  player->player_id = player_fd + PLAYER_BASE;
  player->next = playerListHead;
  playerListHead = player;
  return player;
}
int playerCheckName(player_t *player)
{
  for (size_t i = 0; i < strnlen(player->playername, sizeof(((player_t *)0)->playername)); i++)
  {
    if (player->playername[i] <= '/')
    {
      return 1;
    }
    if (player->playername[i] >= ':' && player->playername[i] <= '@')
    {
      return 1;
    }
    // remove '_' since its valid
    if ((player->playername[i] >= '[' && player->playername[i] <= '`') && player->playername[i] != '_')
    {
      return 1;
    }
    if (player->playername[i] >= '{')
    {
      return 1;
    }
    // make it lower case
    if (player->playername[i] >= 'A' && player->playername[i] <= 'Z')
    {
      player->playername[i] += 32;
    }
  }
  return 0;
}
int playerCheckDuplicate(player_t *player)
{
  player_t *current = playerListHead;
  while (current != NULL)
  {
    if (current != player && strncmp(current->playername, player->playername, sizeof(((player_t *)0)->playername)) == 0)
    {
      return 1;
    }
    current = current->next;
  }
  return 0;
}
uint8_t playerRemove(player_t *player)
{
  if (playerListHead == NULL)
  {
    return 1;
  }
  if (player == NULL)
  {
    return 1;
  }
#ifdef ONLINE_MODE
  mbedtls_aes_free(&player->aes_ctx);
#endif /*ONLINE_MODE*/
#ifdef ONLINE_MODE_AUTH
  httpsFreePlayer(player);
  if (player->texture_value)
  {
    U_free(player->texture_value);
  }
  if (player->texture_signature)
  {
    U_free(player->texture_signature);
  }
#endif /*ONLINE_MODE_AUTH*/
  U_shutdown(player->player_fd, SHUT_RDWR);
  U_close(player->player_fd);
  if(player->packet){
    U_free(player->packet);
  }
  FD_CLR(player->player_fd, &masterset);
  if (player->active)
  {
    playerPushDisconnected(player->player_id, player->playername, sizeof(((player_t *)0)->playername));
    gamePlayerLeft(player);
    printl(LOG_INFO, "player %s removed\n", player->playername);
  }
  if (playerListHead == player)
  {
    playerListHead = player->next;
    U_free(player);
    return 0;
  }
  player_t *current = playerListHead;
  while (current != NULL && current->next != player)
  {
    current = current->next;
  }
  if (current == NULL)
  {
    return 1;
  }
  current->next = player->next;
  U_free(player);
  return 0;
}
void playerCleanup()
{
  for (player_t *player = playerGetHead(); player != NULL;)
  {
    player_t *temp = player->next;
    playerRemove(player);
    if (playerGetHead() == NULL)
    {
      break;
    }
    player = temp;
  }
  for (playerd_t *disconnected = playerPopDisconnected(); disconnected != NULL; disconnected = playerPopDisconnected())
  {
    U_free(disconnected->name);
    U_free(disconnected);
  }
}