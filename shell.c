/* ------------------------------
   $Id: vm-skel.c,v 1.1 2002/10/21 07:16:29 marquet Exp $
   ------------------------------------------------------------

   Volume manager skeleton.
   Philippe Marquet, october 2002

   1- you must complete the NYI (not yet implemented) functions
   2- you may add commands (format, etc.)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drive.h"
#include "mbr.h"
#include "colors.h"
#include "mount.h"
#include "file.h"
#include "dir.h"
#include "sched.h"

/* ------------------------------
   command listh
   ------------------------------------------------------------*/

#define MAX_CMD_SIZE 64
unsigned int in_current_pwd;
char *current_pwd = "/";


/* used to store de arguments sent to each commands */

struct _cmd {
  char *name;
  void (*fun) (unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
  char *comment;
};

static void mkdir();
static void ls(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void dmpb(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void rm(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void xit(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void quit(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void none(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void cd(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void pwd(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void new_file(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void help(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]);


static struct _cmd commands [] = {
  {"mkdir", mkdir, 	"create a directory"},
  {"ls", ls, 	"list directories"},
  {"rm",rm,"remove entry"},
  {"cd",cd,"cd"},
  {"pwd",pwd,"pwd"},
  {"newf",new_file,"new file"},
  {"dmpb", dmpb,	"dump sector b"},
  {"exit", xit,	"exit (without saving)"},
  {"quit", quit,	"save the MBR and quit"},
  {"help", help,	"display this help"},
  {0, none, 		"unknown command, try help"}
} ;

/* ------------------------------
   dialog and execute 
   ------------------------------------------------------------*/

/* we split the command entered by the user so that we have something looking like argc/argv */
/* we doà not use global variables to make sure that everything we writte uses resources that will not cause mayhem while working on concurency */
unsigned int splitter(const char * entry, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  unsigned int nb = 0, tmp = 0;;
  while(*entry != '\0') {
    if(*entry == ' ') {
      split[nb++][tmp] = '\0';
      tmp = 0;
    }
    else if(*entry == EOF || *entry == '\n') {
      split[nb][tmp] = '\0';
      tmp = 0;
    }
    else {
      split[nb][tmp++] = *entry;
    }
    entry++;
  }
  return nb;
}

static void
execute(const char *name)
{
  struct _cmd *c = commands; 
  char split[MAX_CMD_SIZE][MAX_CMD_SIZE];    
  unsigned int nb = 0;
  nb = splitter(name, split);
  while (c->name && strcmp (split[0], c->name))
    c++;
  (*c->fun)(nb, split);
}

static void
loop(void)
{
  char name[MAX_CMD_SIZE];

  while (printf("> "), fgets (name, MAX_CMD_SIZE, stdin) != NULL) 
    execute(name) ;

}

/* ------------------------------
   command execution 
   ------------------------------------------------------------*/

/* static void print_error(char * msg) { */
/*   printf("%s\n", msg); */
/*   exit(EXIT_FAILURE); */
/* } */

static void
dmpb(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  unsigned char buffer[SECTOR_SIZE];
  int i, j;
  int b = atoi(split[1]);
  read_bloc(current_vol, b, buffer);
  printf("reading %d in vol %d\n", b, current_vol);
  printf("i.e. sector %d -- cylinder %d\n", sector_of_bloc(current_vol, b),cylinder_of_bloc (current_vol, b));
  for(i = 0; i < SECTOR_SIZE; i+=16){
    printf("%04d -", i);
    for(j = 0; j < 16; j++) {
      if(j == 8)
	printf(" -");
      printf(" %02x ", buffer[i+j]);
    }
    printf("\"");
    for(j = 0; j < 16; j++){
      if(buffer[i+j] >= 32 && buffer[i+j] < 128) {
	printf("%c", buffer[i+j]);
      }
      else
	printf(".");
    }  
    printf("\"\n");
  }
}

static void
mkdir(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  /* file_desc_t fd; */
  create_file(split[1], FILE_DIRECTORY);

}

static void
new_file(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  /* file_desc_t fd; */
  printf("Ifile %d is used to contain it\n", create_file(split[1], FILE_FILE));
}

static void
rm(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  /* file_desc_t fd; */
  printf("Ifile %d is used to contain it\n", delete_file(split[1])); 
}

/* pour le moment, nous ne listons que les dir */
static void
ls(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  file_desc_t _fd, *fd = &_fd;
  unsigned int ientry = 0; /* the entry index */

  struct entry_s entry;
  int current_path = inumber_of_path(current_pwd);

  open_ifile(fd, current_path);
  seek2_ifile(fd, 0);
  
  /* look after the right entry */
  while (read_ifile (fd, &entry, sizeof(struct entry_s)) != READ_EOF) {
    printf("%s\n", entry.ent_basename);
    ientry++;
  }

}

static void
cd(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
 
  if((inumber_of_path(split[1]))){
    in_current_pwd = inumber_of_path(split[1]);
    current_pwd = split[1];
  }else{
    printf(RED "Le dossier n'existe pas\n" RESET);
    
  }
}

static void
pwd(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]) {
  printf("%s\n",current_pwd);
}


static void
do_xit()
{
  exit(EXIT_SUCCESS);
}


static void
quit(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE])
{
  save_mbr();
  printf(GREEN "mbr saved (if needed :). Exiting program.\n" RESET);
  umount();
  exit(EXIT_SUCCESS);
}

static void
xit(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE])
{
  do_xit(); 
}

static void 
help(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE])
{
    struct _cmd *c = commands;
  
    for (; c->name; c++) 
	printf ("%s\t-- %s\n", c->name, c->comment);
}

static void
none(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE])
{
  printf ("%s\n", cmd[0]) ;
}

int
main(int argc, char **argv)
{
  /* dialog with user */ 
  /* init_master(); */
  /* load_mbr(); */


  in_current_pwd = inumber_of_path("/");
  current_pwd = "/";
  boot();
  mount();
  create_ctx(16380,&loop,NULL,"loop");
  start();
  /* abnormal end of dialog (cause EOF for xample) */
  do_xit();
  umount();
  /* make gcc -W happy */
  exit(EXIT_SUCCESS);
}
