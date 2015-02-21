#ifndef _SUPER_H
#define _SUPER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inode.h"
#include "mbr.h"
#include "vol.h"
#include "drive.h"
#include "colors.h"


#define SUPER_MAGIC 0xDEADBEEF
#define FREE_BLOC_MAGIC 0xAD
#define SUPER 0

int current_vol;

struct super_s {
  unsigned int super_magic;
  unsigned int super_root; 
  unsigned int super_first_free_bloc;
  unsigned int super_n_free; /* nbr bloc libre */
};

extern struct super_s super;
struct super_s current_super;


struct free_bloc_s {
  unsigned int fb_magic;
  unsigned int fb_n_free;
  unsigned int fb_next; /* 0 pas de suivant */
};

void getCurrentSuper();



void init_vol(int vol);
int load_super(unsigned int vol);
void save_super();
int get_n_free_bloc();
int new_bloc();
void free_bloc(int bloc);
void freeBlocs(unsigned int * bloc, unsigned int taille);



#endif
