#ifndef _MBR
#define _MBR
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tools.h"
#include "drive.h"
#include "colors.h"
#include "sched.h"

#define MAX_VOL 8
#define MBR_MAGIC 0xABC32


enum vol_type_e {VOLT_PR, VOLT_SND, VOLT_OTHER};

struct vol_s {
  unsigned int vol_first_cylinder;
  unsigned int vol_first_sector;
  unsigned int vol_n_sector;
  enum vol_type_e vol_type;
};

struct mbr_s {
  struct vol_s mbr_vol[MAX_VOL]; /* partitions */
  unsigned int mbr_n_vol; /* nbr de partitions */
  unsigned int mbr_magic; /* permet de savoir si le mbr a ete initialise */
};



void load_mbr();
void save_mbr();

struct mbr_s mbr;



#endif
