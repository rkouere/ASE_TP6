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


#define DEBUG 1
#define FNNAME __func__


#define TIMER_CLOCK 0xF0
#define TIMER_PARAM 0xF4
#define TIMER_ALARM 0xF8
#define TIMER_IRQ 2
#define TIMER_TICKS 8

#define CORE_STATUS 0x80
#define CORE_ID 0x126
#define CORE_IRQMAPPER 0x82
#define CORE_LOCK 0x98
#define CORE_UNLOCK 0x99
#define CORE_NCORE 8


unsigned int enable_irq;

void irq_enable();
void irq_disable();

#endif
