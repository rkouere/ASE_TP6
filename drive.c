#include "drive.h"


static void go_to_sector(struct parameters *args);

/* utilise pour initialiser le hardware */
static void
empty_it()
{
  return;
}

void init_master() {
  unsigned int i;

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


  /*init semaphore du disque*/
}

void format_sector(unsigned int cylinder, unsigned int sector, unsigned int nsector, unsigned int value) {

  struct parameters args;
  args.cylinder = cylinder;
  args.sector = sector;
  args.n = nsector;

  go_to_sector(&args);
  _out(HDA_DATAREGS, (nsector >> 8) & 0xFF);
  _out(HDA_DATAREGS + 1, nsector & 0xFF);
  _out(HDA_DATAREGS + 2, (value >> 24) & 0xFF);
  _out(HDA_DATAREGS + 3, (value >> 16) & 0xFF);
  _out(HDA_DATAREGS + 4, (value >> 8) & 0xFF);
  _out(HDA_DATAREGS + 5, value & 0xFF);
  _out(HDA_CMDREG, CMD_FORMAT);
  wait_disque();
}

/* lit n nombre de secteur */
void read_sector_n(struct parameters *args) {


  unsigned int cylinder;
  unsigned int sector;
  const unsigned char *buffer;
  int n,i;

  cylinder=args->cylinder;
  sector = args->sector;
  buffer = args->buffer;
  n = args->n;


  if(n > SECTOR_SIZE) {
    printf("Vous essayez de lire un nombre de secteur plus grand que le nombre de secteur dispo\n");
    exit(EXIT_FAILURE);
  }


  create_ctx(16385,&go_to_sector,args,"go to sector context");



  sem_down(semaphore_disque);


  /* _out(HDA_DATAREGS, (cylinder >> 8) & 0xFF); */
  /* _out(HDA_DATAREGS + 1, cylinder & 0xFF); */
  /* _out(HDA_DATAREGS + 2, (sector >> 8) & 0xFF); */
  /* _out(HDA_DATAREGS + 3, sector & 0xFF); */
  /* _out(HDA_CMDREG, CMD_SEEK); */

  /* wait_disque(); */

  _out(HDA_DATAREGS, 1 & 0xFF);
  _out(HDA_CMDREG, CMD_READ);

  wait_disque();
  for(i = 0; i < n; i++)
    MASTERBUFFER[i] = buffer[i];
  sem_up(semaphore_disque);


}


void read_sector(unsigned int cylinder, unsigned int sector, const unsigned char *buffer) {


  struct parameters *str=calloc(1,sizeof(struct parameters));
  str->cylinder = cylinder;
  str->sector = sector;
  str->buffer = buffer;
  str->n = SECTOR_SIZE;

  create_ctx(16386,&read_sector_n,str,"contexte lecture disque");
  /* read_sector_n(cylinder, sector, buffer, SECTOR_SIZE); */
}

void write_sector_n(struct parameters *args) {

  int i;
  unsigned int cylinder;
  unsigned int sector;
  const char *buffer;
  int n;
  cylinder=args->cylinder;
  sector = args->sector;
  buffer = args->buffer;
  n = args->n;

  if(n > SECTOR_SIZE) {
    printf("Vous essayez de lire un nombre de secteur plus grand que le nombre de secteur dispo\n");
    exit(EXIT_FAILURE);
  }


  for(i = 0; i < SECTOR_SIZE; i++)
    MASTERBUFFER[i] = 0;

  create_ctx(16384,&go_to_sector,args,"go to sector context");

  sem_down(semaphore_disque);

  /* _out(HDA_DATAREGS, (cylinder >> 8) & 0xFF); */
  /* _out(HDA_DATAREGS + 1, cylinder & 0xFF); */
  /* _out(HDA_DATAREGS + 2, (sector >> 8) & 0xFF); */
  /* _out(HDA_DATAREGS + 3, sector & 0xFF); */
  /* _out(HDA_CMDREG, CMD_SEEK); */

  /* wait_disque(); */

  _out(HDA_DATAREGS, 0);
  _out(HDA_DATAREGS+1, 1 & 0xFF);
  for(i = 0; i < n; i++)
    MASTERBUFFER[i] = buffer[i];

  _out(HDA_CMDREG, CMD_WRITE);

  wait_disque();
  sem_up(semaphore_disque);

}


void write_sector(unsigned int cylinder, unsigned int sector, const unsigned char *buffer) {

  struct parameters *str=calloc(1,sizeof(struct parameters));
  str->cylinder = cylinder;
  str->sector = sector;
  str->buffer = buffer;
  str->n = SECTOR_SIZE;

  create_ctx(16384,&write_sector_n,str,"contexte ecriture disque");

  /* write_sector_n(cylinder, sector, buffer, SECTOR_SIZE); */
}



static void go_to_sector(struct parameters *args) {

  /* on verifie que les valeures passees en parametres sont les bonnes */
  if(args->cylinder > MAX_CYLINDER) {
    printf("Appel de la fonction go_to_sector avec un cylinder superieur a MAX_CYLINDER\n");
    exit(EXIT_FAILURE);
  }
  if(args->sector > MAX_SECTOR) {
    printf("Appel de la fonction go_to_sector avec un cylinder superieur a MAX_CYLINDER\n");
    exit(EXIT_FAILURE);
  }

  sem_down(semaphore_disque);
  printf("go to : %d %d %d %d\n",(args->cylinder >> 8) & 0xFF,args->cylinder & 0xFF, (args->sector >> 8) & 0xFF,args->sector & 0xFF);
  _out(HDA_DATAREGS, (args->cylinder >> 8) & 0xFF);
  _out(HDA_DATAREGS + 1, args->cylinder & 0xFF);
  _out(HDA_DATAREGS + 2, (args->sector >> 8) & 0xFF);
  _out(HDA_DATAREGS + 3, args->sector & 0xFF);
  _out(HDA_CMDREG, CMD_SEEK);

  wait_disque();

  sem_up(semaphore_disque);
}

void check_hda() {
  int sector_size, max_sector, max_cylinder;

  _out(HDA_CMDREG, CMD_DSKINFO);

  max_cylinder = _in(HDA_DATAREGS) << 8;
  max_cylinder += _in(HDA_DATAREGS + 1);

  if(max_cylinder != MAX_CYLINDER) {
    printf("Max Cylinder is %d. Please RECOMPILE with correct Max cylinder\n", max_cylinder);
    exit(EXIT_FAILURE);
  }

  max_sector = _in(HDA_DATAREGS + 2) << 8;
  max_sector += _in(HDA_DATAREGS + 3);
  if(max_sector != MAX_SECTOR) {
    printf("Max sector is %d. Please RECOMPILE with correct Max sector\n", max_sector);
    exit(EXIT_FAILURE);
  }

  sector_size = _in(HDA_DATAREGS+4) << 8;
  sector_size += _in(HDA_DATAREGS+5);
  if(sector_size != SECTOR_SIZE) {
    printf("Sector size is %d. Please RECOMPILE with correct sector size\n", sector_size);
    exit(EXIT_FAILURE);
  }



}
