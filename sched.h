/*
TP Semaphore, réalisé par Yaker Mahieddine
 */
#ifndef _SCHED_H
#define _SCHED_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>

#include "hardware.h"
#include "hw.h"
#include "colors.h"

#define CTX_MAGIC 0xBABE

#define LOOP 1000000000
#define N 100
<<<<<<< HEAD
#define TIMER 0xFFFFFFFE
=======
>>>>>>> 69860409d1050032fc9e0ed772559c43e4901470
int next_index,cpt;

int randRob; /* used for affecting a context to a core */

typedef void (func_t) (void *);
/* typedef void (irq_handler_func_t)(void); */


typedef enum {  CTX_RDY,/*0 : context pret en attente*/
                CTX_EXQ,/*1 : context en cours d'execution*/
                CTX_STP,/*2 : context stoppé*/
                CTX_DISQUE,/*3 : context en attente du disque */
                CTX_END/*4 : context terminé */
} state_e;




struct ctx_s {
  char *ctx_name;
  void* ctx_rsp;
  void* ctx_rbp;
  unsigned ctx_magic;
  func_t* ctx_f;
  struct parameters* ctx_arg;
  state_e ctx_state;
  char* ctx_stack; /* adresse la plus basse de la pile */
  unsigned ctx_size;
  struct ctx_s* ctx_next;
  struct ctx_s* ctx_sem_next;
};

struct parameters{
  unsigned int cylinder;
  unsigned int sector;
  const unsigned char *buffer;
  int n;
};

struct parameters_m{
  unsigned int cylinder;
  unsigned int sector;

};


struct sem_s {
  char* sem_name;
  int sem_cpt;
  int init_cpt;
  struct ctx_s* sem_head;
  struct ctx_s* sem_last;
};


struct mega_ctx_s {
  struct ctx_s* current_ctx;
  struct ctx_s* ring_head;
  struct ctx_s* return_ctx;
  struct ctx_s* ctx_disque;
  unsigned int nb_ctx;
} ;

/* we alocate one contexte per core */
struct mega_ctx_s mega_ctx[CORE_NCORE];


typedef struct object_s{
	int value;
} object_t;






object_t stack[N];


int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, struct parameters *args,char * name);
int create_ctx(int size, func_t f, struct parameters * args,char * name);
void start_current_ctx();
void del_ctx(struct ctx_s *ctx);
void print_ctx(struct ctx_s *ctx);
void print_pile_ctx();
void start();
void switch_to_ctx(struct ctx_s *new_ctx);
void yield();
void listJob();
void wait_disque();
void reset_ctx_disque();
void sem_init(struct sem_s *sem, unsigned int val, char* name);
void sem_up(struct sem_s *sem);
void sem_down(struct sem_s *sem);
void klock();
void kunlock();



#endif
