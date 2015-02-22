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
  int cor = _in(CORE_ID);
  _mask(1);  
  printf("coucou sur coeur avant yield sur coeur %d\n", cor);
  yield();
  printf("coucou sur coeur après yield sur coeur %d\n", cor);
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

  /* on dit que l'on veut mettre en route 6 coeur */
  
  _out(CORE_STATUS, 0x3);
  
  /* the fonction that is called at each interuption from the clock */
  IRQVECTOR[TIMER_IRQ] = yield;

  /* we set-up the clock */
  _out(TIMER_PARAM, 128+64+32+8);
  _out(TIMER_ALARM, 0xFFFFFFFF - 100);
  irq_enable();


  /* on doit lancer cette fonction car sinon on sortirait driectement du prog */
  yield();
  
  return 0;
}










void ex3() {
  int i = 0;
  int cor = _in(CORE_ID);
  /* Allows all IT */
  _mask(1);  
  while(1) {
    i = 0;
    /* on verifie que le lock est libre. Si c'est le cas, on execute la fonction. Sinon, on attent et on re-essait */
    if(_in(CORE_LOCK) == 1) {
      _out(CORE_LOCK, 0x2);
      printf("[%d] start\n", cor);
      while(i < 0xFFFFFF) {
	i++;
      }
      printf("[%d] end\n", cor);
      _out(CORE_UNLOCK, 0x1);
    }
    else {
      while(i < 0xFFFF) {
	i++;
      }
    }
    i = 0;
    while(i < 0xFFFF) {
      i++;
    }


  }
}

