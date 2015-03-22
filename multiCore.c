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
#define MAX_CMD_SIZE 64



void printYo(void *args) {
  int i = 0;
  int cor = _in(CORE_ID);
  printf("alors\n");
  while(1) {
    for(i = 0; i < 1000000000; i++) {
    }
    printf("yoyoyoyoyoyoyoyoy\n");
    printf("printyo on cor %d\n", cor);
  }
}
void printYo2(void *args) {
  int i = 0;
  int cor = _in(CORE_ID);
  printf("alors\n");
  while(1) {
    for(i = 0; i < 10000; i++) {
    }
    printf("printyo2 on cor %d\n", cor);
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
  _out(TIMER_ALARM, 0xFFFFFFF0);
}

void start_ctx() {
  create_ctx(16380, &printYo, (void*) NULL, "printYo1");
  create_ctx(16380, &printYo, (void*) NULL, "printYo2");
  create_ctx(16380, &printYo2, (void*) NULL, "printYo3");
  create_ctx(16380, &printYo2, (void*) NULL, "printYo4");
  create_ctx(16380, &printYo, (void*) NULL, "printYo5");  
}



void init() {
  int current_cor = _in(CORE_ID);

  _mask(1);
  /* if(current_cor == 0) */
  /*   testLoadBalancer(); */
  /* start_ctx(); */
  printf(BOLDRED"back to ini\n"RESET);
  if(current_cor == 0) {
    /* printf("core n\n"); */
    while(1){};
  }  
  else {
    printf("[init] load bal started on cor %d\n", current_cor);
    create_ctx(16380, &printYo, (void*) NULL, "printYo1");
  }

}


int
main() {

  int i;

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

  randRob = 0;

  /* Interreupt handlers */
  for(i=1; i<16; i++)
    IRQVECTOR[i] = empty_it;

  /* c'est la fonction appellé quand on lance le coeur */
  IRQVECTOR[0] = init;

  /* on dit que l'on veut mettre en route 6 coeur */
  _out(CORE_STATUS, 0x7);
  
  /* on gere l'interuption */
  /* on fait en sorte que toute les interuption de type TIMER_IRQ sont redirige vers le coeur 2 */
  IRQVECTOR[TIMER_IRQ] = yield;
  for(i = 0; i < CORE_NCORE; i++)
    _out(CORE_IRQMAPPER + i, 0);

  
  for(i = 1; i < CORE_NCORE; i++)
    _out(CORE_IRQMAPPER + i, 1 << TIMER_IRQ);

  _out(TIMER_PARAM, 128+64+32+8);
  _out(TIMER_ALARM, 0xFFFFFFFF - 20);

  /* on doit lancer cette fonction car sinon on sortirait driectement du prog */
  init();
  
  return 0;
}


/* int */
/* main() { */

/*   int i; */
/*   /\* init hardware *\/ */
/*   if(init_hardware("core.ini") == 0) { */
/*     fprintf(stderr, "Error in hardware initialization\n"); */
/*     exit(EXIT_FAILURE); */
/*   } */

/*   /\* we initialse each of ctx of each core *\/ */
/*   for(i = 0; i < CORE_NCORE; i++) { */
/*     mega_ctx[i].current_ctx = NULL; */
/*     mega_ctx[i].ring_head = NULL; */
/*     mega_ctx[i].return_ctx = NULL; */
/*     mega_ctx[i].ctx_disque = NULL; */
/*     mega_ctx[i].nb_ctx= 0; */
/*   } */
/*   randRob = 0; */
/*   /\* create_ctx(16380, &loop, (void*) NULL, "loop"); *\/ */
/*   create_ctx(16380, &printYo, (void*) NULL, "printYo1"); */

/*   /\* Interreupt handlers *\/ */
/*   for(i=1; i<16; i++) */
/*     IRQVECTOR[i] = empty_it; */

/*   /\* c'est la fonction appellé quand on lance le coeur *\/ */
/*   IRQVECTOR[0] = init; */

/*   /\* on dit que l'on veut mettre en route 3 coeur *\/ */
/*   _out(CORE_STATUS, 0x7); */
  
/*   /\* on gere l'interuption *\/ */
/*   /\* on fait en sorte que toute les interuption de type TIMER_IRQ sont redirige vers le coeur 2 *\/ */
/*   IRQVECTOR[TIMER_IRQ] = yield; */
/*   for(i = 1; i < CORE_NCORE; i++) */
/*     _out(CORE_IRQMAPPER + i, 0); */

/*   for(i = 1; i < CORE_NCORE; i++) */
/*     _out(CORE_IRQMAPPER + i, 1 << TIMER_IRQ); */

/*   _out(TIMER_PARAM, 128+64+32+8); */
/*   _out(TIMER_ALARM, 0xFFFFFFFF - 20); */

/*   /\* on doit lancer cette fonction car sinon on sortirait driectement du prog *\/ */
/*   init(); */


/*   return 0; */
/* } */
