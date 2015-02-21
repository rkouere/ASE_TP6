/* ------------------------------
   $Id: current.c,v 1.1 2009/10/30 10:15:19 marquet Exp $
   ------------------------------------------------------------

   Access to $CURRENT_VOLUME and $HW_CONFIG
   Philippe Marquet, Oct 2009
   
*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#ifndef _CURRENT
#define _CURRENT

extern int load_super(unsigned int vol);

#define DEFAULT_HW_CONFIG "hardware.ini"

unsigned int current_volume;
int load_current_volume();
char * get_hw_config();


#endif
