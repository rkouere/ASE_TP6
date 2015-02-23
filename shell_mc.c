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
#include "sched.h"
#include "hardware.h"
#include "hw.h"

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

static void ps();
static void xit(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void quit(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void none(unsigned int nb, char cmd[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void help(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE]);
static void ping();


static struct _cmd commands [] = {
  {"exit", xit,	"exit (without saving)"},
  {"ping", xit,	"exit (without saving)"},
  {"quit", quit,	"save the MBR and quit"},
  {"ps", ps,	"get the list of activities on processors"},
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

  while (printf("shell_mc :|->  "), fgets (name, MAX_CMD_SIZE, stdin) != NULL) 
    execute(name) ;

}
static void
ps(){
  listJob();
}
/* ------------------------------
   command execution 
   ------------------------------------------------------------*/

/* static void print_error(char * msg) { */
/*   printf("%s\n", msg); */
/*   exit(EXIT_FAILURE); */
/* } */


static void
do_xit()
{
  exit(EXIT_SUCCESS);
}


static void
quit(unsigned int nb, char split[MAX_CMD_SIZE][MAX_CMD_SIZE])
{

  printf(GREEN "mbr saved (if needed :). Exiting program.\n" RESET);
  
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


static
void f_ping(void *args)
{
  int i, y;
  i = 50;
  while(i > 0) {
    for(y = 0; y < 100000; y++){}; 

  }

}

static
void ping(){
  create_ctx(16380, &f_ping, (void*) NULL, "ping");
}

void init() {
  _mask(1);
  yield();

}


static void
empty_it()
{
    return;
}

int
init_hard() {

  int i;
  irq_disable();

  /* init hardware */
  if(init_hardware("core.ini") == 0) {
    fprintf(stderr, "Error in hardware initialization\n");
    exit(EXIT_FAILURE);
  }

  /* Interreupt handlers */
  for(i=1; i<16; i++)
    IRQVECTOR[i] = empty_it;

  /* c'est la fonction appellé quand on lance le coeur */
  IRQVECTOR[0] = init;

  /* on initialise le rand robin */
  randRob = 0;



   /* on dit que l'on veut mettre en route 6 coeur */

  _out(CORE_STATUS, 0x3);

  /* the fonction that is called at each interuption from the clock */
  IRQVECTOR[TIMER_IRQ] = yield;

  /* we set-up the clock */
  _out(TIMER_PARAM, 128+64+32+8);
  _out(TIMER_ALARM, TIMER);
  irq_enable();


  /* on doit lancer cette fonction car sinon on sortirait driectement du prog */

  return 0;
}


int
main(int argc, char **argv)
{
  /* dialog with user */

  init_hard();
  create_ctx(16380, &f_ping, (void*) NULL, "ping1");
  create_ctx(16380, &f_ping, (void*) NULL, "ping2");

  /* create_ctx(16380, loop, (void*) NULL, "loop"); */
  loop();
  /* abnormal end of dialog (cause EOF for xample) */
  do_xit();

  /* make gcc -W happy */
  exit(EXIT_SUCCESS);
}
