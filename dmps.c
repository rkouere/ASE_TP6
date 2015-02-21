#include "drive.h"
#include <string.h>

static void
empty_it()
{
    return;
}

int
main() {
  unsigned int i;
  int y, z;
  unsigned char *buffer = malloc(SECTOR_SIZE * sizeof(unsigned char));

    
  /* init hardware */
  if(init_hardware("hardware.ini") == 0) {
    fprintf(stderr, "Error in hardware initialization\n");
    exit(EXIT_FAILURE);
  }

  /* Interreupt handlers */
  for(i=0; i<16; i++)
    IRQVECTOR[i] = empty_it;

  /* Allows all IT */
  _mask(1);
  check_hda();

  for(i = 0; i < MAX_CYLINDER; i++)
    for(y = 0; y < MAX_SECTOR; y++) {
       read_sector(i, y, buffer);
         for(z = 0; z < SECTOR_SIZE; z++) {
	   printf("%c", buffer[z]);
	 }
	 
    }
  return 0;
}

