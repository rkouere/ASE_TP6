#ifndef _VOL
#define _VOL
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "tools.h"
#include "drive.h"
#include "colors.h"
#include "hw.h"
#include "sched.h"
#include "mbr.h"

/* !!!! n bloc = numero de bloc, et pas nombre de blocs */
void read_bloc(unsigned int vol, unsigned int nbloc, unsigned char *buffer);
void read_bloc_n(unsigned int vol, unsigned int nbloc, unsigned char *buffer, unsigned int size);

void write_bloc(unsigned int vol, unsigned int nbloc, const unsigned char *buffer);
void write_bloc_n(unsigned int vol, unsigned int nbloc, const unsigned char *buffer, unsigned int size);
void format_vol(unsigned int vol);
int sector_of_bloc(int vol, int b);
int cylinder_of_bloc(int vol, int b);
#endif
