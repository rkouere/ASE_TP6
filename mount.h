/* ------------------------------
   $Id: mount.h,v 1.1 2009/11/16 05:38:07 marquet Exp $
   ------------------------------------------------------------

   Initialization and finalization
   Philippe Marquet, Nov 2009

*/

#ifndef _MOUNT_H_
#define _MOUNT_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "hardware.h"
#include "config.h"
#include "tools.h"
#include "mbr.h"
#include "super.h"
#include "colors.h"
#include "drive.h"
#include "hw.h"

/*  initialize hardware, mount the "current" volume
    configuration with the $HW_CONFIG and $CURRENT_VOLUME environment
    variables. 
 */
#define DEFAULT_HW_CONFIG "hardware.ini"

unsigned int current_volume;
void mount();
void boot();
void umount();
void load_current_volume ();
void boot();
#endif
