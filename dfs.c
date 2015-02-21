#include "super.h"
#include "mount.h"

static void
list()
{
  int n_vol, f_vol, a_vol, p_vol;
  double pct;

  printf(MAGENTA "%12s - %14s - %12s - %16s - %4s\n" RESET, "partition n.",  "Size", "Used", "Available", "Use");
  
  
  n_vol = mbr.mbr_vol[current_volume].vol_n_sector * SECTOR_SIZE;
  f_vol = get_n_free_bloc() * SECTOR_SIZE;
  a_vol = n_vol - f_vol;
  pct = (double)a_vol/(double)n_vol;
  p_vol = pct*100;

  printf("%12d   %14d   %12d   %16d   %4d%%\n", current_volume, n_vol, a_vol, f_vol, p_vol);

}

int main (int argc, char **argv) {
  
  /* init_master(); */
  /* load_mbr(); */
  /* load_current_volume(); */
  mount();
  
  list();

  return 0;
}
