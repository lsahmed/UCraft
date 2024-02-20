
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>

#include "UCraft.h"
static uint8_t cleanup_flag;

void cleanup()
{
  // clean up flag
  cleanup_flag = 1;
}
int main(int argc, char const *argv[])
{
  signal(SIGINT, cleanup);
  UCraftStart(&cleanup_flag);
}