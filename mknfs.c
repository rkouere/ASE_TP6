#include "super.h"
#include "mount.h"
#include "tools.h"
#include <errno.h>

void
mknfs_load_current_volume ()
{
  char* current_volume_str;
  current_volume_str = getenv("CURRENT_VOLUME");
  ffatal(current_volume_str != NULL, "no definition of $CURRENT_VOLUME"); 

  errno = 0;
  current_volume = strtol(current_volume_str, NULL, 10);
  ffatal(!errno, "bad value of $CURRENT_VOLUME %s", current_volume_str);

}

int
main(int argc, char *argv[]) {
  /* struct super_s super; */

  if(DEBUG)
    printDebug(FNNAME, "init master");

  /* init_master(); */
  if(DEBUG)
    printDebug(FNNAME, "load mbr");
  /* boot(); */
  mount();

  if(DEBUG)
    printDebug(FNNAME, "init vol");
  init_vol(current_volume);
  printDebug(FNNAME, "save mbr");
  save_mbr();
  return 0;
}
