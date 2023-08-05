
#include "wrapper.h"
#include "s2c.h"
#include "socketio.h"
#include "log.h"
#include "util.h"
// protocol look up table which maps packets ids to a definition, see util/protocol
const uint8_t s2c_1_19_4_1_19_4[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static readPacketVars_t readPacketVars = {.pktbytes = -1};

readPacketVars_t *readValues() { return (readPacketVars_t *)&readPacketVars; }
uint8_t readAllowed() { return (readPacketVars.bufferpos < readPacketVars.buffersize); }
void readStart(player_t *player) { readPacketVars.player = player; }
uint8_t readPeekByte()
{
  if (readPacketVars.bufferpos > sizeof(readPacketVars.buffer))
  {
    printl(LOG_ERROR, "read buffer overflow!\n");
    readPacketVars.player->remove_player = 1;
    return 0;
  }
  return readPacketVars.buffer[readPacketVars.bufferpos++];
}
uint8_t readByte()
{
  if (readPacketVars.pktbytes)
  {
    readPacketVars.pktbytes--;
    return readPeekByte();
  }
  printl(LOG_ERROR, "readByte called without packet or reading too much\n");
  return 0;
}
void readBuffer(char *buffer, size_t size)
{
  uint32_t i;
  for (i = 0; i < size; i++)
  {
    buffer[i] = readByte();
  }
}
uint16_t readShort()
{
  uint16_t ret = 0;
#if (ENDIAN)
  ret |= readByte() << 8;
  ret |= readByte();
#else
  ret |= readByte();
  ret |= readByte() << 8;
#endif
  return ret;
}
double readDouble()
{
  uint64_t c;
  double v;
  readBuffer((char *)&c, sizeof(uint64_t));
#if (ENDIAN)
  c = __bswap_64(c);
#endif
  memcpy(&v, &c, sizeof(double));
  return v;
}
float readFloat()
{
  uint32_t c;
  float v;
  readBuffer((char *)&c, sizeof(uint32_t));
#if (ENDIAN)
  c = __bswap_32(c);
#endif
  memcpy(&v, &c, sizeof(float));
  return v;
}
int64_t readLong()
{
  int64_t c;
  readBuffer((char *)&c, sizeof(int64_t));
#if (ENDIAN)
  c = __bswap_64(c);
#endif
  return c;
}
int32_t readVarInt()
{
  uint32_t value = 0;
  uint32_t position = 0;
  uint8_t currentByte;

  for (int i = 0; i < 5; i++)
  {
    currentByte = readByte();
    value |= (currentByte & 0x7F) << position;

    if ((currentByte & 0x80) == 0)
    {
      break;
    }
    position += 7;

    if (position >= 32)
    {
      printl(LOG_ERROR, "VarInt is too big");
      readPacketVars.player->remove_player = 1;
      return 0;
    }
  }

  return value;
}
void readPosition(int32_t *x, int32_t *y, int32_t *z)
{
  uint64_t pos;
  readBuffer((char *)&pos, sizeof(uint64_t));
#if (ENDIAN)
  pos = __bswap_64(pos);
#endif
  *x = (pos >> 38) & 0x3FFFFFF;
  *z = (pos >> 12) & 0x3FFFFFF;
  *y = pos & 0xFFF;
}

void readString(char *data, size_t maxlen)
{
  int32_t toread = readVarInt();
  uint32_t len = 0;
  while (toread--)
  {
    if (len < maxlen)
    {
      data[len++] = readByte();
    }
    else
    {
      readByte();
    }
  }
  data[len] = '\0';
}

// Writing utils
static sendPacketVars_t sendPacketVars;

void sendSwitchToGlobalBuffer()
{
  if (sendPacketVars.globalbuffer == NULL)
  {
    sendPacketVars.globalbuffersize = MEM_CHUNK_SIZE;
    if ((sendPacketVars.globalbuffer = U_malloc(sendPacketVars.globalbuffersize)) == NULL)
    {
      printl(LOG_ERROR, "malloc failed globalbuffer\n");
      return;
    }
  }
  sendPacketVars.switch_to_global_buffer = 1;
}
void sendRevertFromGlobalBuffer()
{
  sendPacketVars.switch_to_global_buffer = 0;
}

void sendSwitchToLocalBuffer(char *buf, size_t maxlen)
{
  sendPacketVars.localbuffer = buf;
  sendPacketVars.localbuffersize = maxlen;
  sendPacketVars.switch_to_localbuffer = 1;
}
size_t sendRevertFromLocalBuffer()
{
  size_t len = sendPacketVars.localbufferindex;
  sendPacketVars.localbuffer = 0;
  sendPacketVars.localbuffersize = 0;
  sendPacketVars.switch_to_localbuffer = 0;
  sendPacketVars.localbufferindex = 0;
  return len;
}
void sendExtByte(uint8_t b)
{
  if (sendPacketVars.switch_to_global_buffer)
  {
    if (sendPacketVars.globalbufferindex >= sendPacketVars.globalbuffersize)
    {
      uint8_t *buffer = NULL;
      // allocate the required memory
      sendPacketVars.globalbuffersize += MEM_CHUNK_SIZE;
      buffer = U_realloc(sendPacketVars.globalbuffer, sendPacketVars.globalbuffersize);
      if (buffer == NULL)
      {
        printl(LOG_ERROR, "Memory allocation failed globalbuffersize!\n");
        sendPacketVars.player->remove_player = 1;
        return;
      }
      // printl(LOG_INFO,"Buffer size: %ld %ld\n", sendPacketVars.globalbuffersize,sendPacketVars.globalbufferindex);
      sendPacketVars.globalbuffer = buffer;
    }
    sendPacketVars.globalbuffer[sendPacketVars.globalbufferindex++] = b;
  }
  else
  {
    sendMainByte(b);
  }
}

size_t sendGetGlobalBufferSpaceRemaining() { return sendPacketVars.globalbuffersize - sendPacketVars.globalbufferindex; }

size_t sendGetGlobalBufferIndex() { return sendPacketVars.globalbufferindex; }
void sendclearGlobalBuffer()
{

  sendPacketVars.globalbufferindex = 0;
}
size_t sendGlobalBuffer(player_t *player)
{
  size_t start_index = player->global_buffer_start_index;
  size_t space_left = sendGetGlobalBufferIndex();
  // copy the packet from the global buffer to the local buffer
  while (start_index != sendPacketVars.globalbufferindex && space_left)
  {
    if (start_index > sendPacketVars.globalbuffersize)
    {
      printl(LOG_ERROR, "global buffer copy overflow! %ld\n", start_index);
      return 0;
    }
    sendMainByte(sendPacketVars.globalbuffer[(start_index++)]);
    space_left--;
  }
  // player->global_buffer_start_index = start_index;
  //  free uneeded space if its more than MEM_CHUNK_THRESHOLD chunk sizes
  if (sendPacketVars.globalbuffer != NULL)
  {
    if ((ssize_t)((sendPacketVars.globalbuffersize - sendPacketVars.globalbufferindex) / MEM_CHUNK_SIZE) >= MEM_CHUNK_THRESHOLD)
    {
      uint8_t *buffer = NULL;

      // printl(LOG_INFO,"extra memory can be freed %ld new: %ld\n", (sendPacketVars.globalbuffersize - sendPacketVars.globalbufferindex), sendPacketVars.globalbufferindex);
      buffer = U_realloc(sendPacketVars.globalbuffer, sendPacketVars.globalbufferindex + 1);
      if (buffer == NULL)
      {
        printl(LOG_ERROR, "Memory de/allocation failed!\n");
        sendPacketVars.player->remove_player = 1;
        return 1;
      }
      sendPacketVars.globalbuffer = buffer;
      sendPacketVars.globalbuffersize = sendPacketVars.globalbufferindex + 1;
    }
  }
  if (!space_left)
  {
    return 1;
  }
  return 0;
}
size_t sendData(uint8_t *data, size_t buffersize)
{
  int sock = sendPacketVars.player->player_fd;
  size_t totalSent = 0;
  //split the packet in fragments
  while (totalSent < buffersize)
  {
    size_t remaining = buffersize - totalSent;
    size_t blockSize = remaining < MAX_SEND_FRAGMENT_SIZE ? remaining : MAX_SEND_FRAGMENT_SIZE;
    ssize_t r = U_send(sock, (char *)data + totalSent, blockSize, MSG_NOSIGNAL);

    if (r < 0)
    {
      //the issue with EAGAIN is that if we loop over it again with no delay, it will not work so for now just simply dispatch the packet later
      if (errno == EAGAIN)
      {
        if(sendPacketVars.player->packet){
          printl(LOG_ERROR,"Packet already exists! try again later\n");
          return totalSent;
        }
        //allocate the packet
        sendPacketVars.player->packet = U_malloc(remaining);
        if (sendPacketVars.player->packet == NULL)
        {
          printl(LOG_ERROR, "Memory allocation failed alt packet!\n");
          sendPacketVars.player->remove_player = 1;
          return totalSent;
        }
        //copy the packet
        sendPacketVars.player->packet_len = remaining;
        memcpy(sendPacketVars.player->packet, (char *)data + totalSent, remaining);
        sendPacketVars.player->packet_dispatch_flag = 1;
        return totalSent;
      }
      printl(LOG_ERROR, "could not send (%d) code %ld (%p %ld)\n", sock, r, data, totalSent);
      printl(LOG_ERROR, "errno: %s\n", strerror(errno));
      sendPacketVars.player->remove_player = 1;
      return totalSent;
    }
    totalSent += r;
  }
  return totalSent;
}
void sendStartPlayer(player_t *player)
{
  sendPacketVars.player = player;
  if (sendPacketVars.buffer == NULL)
  {
    sendPacketVars.buffersize = MEM_CHUNK_SIZE;
    if ((sendPacketVars.buffer = U_malloc(sendPacketVars.buffersize)) == NULL)
    {
      printl(LOG_ERROR, "malloc failed buffer\n");
      return;
    }
  }
  sendPacketVars.bufferindex = 0;
}
void sendMainByte(uint8_t byte)
{
  if (sendPacketVars.bufferindex >= sendPacketVars.buffersize)
  {
    uint8_t *buffer = NULL;
    // allocate the required memory
    sendPacketVars.buffersize += MEM_CHUNK_SIZE;
    buffer = U_realloc(sendPacketVars.buffer, sendPacketVars.buffersize);
    if (buffer == NULL)
    {
      printl(LOG_ERROR, "memory allocation failed buffer!\n");
      sendPacketVars.player->remove_player = 1;
      return;
    }
    // printl(LOG_INFO,"Buffer size: %ld\n", sendPacketVars.buffersize);
    sendPacketVars.buffer = buffer;
  }
  sendPacketVars.buffer[sendPacketVars.bufferindex++] = byte;
}
void sendDispatch()
{
  // send the data
  if (sendPacketVars.bufferindex != 0)
  {
#ifdef ONLINE_MODE
    if (sendPacketVars.player->encryption_recv_event)
    {
      if (sendPacketVars.player->remove_player)
      {
        return;
      }
      int ret = mbedtls_aes_crypt_cfb8(&sendPacketVars.player->aes_ctx, MBEDTLS_AES_ENCRYPT, sendPacketVars.bufferindex, sendPacketVars.player->iv_encrypt, sendPacketVars.buffer, sendPacketVars.buffer);
      if (ret != 0)
      {
        printl(LOG_ERROR, "aes encrypt failed %d\n", ret);
        return;
      }
    }
#endif /*ONLINE_MODE*/
    sendData(sendPacketVars.buffer, sendPacketVars.bufferindex);
  }
  // free uneeded space if its more than MEM_CHUNK_THRESHOLD chunk sizes
  if ((ssize_t)((sendPacketVars.buffersize - sendPacketVars.bufferindex) / MEM_CHUNK_SIZE) >= MEM_CHUNK_THRESHOLD)
  {
    uint8_t *buffer = NULL;

    // printl(LOG_INFO,"extra memory can be freed %ld new: %ld\n", (sendPacketVars.buffersize - sendPacketVars.bufferindex) / MEM_CHUNK_SIZE, sendPacketVars.bufferindex);
    buffer = U_realloc(sendPacketVars.buffer, sendPacketVars.bufferindex + 1);
    if (buffer == NULL)
    {
      printl(LOG_ERROR, "Memory de/allocation failed buffer!\n");
      sendPacketVars.player->remove_player = 1;
      return;
    }
    sendPacketVars.buffer = buffer;
    sendPacketVars.buffersize = sendPacketVars.bufferindex + 1;
  }
}
uint8_t sendAllowed() { return 1; }
void sendStart()
{
  if (sendPacketVars.packetbuffer == NULL)
  {
    sendPacketVars.packetsize = MEM_CHUNK_SIZE;
    if ((sendPacketVars.packetbuffer = U_malloc(sendPacketVars.packetsize)) == NULL)
    {
      printl(LOG_ERROR, "malloc failed packetbuffer\n");
      return;
    }
  }
  sendPacketVars.packetindex = 0;
}
void sendByte(uint8_t b)
{

  if (sendPacketVars.switch_to_localbuffer)
  {
    if (sendPacketVars.localbufferindex >= sendPacketVars.localbuffersize)
    {
      printl(LOG_ERROR, "local buffer no more space! %ld/%ld\n", sendPacketVars.localbufferindex, sendPacketVars.localbuffersize);
      sendPacketVars.player->remove_player = 1;
      return;
    }
    if (sendPacketVars.localbuffer == NULL)
    {
      printl(LOG_ERROR, "local buffer NULL\n");
      sendPacketVars.player->remove_player = 1;
      return;
    }
    sendPacketVars.localbuffer[sendPacketVars.localbufferindex++] = b;
  }
  else
  {
    if (sendPacketVars.packetindex >= sendPacketVars.packetsize)
    {
      uint8_t *buffer = NULL;
      // allocate the required memory
      sendPacketVars.packetsize += MEM_CHUNK_SIZE;
      buffer = U_realloc(sendPacketVars.packetbuffer, sendPacketVars.packetsize);
      if (buffer == NULL)
      {
        printl(LOG_ERROR, "Memory allocation failed!\n");
        sendPacketVars.player->remove_player = 1;
        return;
      }
      // printl(LOG_INFO,"Buffer size: %ld\n", sendPacketVars.packetsize);
      sendPacketVars.packetbuffer = buffer;
    }
    sendPacketVars.packetbuffer[sendPacketVars.packetindex++] = b;
  }
}
void sendPlayPacketHeader(size_t id)
{
  if (id < PLAYS2C_MAPPING_LEN && id > 0)
  {
    if (!(s2c_1_19_4_1_19_4[id] == 0xff))
    {
      sendByte(s2c_1_19_4_1_19_4[id]);
      return;
    }
    printl(LOG_ERROR, "Incorrect packet id ID:%ld player: %d\n", id, sendPacketVars.player->player_id);
  }
  sendPacketVars.player->remove_player = 1;
}
void sendBuffer(const char *buf, size_t len)
{
  for (size_t i = 0; i < len; i++)
  {
    sendByte(buf[i]);
  }
}
void sendInt(int32_t v)
{
  int32_t c;
#if (ENDIAN)
  c = __bswap_32(v);
#else
  c = v;
#endif
  sendBuffer((char *)&c, sizeof(int32_t));
}
void sendShort(int16_t v)
{
  uint16_t c;
#if (ENDIAN)
  c = __bswap_16(v);
#else
  c = v;
#endif
  sendBuffer((char *)&c, sizeof(int16_t));
}
void sendLong(int64_t v)
{
  int64_t c;
#if (ENDIAN)
  c = __bswap_64(v);
#else
  c = v;
#endif
  sendBuffer((char *)&c, sizeof(int64_t));
}
void sendDouble(double v)
{
  uint64_t c;
  memcpy(&c, &v, sizeof(uint64_t));
#if (ENDIAN)
  c = __bswap_64(c);
#endif
  sendBuffer((char *)&c, sizeof(uint64_t));
}
void sendFloat(float v)
{
  uint32_t c;
  memcpy(&c, &v, sizeof(uint32_t));
#if (ENDIAN)
  c = __bswap_32(c);
#endif
  sendBuffer((char *)&c, sizeof(uint32_t));
}
void sendRawData(char *dat, size_t len)
{
  for (size_t i = 0; i < len; i++)
  {
    sendExtByte(dat[i]);
  }
}
void sendDone()
{
  size_t packet_len = sendPacketVars.packetindex;
  // construct the packet header
  if (sendPacketVars.player->compression_event)
  {
    packet_len++;
  }
  // construct a varint for the 'Packet Length' field
  size_t value = packet_len;
  for (size_t i = 0; i < 4; i++)
  {
    if ((value & ~0x7F) == 0)
    {
      sendExtByte(value);
      break;
    }

    sendExtByte((value & 0x7F) | 0x80);
    value = (uint32_t)value >> 7;
  }
  // varint for the 'Data Length' field
  if (sendPacketVars.player->compression_event)
  {
    sendExtByte(0);
  }
  // send the packet
  sendRawData((char *)sendPacketVars.packetbuffer, sendPacketVars.packetindex);
  // free uneeded space if its more than MEM_CHUNK_THRESHOLD chunk sizes
  if ((ssize_t)((sendPacketVars.packetsize - sendPacketVars.packetindex) / MEM_CHUNK_SIZE) >= MEM_CHUNK_THRESHOLD)
  {
    uint8_t *buffer = NULL;

    // printl(LOG_INFO,"extra memory can be freed %ld new: %ld\n", (sendPacketVars.packetsize - sendPacketVars.packetindex) / MEM_CHUNK_SIZE, sendPacketVars.packetindex);
    buffer = U_realloc(sendPacketVars.packetbuffer, sendPacketVars.packetindex + 1);
    if (buffer == NULL)
    {
      printl(LOG_ERROR, "Memory de/allocation failed packet!\n");
      sendPacketVars.player->remove_player = 1;
      return;
    }
    sendPacketVars.packetbuffer = buffer;
    sendPacketVars.packetsize = sendPacketVars.packetindex + 1;
  }
}

void sendVarInt(int32_t value)
{
  for (int i = 0; i < 5; i++)
  {
    if ((value & ~0x7F) == 0)
    {
      sendByte(value);
      return;
    }

    sendByte((value & 0x7F) | 0x80);

    value = (uint32_t)value >> 7;
  }
}
void sendPosition(int32_t x, int32_t y, int32_t z)
{
  uint64_t pos = (((uint64_t)x & 0x3FFFFFF) << 38) | (((uint64_t)z & 0x3FFFFFF) << 12) | ((uint64_t)y & 0xFFF);
#if (ENDIAN)
  pos = __bswap_64(pos);
#endif
  sendBuffer((char *)&pos, sizeof(uint64_t));
}
void sendString(const char *str, size_t len)
{
  if (len == (size_t)(-1))
  {
    len = strnlen(str, MAX_STRING_SIZE);
  }
  if (len > MAX_STRING_SIZE)
  {
    printl(LOG_ERROR, "Send string failed! len(%ld) > %ld\n", len, (size_t)MAX_STRING_SIZE);
    sendPacketVars.player->remove_player = 1;
    return;
  }
  sendVarInt(len);
  for (uint32_t i = 0; i < len; i++)
  {
    sendByte(str[i]);
  }
}
// TODO: both of these are not correctly implemented but its random right?
void sendUUID(uint16_t seed)
{
  char stmp[16];
  memset(stmp, 0, sizeof(stmp));
  stmp[6] = 0x30;
  stmp[8] = 0x80;
  stmp[0] = (seed >> 8) & 0xff;
  stmp[1] = (seed)&0xff;
  sendBuffer(stmp, sizeof(stmp));
}
void sendUUIDString(uint16_t seed)
{
  static char const hexdigits_lower[16] = "0123456789abcdef";
  int i;
  char stmp[38];
  for (i = 0; i < 36; i++)
  {
    stmp[i] = '0';
  }
  stmp[36] = 0;
  stmp[8] = '-';
  stmp[13] = '-';
  stmp[18] = '-';
  stmp[23] = '-';
  stmp[14] = '3';
  stmp[19] = '8';
  stmp[0] = hexdigits_lower[((seed) >> 12 & 0xf)];
  stmp[1] = hexdigits_lower[((seed >> 8) & 0xf)];
  stmp[2] = hexdigits_lower[((seed >> 4) & 0xf)];
  stmp[3] = hexdigits_lower[((seed)&0xf)];
  sendString(stmp, strnlen(stmp, sizeof(stmp)));
}

void socketioCleanup()
{
  if (sendPacketVars.buffer)
  {
    U_free(sendPacketVars.buffer);
    sendPacketVars.buffer = NULL;
  }
  if (sendPacketVars.packetbuffer)
  {
    U_free(sendPacketVars.packetbuffer);
    sendPacketVars.packetbuffer = NULL;
  }
  if (sendPacketVars.globalbuffer)
  {
    U_free(sendPacketVars.globalbuffer);
    sendPacketVars.globalbuffer = NULL;
  }
}

void socketioLog()
{
  printl(LOG_INFO, "SEND: buffer: %ld packet: %ld globalbuffer: %ld\n", sendPacketVars.buffersize, sendPacketVars.packetsize, sendPacketVars.globalbuffersize);
}