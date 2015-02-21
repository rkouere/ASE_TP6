#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "drive.h"
#include "mount.h"
#include "colors.h"
#include "sched.h"
#include "hw.h"

static void
empty_it()
{
    return;
}


int
main() {
  int i, y;
  boot();
  mount();
  printf(BOLDGREEN"[frmt] start formating\n"RESET);
  for(i = 0; i < MAX_CYLINDER; i++) {
    printf(GREEN"formating %d, %d\r"RESET, i, 0);fflush(stdout); 
    for(y = 0; y < MAX_SECTOR; y++)
      format_sector(i, y, SECTOR_SIZE, 0);
  }
  return 0;
}
