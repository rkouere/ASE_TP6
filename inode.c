#include "inode.h"



void read_inode(unsigned int inumber, struct inode_s *inode) {
  read_bloc_n(current_vol, inumber, (unsigned char*)inode, sizeof(struct inode_s));
}




void write_inode(const unsigned int inumber, const struct inode_s *inode) {
  write_bloc_n(current_vol, inumber, (unsigned char*)inode, sizeof(struct inode_s));
}







/* permet de créer un inode */
unsigned int create_inode(enum file_type_e type) {
  struct inode_s inode;
  unsigned int inumber;

  /* on verifie que la taille d'un inode ne depasse pas celle d'un bloc */
  if(sizeof(inode) > BLOC_SIZE) {
    printf("La taille de l'ineoud est trop grande. Veuillez la changer (le problème vient probablement de la variable N_DIRECT.\n");
    exit(EXIT_FAILURE);
  }


  memset(&inode, 0, sizeof(struct inode_s));
  inode.in_type = type;
  inumber = new_bloc();
  /* on verifie que le disque n'est pas plein (new_bloc() renvois 0 si il est plein */
  if(!inumber) {
    printf("Le volume %d est plein. Il va faloir faire de la place pour pouvoir créer quoi que ce soit.\n", current_vol);
    return 0;
  }
  else {
    if(DEBUG)
      printDebug(FNNAME, "write inode (inode created)");
    write_inode(inumber, &inode);
  }
  return inumber;
}




int delete_inode(unsigned int inumber) {
  struct inode_s inode;
  unsigned int blocs[NNBPB];
  unsigned int d_blocs[NNBPB];
  int i = 0;
  read_inode(inumber, &inode);
  
  freeBlocs(inode.in_direct, N_DIRECT);

  /* on verifie qu'il y a des inodes */
  if(inode.in_indirect) {
    read_bloc_n(current_vol, inode.in_indirect, (unsigned char*)blocs, sizeof(blocs));
    /* printf("freeing in_indirect[] tab -> \n"); */
    freeBlocs(blocs, NNBPB);
    /* printf("freeing in_indirect -> \n"); */
    free_bloc(inode.in_indirect);
  }

  /* double indirection */
  if(inode.in_d_indirect) {
    /* on récupère le tableau de double indirection */
    read_bloc_n(current_vol, inode.in_d_indirect, (unsigned char*)blocs, sizeof(blocs));
    /* pour chaque tableau dans la double indirection, on les lit et on les libère */
    for(i=0; i<NNBPB; i++) {
      if(blocs[i]) {
	/* on lit le tablea */
	read_bloc_n(current_vol, blocs[i], (unsigned char*)d_blocs, sizeof(d_blocs));
	/* on libère le tableau */
	freeBlocs(d_blocs, NNBPB);
      }
    }

    /* on libère le tableau de double indirection */
    freeBlocs(blocs, NNBPB);
    /* on libère le bloc de la double indirection de l'inode */
    free_bloc(inode.in_d_indirect);
  }

  free_bloc(inumber);

  return 0;
}

/* permet de connaitre le numero de bloc associe au fbloc ieme bloc un endroit dans l'inoeud */
unsigned int my_vbloc_of_fbloc(unsigned int inumber, unsigned int fbloc, bool_t do_allocate) {
  struct inode_s inode;
  unsigned int blocs[NNBPB], d_blocs[NNBPB];
  /* struct free_bloc_s bloc; */
  int d_bloc = 0;
  int dd_bloc = 0;

  read_inode(inumber, &inode);
  /* si il est dans les liens direct, le numero du fbloc est celui contenu dans l'ineud */
  if(fbloc < N_DIRECT) {
    /* si on veut alouer le fbloc. Si il est vide, on lui donne un nouveau numero de bloc. */ 
    if(inode.in_direct[fbloc] == BLOC_NULL){
      if(do_allocate) {
      	/* printf("allocation in_direct -> \n"); */
	inode.in_direct[fbloc] = new_bloc();
	write_inode(inumber, &inode);
      }else return BLOC_NULL;
    } 
    /* printf("returning inode.in_direct[%d] = %d -> \n",fbloc, inode.in_direct[fbloc]); */
    return inode.in_direct[fbloc];
  }
  fbloc -= N_DIRECT;
  /* sinon, on change l'indice de flboc afin de voir si il est dans ndirect. */
  if(fbloc < NNBPB) {
    if(inode.in_indirect == BLOC_NULL) {
      if(do_allocate) {
	/* printf("allocation in_indirect -> \n"); */
	inode.in_indirect = new_bloc();
	/* printf("in_indirect -> %d\n",inode.in_indirect); */
	write_inode(inumber, &inode);
      }else return BLOC_NULL;
    }
    /* on rempli un tableau de int pour avoir les numéros de blocs */
    /* printf("reading in_indirect:%d -> \n", inode.in_indirect); */
    read_bloc_n(current_vol, inode.in_indirect, (unsigned char *)blocs, sizeof(blocs));

    if(do_allocate && blocs[fbloc] == BLOC_NULL) {
      /* printf("allocation in_indirect[%d] -> \n",fbloc); */
      blocs[fbloc] = new_bloc();
      write_bloc_n(current_vol, inode.in_indirect, (unsigned char *)blocs, sizeof(blocs));
    }else if(blocs[fbloc] == BLOC_NULL)
       return BLOC_NULL;
    /* printf("returning in_indirect[%d] = %d -> \n", fbloc, blocs[fbloc]); */
    return blocs[fbloc];
  }

  /* sinon, on change l'indice de flboc afin de voir si il est dans d_ndirect. */
  fbloc -= NNBPB;
  /* modif dans le test, si on veut tout faire d'un coup, c'est la taile NNBPB * NNBPB  */
  if(fbloc < NNBPB*NNBPB) {
    /* on trouve dans quelle sous structure le bloc se trouve */
    d_bloc = fbloc / NNBPB;
    dd_bloc = fbloc % NNBPB;

    if(inode.in_d_indirect == BLOC_NULL) {
      if(do_allocate) {
  	inode.in_d_indirect = new_bloc();
  	write_inode(inumber, &inode);
      }
      else
  	return BLOC_NULL;
    }
    /* on rempli un tableau de int pour avoir les numéros de blocs */
    read_bloc_n(current_vol, inode.in_d_indirect, (unsigned char *)blocs, sizeof(blocs));
    if(do_allocate && blocs[d_bloc] == BLOC_NULL) {
      blocs[d_bloc] = new_bloc();
      write_bloc_n(current_vol, inode.in_d_indirect, (unsigned char *)blocs, sizeof(blocs));
    }

    
    /* on lit le bloc contenant la double indirection et on fait ce qu'il faut pour alouer le bloc si necessaire. */
    read_bloc_n(current_vol, blocs[d_bloc], (unsigned char *)d_blocs, sizeof(d_blocs));
    if(d_blocs[dd_bloc] == BLOC_NULL) {
      if(do_allocate) {
	d_blocs[dd_bloc] = new_bloc();
	write_bloc_n(current_vol, blocs[d_bloc], (unsigned char *)d_blocs, sizeof(d_blocs));
      }
      else 
	return BLOC_NULL;
    }
    
    /* on retourne l'élément voulu dans le tableau de la deuxième branche */
    return d_blocs[dd_bloc];
  }
  else
    return 0;
} 
unsigned int vbloc_of_fbloc(unsigned int inumber, unsigned int fbloc) {
  return my_vbloc_of_fbloc(inumber, fbloc, FALSE);
}

unsigned int allocate_vbloc_of_fbloc(unsigned int inumber, unsigned int fbloc) {
  return my_vbloc_of_fbloc(inumber, fbloc, TRUE);
}
