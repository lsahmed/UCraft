#ifndef _CONFIG__ADV_H
#define _CONFIG__ADV_H

#define ENDIAN 1 // 1: little, 0:big

#define TICK_TIME_US 10000
#define MEM_CHUNK_SIZE 512 //Allocation granularity 
#define MEM_CHUNK_THRESHOLD 4 //Number of chunks to allocate before reallocating

#define MAX_SEND_FRAGMENT_SIZE 1024 //Number of bytes to send on the wire each call
#define SEND_PACKET_TIMEOUT 200 //Number of ticks before timing out
#define MAX_STRING_SIZE 1024
#define READBUFSIZE 4096

#define AUTH_HOST "sessionserver.mojang.com"
#define AUTH_HOST_PORT "443"
#define AUTH_TIMEOUT 100 //Number of ticks before timing out

#define PLAYER_BASE 0x4

#endif