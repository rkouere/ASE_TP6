#include "mkvol.h"


static void
empty_it()
{
    return;
}

int
main(int argc, char * argv[]) {
  unsigned int i;
  
  if(argc != 7) {
    printf("le programe prend les parametres suivants : \n");
    printf("-s [taille de la partition]\n");

  }

  /* init hardware */
  if(init_hardware("hardware.ini") == 0) {
    fprintf(stderr, "Error in hardware initialization\n");
    exit(EXIT_FAILURE);
  }

  /* Interreupt handlers */
  for(i=0; i<16; i++)
    IRQVECTOR[i] = empty_it;

  /* Allows all IT */
  _mask(1);
  /* FIN init hardware */


  /* new_vol.vol_first_cylinder = 1; */
  load_mbr();
  save_mbr();  
  /* mbr.mbr_vol[0] = new_vol; */
  /* test mbr */
  /* load_mbr(); */
  /* save_mbr(); */
  /* load_mbr(); */
  /* printf("fin save\n");   */
  /* FIN test mbr */

  return 0;
}


