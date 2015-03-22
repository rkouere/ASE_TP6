/* defini les info sur le disque */
#ifndef _HARDWARE
#define _HARDWARE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include "hardware.h"

#define SECTOR_SIZE 256
#define MAX_SECTOR 16
#define MAX_CYLINDER 16

#define HDA_CMDREG 0x3F6
#define HDA_DATAREGS 0x110
#define HDA_IRQ 14


#define DEBUG 0
#define FNNAME __func__


#define TIMER_CLOCK 240
#define TIMER_PARAM 244
#define TIMER_ALARM 248
#define TIMER 0xFFFFFFFE
#define TIMER_IRQ 2
#define TIMER_TICKS 8

#define CORE_STATUS 128
#define CORE_ID 294
#define CORE_IRQMAPPER 130
#define CORE_LOCK 152
#define CORE_UNLOCK 153
#define CORE_NCORE 5


unsigned int enable_irq;

void irq_enable();
void irq_disable();

#endif
