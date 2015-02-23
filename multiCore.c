#include "hardware.h"
#include <stdio.h>
#include "hw.h"
#include <stdlib.h>
#include "sched.h"


/* verou */
/*
Deux prog :
1 affiche les coeurs qui affichent leur numéros 
2 : de tps en tps : pareil mais desfois, interuption qui affiche un truc
1 et 2 : non-deterministe

3 : on veut que un coeur, quand il execute l'affichage, il boucle un cetains temps et affiche ce qu'il a fini.
Il faut que le coeur ne soit aps interompu jusqu'à ce qu'il finisse.
On va utiliser un mecanisme de verou

-> prendre verou, faire la boucle, relacher le verou.
Prendre le verou, c'est lire CORE LOCK (si on a 1, il est verouillé donc c'est fini)
Si 0, on fait une attente active (une boucle) et on reessait de lire CORE_LOCK
Relacher le verou, c'est ecrire dans core unlock.

*/


int nbCor = 3;


void f_ping(void *args)
{
  int i, y;
  int cor = _in(CORE_ID);
  i = 50;
  while(i > 0) {
    printf(BOLDCYAN"on core %d\n", cor);
    printf("A");
    for(y = 0; y < 100000; y++){}; 
    printf("B");
    printf("C");
    i--;
  }
  printf(BOLDCYAN"ping has finished %d\n", cor);
}

void f_pang(void *args)
{
  int i, y;
  int cor = _in(CORE_ID);
  i = 10;
  while(i > 0) {
    printf(BOLDCYAN"on core %d\n", cor);
    printf("X");
    for(y = 0; y < 100000; y++){}; 
    printf("Y");
    printf("Z");
    printf("\n");
    i--;
  }
  printf(BOLDCYAN"pang has finished %d\n", cor);
}

void f_pong(void *args)
{
  int i, y;
  int cor = _in(CORE_ID);
  i = 100;
  while(i > 0) {
    printf(BOLDCYAN"on core %d\n", cor);
    printf("1");
    for(y = 0; y < 100000; y++){}; 
    printf("2");
    printf("3");
    i--;
  }

  printf(BOLDCYAN"pong has finished %d\n", cor);
}
void f_prong(void *args)
{
  int i, y;
  int cor = _in(CORE_ID);
  i = 100;
  while(i > 0) {
    printf(BOLDCYAN" on core %d\n", cor);
    printf("$$");
    for(y = 0; y < 100000; y++){}; 
    printf("%%");
    printf("!!");
    i--;
  }

  printf(BOLDCYAN"pong has finished %d\n", cor);
}

static void
empty_it()
{
    return;
}




void irqCoucou() {
  int cor = _in(CORE_ID);
  printf("coucou sur coeur %d\n", cor);
  _out(TIMER_ALARM, 0xFFFFFFFF - 100);
}

void init() {
  int cor = _in(CORE_ID), i;
  int cor_with_max_ctx; /* the cor with the maximum of ctx */
  /* used to find the number of context */
  /* I didn't find another way to deal with it */
  int nb_max_ctx; 

  _mask(1);  
  printf("coucou sur coeur avant yield sur coeur %d\n", cor);
  printf("CORE_NCORE = %d\n", CORE_NCORE);
  yield();
  /* we have finished the contexts we initialised, now, we need to steal some context from the other core */
  while(1) {
    cor_with_max_ctx = -1;
    nb_max_ctx = 1; /* we the core only has one ctx, there is no point to steal it */
    /* TO DELETE : loop to wait and let me print things in a way that is manageable */
    for(i = 0; i < 1000000; i++) {}
  /* we are going to go through all the current cores and check wich one has the highest number of cor */
    for(i = 0; i < CORE_NCORE; i++) {
      if(mega_ctx[i].nb_ctx > nb_max_ctx) {
	cor_with_max_ctx = i;
	nb_max_ctx = mega_ctx[i].nb_ctx;
      }
	
      printf(BOLDRED"cor %d a %d ctx\n"RESET, i, mega_ctx[i].nb_ctx);
    }

  }
}


int
main() {

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

  create_ctx(16380, &f_ping, (void*) NULL, "ping");
  create_ctx(16380, &f_pang, (void*) NULL, "pang");
  create_ctx(16380, &f_pong, (void*) NULL, "pong");
  create_ctx(16380, &f_prong, (void*) NULL, "prong");
  create_ctx(16380, &f_prong, (void*) NULL, "prong2");

  /* on dit que l'on veut mettre en route 3 coeur */
  
  _out(CORE_STATUS, CORE_NCORE);
  
  /* the fonction that is called at each interuption from the clock */
  IRQVECTOR[TIMER_IRQ] = yield;

  /* we set-up the clock */
  _out(TIMER_PARAM, 128+64+32+8);
  _out(TIMER_ALARM, TIMER);
  irq_enable();


  /* on doit lancer cette fonction car sinon on sortirait driectement du prog */
  init();
  return 0;
}
