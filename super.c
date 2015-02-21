#include "super.h"

/* fonciton utilise pour tester -- a ne pas confondre avec get last super :) */

void getCurrentSuper() {
  printf("--------INFO SUPER ------\n");
  printf("super first free bloc = %d\n", current_super.super_first_free_bloc);
  printf("super n free bloc = %d\n", current_super.super_n_free);
  printf("--------FIN INFO SUPER ------\n\n");

}

void init_vol(int vol) {
  struct super_s super;
  struct inode_s root;
  struct free_bloc_s free_block;
  int rootINumber = SUPER + 1;
  int free_size = mbr.mbr_vol[vol].vol_n_sector - 2;

  super.super_magic = SUPER_MAGIC;
  super.super_root = SUPER + 1;
  super.super_first_free_bloc = rootINumber + 1;
  super.super_n_free = free_size;

  if(DEBUG) {
    printDebug(FNNAME, "save super");
    printf("vol = %d\n", vol);
  }
  write_bloc_n(vol, SUPER, (unsigned char*)&super, sizeof(super));

  printf(BOLDGREEN"[init vol]"RESET GREEN" create root\n");
  memset(&root, 0, sizeof(struct inode_s));
  root.in_type = FILE_DIRECTORY;
  write_bloc_n(vol, rootINumber, (unsigned char*)&root, sizeof(root));

  free_block.fb_magic = FREE_BLOC_MAGIC;
  free_block.fb_n_free = free_size;
  free_block.fb_next = 0;

  printf(BOLDGREEN"[init vol]"RESET GREEN" save first free bloc n\n"RESET);
  write_bloc_n(vol, super.super_first_free_bloc, (unsigned char*)&free_block, sizeof(free_block));
  
}


int load_super(unsigned int vol) {

  current_vol = vol;
  printf(BOLDGREEN"[load super]"RESET GREEN" read bloc n\n");
  read_bloc_n(current_vol, SUPER, (unsigned char*)&current_super, sizeof(current_super));
  /* si le volume n'a pas ete initialise */
  printf(BOLDGREEN"current_super.super_magic = %x -- SUPER_MAGIC is supposed to be= %x\n", current_super.super_magic, SUPER_MAGIC);


  if(current_super.super_magic != SUPER_MAGIC)
    return 0;
  else
    return 1;
  /* return current_super.super_magic; */
}

void save_super() {
  if(DEBUG)
    printDebug(FNNAME, "writte super");
  write_bloc_n(current_vol, SUPER, (unsigned char*)&current_super, sizeof(current_super));
}

int new_bloc() {
  struct free_bloc_s free_bloc;
  /* int i; */
  int new = current_super.super_first_free_bloc;
  int resetBloc[SECTOR_SIZE/sizeof(int)];/* utilise pour intialiser tous les blocs à 0 */
  memset(resetBloc, 0, sizeof(resetBloc));
  /* for(i=0; i<SECTOR_SIZE/sizeof(int); i++) */
  /*   resetBloc[i] = 0; */

  if(current_super.super_n_free == 0)
    return 0;
  
  if(DEBUG)
    printf(BOLDGREEN"[new bloc]" RESET GREEN " create new bloc at %d\n", new);
  /* on recupere les info du premier bloc libre */
  read_bloc_n(current_vol, new, (unsigned char*)&free_bloc, sizeof(free_bloc));

  /* on initialise le nouveau bloc avec des 0 */
  /* printf(BOLDGREEN"[new bloc]" RESET GREEN " init bloc %d a zero\n", new); */
  /* write_bloc(current_vol, new, (unsigned char*)resetBloc); */

  /* si le groupe de bloc n'a qu'un bloc */
  if(free_bloc.fb_n_free == 1) {
    current_super.super_first_free_bloc = free_bloc.fb_next;
    current_super.super_n_free --;
    save_super();
    return new;
  }
  else {
    /* on met a jour le super */
    current_super.super_n_free--;
    current_super.super_first_free_bloc ++;
    /* on change les données qui seront dans le premier bloc libre */
    free_bloc.fb_n_free --;
    /* on met à jour le premier bloc libre */
    if(DEBUG)
      printDebug(FNNAME, "mise a jour info premier bloc libre");
    write_bloc_n(current_vol, current_super.super_first_free_bloc, (unsigned char*)&free_bloc, sizeof(free_bloc));
    if(DEBUG)
      printDebug(FNNAME, "save super\n");
    save_super();
    if(DEBUG) {
      printDebug(FNNAME, "return new");
      printf("new = %d -- current_super.super_first_free_bloc = %d\n", new, current_super.super_first_free_bloc);
    }
    return new;
  }
}

void free_bloc(int bloc) {
  struct free_bloc_s free_bloc;
  if(bloc != 0) {
    free_bloc.fb_n_free = 1;
    free_bloc.fb_next = current_super.super_first_free_bloc;
    write_bloc_n(current_vol, bloc, (unsigned char*)&free_bloc, sizeof(free_bloc));
    current_super.super_first_free_bloc = bloc;
    current_super.super_n_free ++;
  }
}

/* permets de freer plusieurs blocs d'un coup. particulièremeent utilse dans la bibliothèque de gestion d'inode */
void freeBlocs(unsigned int blocs[], unsigned int taille) {
  int i;
  for(i = 0; i < taille; i++){
    if(blocs[i])
      free_bloc(blocs[i]);
  }
}


int get_n_free_bloc() {
  return current_super.super_n_free;
}

