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

/*
Poursuivre :
On a un ordonancement sur une mcahine multicoeur et d'equilibrer la charge sur les coeur.
Klock protege des structure du noyaux
maintenant :
reprendre des sémaphore (producteur/consomateur) et les réimplementer.
Semaphore : appel depuis un context et possiblement bloquant (=on passe la main à un autre context)
1. reprendre l'implementaiton semaphore d'ASE et la mettre au multicoeur
2. implementation "spin-lock"
-> idée : je suis bloqué, j'attend un tout petit peut et quand j'ai fini j'espere que la ressource est débloqué (car je suis sur un autre coeur).
-> implémentation : "courte" attente active. On espere que pendant ce temps la, sur un autre coeur, un context progresse et va me liberer.
-> après l'attente : si ressource OK, tout va bien. Sinon on se bloque.
!!! cela ne marche que si on est sur un autre coeur.
 */


int nbCor = 3;


void f_ping(void *args)
{
  int i, y;
  i = 5000;
  while(i > 0) {
    for(y = 0; y < 100000; y++){}; 
    i--;
  }

}

void f_pang(void *args)
{
  int i, y;
  i = 1000;
  while(i > 0) {
    for(y = 0; y < 100000; y++){}; 
    i--;
  }

}

void f_pong(void *args)
{
  int i, y;
  i = 1000;
  while(i > 0) {
    for(y = 0; y < 100000; y++){}; 
    i--;
  }


}
void f_prong(void *args)
{
  int i, y;
  i = 100;
  while(i > 0) {
    for(y = 0; y < 100000; y++){}; 
    i--;
  }


}
void printYo() {
  int i = 0;
  int cor = _in(CORE_ID);
  while(1) {
    for(i = 0; i < 10000; i++) {
      printf("yoyoyoyoyoyoyoyoy\n");
      printf("printyo on cor %d\n", cor);
    }
  }
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

void testLoadBalancer() {
  irq_disable();
  klock();

  randRob = 0;
  create_ctx(16380, &printYo, (void*) NULL, "printYo1");
  randRob = 0;
  create_ctx(16380, &printYo, (void*) NULL, "printYo2");
  randRob = 0;
  create_ctx(16380, &printYo, (void*) NULL, "printYo3");
  randRob = 0;
  create_ctx(16380, &printYo, (void*) NULL, "printYo4");
  randRob = 0;
  create_ctx(16380, &printYo, (void*) NULL, "printYo5");

  irq_enable();
  kunlock();
  
}

void loadBalancer(int current_cor) {
  int cor_with_max_ctx; /* the cor with the maximum of ctx */
  int i;

  /* we have finished the contexts we initialised, now, we need to steal some context from the other cores */
  while(1) {
    /* by default we take the first core as our starting point */
    cor_with_max_ctx = 0;
    /* TO DELETE : loop to wait and let me print things in a way that is manageable */
    for(i = 0; i < 1000000; i++) {}
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
      printf(BOLDCYAN"cor %d has taken a context from cor %d\n", current_cor, cor_with_max_ctx);
      mega_ctx[current_cor].current_ctx = mega_ctx[cor_with_max_ctx].current_ctx->ctx_next;
      mega_ctx[cor_with_max_ctx].current_ctx->ctx_next = mega_ctx[current_cor].current_ctx->ctx_next;
      mega_ctx[current_cor].current_ctx->ctx_next = mega_ctx[current_cor].current_ctx;
      yield();
    }
    irq_enable();
    kunlock();

  }
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
  create_ctx(16380, &f_pang, (void*) NULL, "prong2");

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
  testLoadBalancer();
  yield();

  return 0;
}
