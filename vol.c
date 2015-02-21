
#include "vol.h"

int sector_of_bloc(int vol, int b) {
  return (mbr.mbr_vol[vol].vol_first_sector + b) % MAX_SECTOR;
}

int cylinder_of_bloc(int vol, int b) {
  return mbr.mbr_vol[vol].vol_first_cylinder + ((mbr.mbr_vol[vol].vol_first_sector + b) / MAX_SECTOR);
}


void read_bloc_n(unsigned int vol, unsigned int nbloc, unsigned char *buffer, unsigned int size) {
  int cylinder, sector;
  struct parameters args;

  /* si on essait de lire un volume qui n'a pas été initialise */
  if(DEBUG) {
    printDebug(FNNAME, "");
    printf("vol = %d -- mbr.mbr_n_vol = %d -- nbloc = %d\n", vol, mbr.mbr_n_vol, nbloc);

  }
  if(vol > mbr.mbr_n_vol) {
    printf(BOLDRED "Ce volume n'existe pas sur le disque\n" RESET);
    exit(EXIT_FAILURE);
  }
  /* si on essait de lire un numero de bloc qui est plus grand que le nombre de bloc alloué dans la partition */
  if(nbloc > mbr.mbr_vol[vol].vol_n_sector) {
    printf(BOLDRED "Vous essayé de lire un nombre de bloque trop grand. Bye!\n" RESET);
    exit(EXIT_FAILURE);
  }
  cylinder = cylinder_of_bloc(vol, nbloc);
  sector = sector_of_bloc(vol, nbloc);
  if(DEBUG)
    printf(BOLDGREEN"[read bloc n]"RESET GREEN" cylinder = %d -- sector = %d\n"RESET, cylinder, sector);

  args.cylinder = cylinder;
  args.sector = sector;
  args.buffer = buffer;
  args.n = size;
  read_sector_n(&args);

}



void read_bloc(unsigned int vol, unsigned int nbloc, unsigned char *buffer) {
  read_bloc_n(vol, nbloc, buffer, SECTOR_SIZE);

}

void write_bloc_n(unsigned int vol, unsigned int nbloc, const unsigned char *buffer, unsigned int size) {
  int cylinder, sector;
  struct parameters args;

  if(DEBUG) {
    printDebug(FNNAME, "");
    printf("vol = %d -- mbr.mbr_n_vol = %d\n", vol, mbr.mbr_n_vol);
  }
  if(vol + 1 > mbr.mbr_n_vol) {
    printf(BOLDRED "Ce volume n'existe pas sur le disque\n" RESET);
    exit(EXIT_FAILURE);
  }
  
  if(nbloc > mbr.mbr_vol[vol].vol_n_sector) {
    printf(BOLDRED "Vous essayé de lire un nombre de bloque trop grand. Bye!\n" RESET);
    exit(EXIT_FAILURE);
  }
  
  cylinder = cylinder_of_bloc(vol, nbloc);
  sector = sector_of_bloc(vol, nbloc);
  if(DEBUG)
    printf(BOLDGREEN"[write bloc n]"RESET GREEN" cylinder = %d -- sector = %d\n"RESET, cylinder, sector);

  args.cylinder = cylinder;
  args.sector = sector;
  args.buffer = buffer;
  args.n = size;
  read_sector_n(&args);

  write_sector_n(&args);


}

void write_bloc(unsigned int vol, unsigned int nbloc, const unsigned char *buffer) {
  write_bloc_n(vol, nbloc, buffer, SECTOR_SIZE);

}

void format_vol(unsigned int vol) {
  int cylinder, sector, nsector;
  if(DEBUG)
    printDebug(FNNAME, "");
  if(vol +1 > mbr.mbr_n_vol) {
    printf(BOLDRED "Ce volume n'existe pas sur le disque\n" RESET);
    exit(EXIT_FAILURE);
  }
  nsector = mbr.mbr_vol[vol].vol_n_sector;
  cylinder = cylinder_of_bloc(vol, nsector);
  sector = sector_of_bloc(vol, nsector);
  format_sector(cylinder, sector, nsector, 0);
}
