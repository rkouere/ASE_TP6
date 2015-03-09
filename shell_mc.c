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
  {"ping", ping,	"exit (without saving)"},
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
loop()
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
  int y/*, i */;

  while(1) {
    for(y = 0; y < 100000; y++){}; 
  }

}

static
void ping(){
  create_ctx(16380, &f_ping, (void*) NULL, "ping");
  create_ctx(16380, &f_ping, (void*) NULL, "ping2");
  create_ctx(16380, &f_ping, (void*) NULL, "ping3");
}


void loadBalancer(int current_cor) {
  int cor_with_max_ctx; /* the cor with the maximum of ctx */
  int i;
  if(!DEBUG)
  printf("load balancer started for cor %d\n", current_cor);
  /* we have finished the contexts we initialised, now, we need to steal some context from the other cores */
  while(1) {
    /* by default we take the first core as our starting point */
    cor_with_max_ctx = 0;
    /* TO DELETE : loop to wait and let me print things in a way that is manageable */
    /* we are going to go through all the current cores and check wich one has the highest number of cor */
    /* note: we start at one because we have already used core number one as our starting point */
    for(i = 0; i < CORE_NCORE; i++) {
      if(mega_ctx[i].nb_ctx > mega_ctx[cor_with_max_ctx].nb_ctx) {
	cor_with_max_ctx = i;
      }
    }
    /* we need to make sure that everything we are doing is "safe" */
    irq_disable();
    klock();
    /* we check that the lucky core has more that one task to do (there is no point to steal its only task, poor soul) */
    /* if it has, we take the next context it was supposed to deal with */
    if(mega_ctx[cor_with_max_ctx].nb_ctx > 1) {
      /* printf(BOLDCYAN"cor %d has taken a context from cor %d\n", current_cor, cor_with_max_ctx); */
      assert(mega_ctx[cor_with_max_ctx].ring_head->ctx_next);
      mega_ctx[current_cor].ring_head = mega_ctx[cor_with_max_ctx].ring_head;
      mega_ctx[cor_with_max_ctx].ring_head = mega_ctx[current_cor].ring_head->ctx_next;
      mega_ctx[current_cor].ring_head->ctx_next = mega_ctx[current_cor].ring_head;
      mega_ctx[current_cor].nb_ctx++;
      mega_ctx[cor_with_max_ctx].nb_ctx--;

    }
    irq_enable();
    kunlock();

  }
}





static void
empty_it()
{
    return;
}


/* this funciton will do two separate things:
   - start the functions we initiated in the main
   - "steal" functions from the other cores to balance the workload of each core
 */
void init() {
  int current_cor = _in(CORE_ID);

  _mask(1);
  yield();
  printf(BOLDGREEN"core %d has finished to execute its first contexts. It is now waiting to steal some\n"RESET, current_cor);
  /* if(current_cor == 0) */
  /*   testLoadBalancer(); */

  loadBalancer(current_cor);
  /* while(1); */
}

int
main(int argc, char **argv)
{
  int i;
  irq_disable();

  /* init hardware */
  if(init_hardware("core.ini") == 0) {
    fprintf(stderr, "Error in hardware initialization\n");
    exit(EXIT_FAILURE);
  }

  /* we initialse each of ctx of each core */
  for(i = 0; i < CORE_NCORE; i++) {
    mega_ctx[i].current_ctx = NULL;
    mega_ctx[i].ring_head = NULL;
    mega_ctx[i].return_ctx = NULL;
    mega_ctx[i].ctx_disque = NULL;
    mega_ctx[i].nb_ctx= 0;
  }

  /* Interreupt handlers */
  for(i=1; i<16; i++)
    IRQVECTOR[i] = empty_it;

  /* c'est la fonction appellé quand on lance le coeur */
  IRQVECTOR[0] = init;

  /* on initialise le rand robin */
  randRob = 0;
  create_ctx(16380, &loop, (void*) NULL, "loop");
  /* on dit que l'on veut mettre en route 3 coeur */
  for(i = 0; i < CORE_NCORE; i++) {
    printf("le coeur %d a %d contextes\n", i, mega_ctx[i].nb_ctx);
  }
  _out(CORE_STATUS, 0x7);

  /* the fonction that is called at each interuption from the clock */
  IRQVECTOR[TIMER_IRQ] = yield;

  /* we set-up the clock */
  _out(TIMER_PARAM, 128+64+32+8);
  _out(TIMER_ALARM, TIMER);
  irq_enable();


  init();
  /* testLoadBalancer(); */
  /* yield(); */

  /* abnormal end of dialog (cause EOF for xample) */
  do_xit();

  /* make gcc -W happy */
  exit(EXIT_SUCCESS);
}
