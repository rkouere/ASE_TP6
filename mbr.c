#include "mbr.h"


void load_mbr() {
  char c;
  if(sizeof(struct mbr_s) > SECTOR_SIZE) {
    printf("La taille du mbr est superieur à la taille d'un secteur. Ce disque ne peut pas être utilisé. Bye!\n");
    exit(EXIT_FAILURE);
  }

  /* lit le premier secteur et le met dans le mbr */
  read_sector(0, 0, (unsigned char *)&mbr);

  if(DEBUG) {
    printDebug(FNNAME, "");
    printf("mbr.mbr_magic = %x -- MBR_MAGIC should be %x\n", mbr.mbr_magic, MBR_MAGIC);
  }
  if(mbr.mbr_magic != MBR_MAGIC) {
    mbr.mbr_n_vol = 0;
    mbr.mbr_magic = MBR_MAGIC;
    printf(BOLDRED "The hard drive is not formated. Do you want to continue ?\n" RESET);
    printf(BOLDWHITE "y" RESET " = Yes - " BOLDWHITE "anything else " RESET " = No\n");
    c = getchar();
    if(c != 'y') {
      printf("Operation aborted\n");
      exit(EXIT_SUCCESS);
    }
    else save_mbr();
  }

}

void save_mbr() {
  write_sector(0, 0, (unsigned char *)&mbr);
}




