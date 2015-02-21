#include "mount.h"
#include "inode.h"
#include "vol.h"
#include "mbr.h"
#include "super.h"



/* fonction qui m'aide a faire des tests */
void printVol() {
  int i;
  struct inode_s inode;
  load_current_volume();
  for(i = 1; i < mbr.mbr_vol[current_vol].vol_n_sector; i++) {
    read_inode(i, &inode);
    if(inode.in_type == FILE_FILE)
      printf("inode %d is file\n", i);
  }
}

void printInfoInode(unsigned int inumber) {
  struct inode_s inode;
  read_inode(inumber, &inode);
  printf("size");
}

int
main(int argc, char * argv[]) {
  mount();
  printf("%d \n", create_inode(FILE_FILE));
  printf("%d \n", create_inode(FILE_FILE));
  umount();
  return 0;
}
