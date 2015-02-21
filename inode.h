/* ------------------------------
   $Id: inode.h,v 1.2 2009/11/17 09:44:12 marquet Exp $
   ------------------------------------------------------------

   Inode manipulation interface.
   Philippe Marquet, Nov 2009

*/

#ifndef _INODE_H_
#define _INODE_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "tools.h"
#include "hw.h"
#include "super.h"
#include "mount.h"


#define BLOC_SIZE       SECTOR_SIZE  
#define DATA_BLOC_SIZE  BLOC_SIZE
#define N_DIRECT 10



/* Nombre Numéro Bloc Par Bloc = nombre de numéros de blocs que l'on peut stoquer dans un bloc de type indirect ou n_indirect. */
#define NNBPB (BLOC_SIZE/sizeof(int))
/* #define NNBPB 1 */

/* different kind of files */
enum file_type_e {FILE_FILE = 0xAA, FILE_DIRECTORY = 0xAB, FILE_SPECIAL = 0xAC};

/* inode */
struct inode_s {
  enum file_type_e in_type;
  unsigned int ind_size;      /* in octets */
  unsigned int in_direct[N_DIRECT];
  unsigned int in_indirect;
  unsigned int in_d_indirect;

};

/* a bloc full of zeros */
#define BLOC_NULL 0

/* inodes i/o */
void read_inode (unsigned int inumber, struct inode_s *inode);
void write_inode (const unsigned int inumber, const struct inode_s *inode);

/* inodes creation/deletion.
   The creation return a inumber */
unsigned int create_inode(enum file_type_e type); 
int delete_inode(unsigned int inumber);

/* return the bloc index on the volume of a given bloc index in a
   file.  
   Return BLOC_NULL for a bloc full of zeros */ 
unsigned int vbloc_of_fbloc(unsigned int inumber, unsigned int fbloc); 

/* allocate and return a bloc on the volume (in order to write in the
   file).
   This may imply indirect and d_indirect bloc creation.
   Return BLOC_NULL if no allocation was possible. */
unsigned int allocate_vbloc_of_fbloc(unsigned int inumber, unsigned int bloc);

#endif
