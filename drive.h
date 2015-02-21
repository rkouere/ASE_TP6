#ifndef _DRIVE_H
#define _DRIVE_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "hardware.h"
#include "hw.h"
#include "tools.h"


#include "sched.h"





struct sem_s *semaphore_disque;



void check_hda();
void read_sector(unsigned int cylinder, unsigned int sector, const unsigned char *buffer);
void read_sector_n(struct parameters *args);
void write_sector(unsigned int cylinder, unsigned int sector, const unsigned char *buffer);
void write_sector_n(struct parameters *args);
void format_sector(unsigned int cylinder, unsigned int sector, unsigned int nsector,unsigned int value);
void init_master();

#endif
