#include "sched.h"

void print_ctx(struct ctx_s *ctx){
  printf("|> ");
  printf(BOLDWHITE"%s with state %d\n"RESET,ctx->ctx_name,ctx->ctx_state);
}

void print_pile_ctx(){
  irq_disable();
  int currentCor = _in(CORE_ID);
  unsigned int i;
  struct ctx_s *ctx;
  assert(mega_ctx[currentCor].ring_head != NULL);
  ctx = mega_ctx[currentCor].ring_head;

  printf(BOLDGREEN"\n=============================\n"RESET);
  printf(BOLDGREEN"%d ctx\n"RESET,mega_ctx[currentCor].nb_ctx);
  printf("\n>><<\n");
  for(i=0;i<mega_ctx[currentCor].nb_ctx;i++){
    print_ctx(ctx);
    ctx = ctx->ctx_next;
  }
  printf(BLUE"\n=============================\n"RESET);
  irq_enable();
}

void listJob(){
  int i;
  unsigned int j;
  struct mega_ctx_s *mctx;
  struct ctx_s *ctx;
  /* Boucle parcourant les processeurs */
  for(i=0;i<CORE_NCORE;i++){
    mctx = &mega_ctx[i];
    printf(BOLDGREEN"\nProcésseur n°%d,%d contexts: \n"RESET,i,mctx->nb_ctx);
    if(!mctx->ring_head){/*Si aucune tache sur ce processeurs on affiche ce message et passe au processeur suivant*/
      printf(BOLDWHITE"\tAucun context sur ce processeur\n"RESET);
      continue;
    }
    ctx = mctx->ring_head;
    /*Boucle parcourant les contexts de la ring_head*/
    for(j=0;j<mctx->nb_ctx;j++){
      printf("\t%s\n",ctx->ctx_name);
      ctx= ctx->ctx_next;
    }
  }
}

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f,struct parameters * args,char *name){

  ctx->ctx_stack = (char*) calloc(stack_size,sizeof(char));
  if ( ctx->ctx_stack == NULL) return 1;
  int corToInit = randRob%CORE_NCORE;

  printf(BOLDGREEN"[create_ctx] corToInit = %d\n"RESET, corToInit);

  ctx->ctx_name = name;
  ctx->ctx_state = CTX_RDY;
  ctx->ctx_size = stack_size;
  ctx->ctx_f = f;
  ctx->ctx_arg = args;

  /* QUESTION : comment peut-on initialiser le rsp et le rbp ici car nous n'avons pas encore sauvegardé les contexte ? */
  ctx->ctx_rsp = &(ctx->ctx_stack[stack_size-16]);
  ctx->ctx_rbp = ctx->ctx_rsp;
  /* fin de ma question */
  ctx->ctx_magic = CTX_MAGIC;
  ctx->ctx_next = ctx;
  mega_ctx[randRob%CORE_NCORE].nb_ctx++;

  if(DEBUG)
    printf(BOLDBLUE"\n%d ) creating ctx %s on \n"RESET,mega_ctx[corToInit].nb_ctx,name);

  return 0;
}


int create_ctx(int size, func_t f, struct parameters * args,char *name){


  irq_disable();
  struct ctx_s* new_ctx = (struct ctx_s*) calloc(1,sizeof(struct ctx_s));
  int corToInit = randRob++%CORE_NCORE;

  assert(new_ctx);

  if(init_ctx(new_ctx, size, f, args ,name)){ /* error */ return 1; }
  /* if this is the first ctx that is initialised, we initialise the ring_head */
  if(!mega_ctx[corToInit].ring_head){
    mega_ctx[corToInit].ring_head = new_ctx;
    mega_ctx[corToInit].ring_head->ctx_next = new_ctx;
  }
  else {
    new_ctx->ctx_next = mega_ctx[corToInit].ring_head->ctx_next;
    mega_ctx[corToInit].ring_head->ctx_next = new_ctx;
  }
  irq_enable();

  return 0;
}






void del_ctx(struct ctx_s *ctx){
  irq_disable();
  int currentCor = _in(CORE_ID);
  assert(ctx != NULL);
  if(DEBUG)
    printf(RED"Deleting : "RESET);

  if(DEBUG)
    print_ctx(ctx);

  if(DEBUG)
    printf(RED"ADDRESS : %p \n"RESET,(void *)ctx);

  /* si il n'y a qu'un seul context, on libere l'espace memoire de la stack et le pointeur de la structure */
  /* if(ctx == ctx->ctx_next){ */
  /*   free(ctx->ctx_stack); */
  /*   free(ctx); */
  /* } else { */
  /*   struct ctx_s *suivant = ctx->ctx_next; */
  /*   struct ctx_s *precedent = ctx; */
  /*   while(precedent->ctx_next != ctx ) */
  /*     precedent = precedent->ctx_next; */
  /*   precedent->ctx_next = suivant; */
  /*   ctx = NULL; */
  /* } */

}

void start(){
  printf("Entering in start for schedule");
  yield();
}



void switch_to_ctx(struct ctx_s *new_ctx){
  int currentCor = _in(CORE_ID);
  assert(new_ctx != NULL);
  assert(new_ctx->ctx_magic == CTX_MAGIC);

  
  if(!mega_ctx[currentCor].ring_head & !mega_ctx[currentCor].current_ctx){
    mega_ctx[currentCor].return_ctx = (struct ctx_s*)malloc(sizeof(struct ctx_s));
    mega_ctx[currentCor].return_ctx->ctx_magic = CTX_MAGIC;
    __asm__ ("movl %%esp, %0\n" :"=r"(mega_ctx[currentCor].return_ctx->ctx_rsp));
    __asm__ ("movl %%ebp, %0\n" :"=r"(mega_ctx[currentCor].return_ctx->ctx_rbp));
  } else {
    __asm__ ("movl %%esp, %0\n" :"=r"(mega_ctx[currentCor].current_ctx->ctx_rsp));
    __asm__ ("movl %%ebp, %0\n" :"=r"(mega_ctx[currentCor].current_ctx->ctx_rbp));
  }

  mega_ctx[currentCor].current_ctx = new_ctx;

  __asm__ ("movl %0, %%esp\n" ::"r"(mega_ctx[currentCor].current_ctx->ctx_rsp));
  __asm__ ("movl %0, %%ebp\n" ::"r"(mega_ctx[currentCor].current_ctx->ctx_rbp));
  
  /* if the current context has not been started, we statrt it */
  if(mega_ctx[currentCor].current_ctx->ctx_state == CTX_RDY){
    mega_ctx[currentCor].current_ctx->ctx_state = CTX_EXQ;
    irq_enable();
    (*mega_ctx[currentCor].current_ctx->ctx_f)(mega_ctx[currentCor].current_ctx->ctx_arg);
    irq_disable();
    mega_ctx[currentCor].current_ctx->ctx_state = CTX_END;
    yield();
  }
  irq_enable();
}



void yield(){
  int currentCor = _in(CORE_ID);
  irq_disable();
  _out(TIMER_ALARM,TIMER);  /* alarm at next tick (at 0xFFFFFFFF) */
  if(DEBUG){
    printf("\n !!!!!!!! CPT : %d!!!!!!!\n",cpt++);
    printf(GREEN"\n======================\n"RESET);
    printf(GREEN"\nENTERING YIELD\n"RESET);
    printf(GREEN"\n======================\n"RESET);
    if(mega_ctx[currentCor].current_ctx)
      print_ctx(mega_ctx[currentCor].current_ctx);
    else
      print_ctx(mega_ctx[currentCor].ring_head);
    printf(GREEN"\n======================\n"RESET);
    print_pile_ctx();
  }

  /* if the current context of the thread has no jobs to do */
  if(!mega_ctx[currentCor].current_ctx){
    mega_ctx[currentCor].current_ctx = mega_ctx[currentCor].ring_head;
    mega_ctx[currentCor].current_ctx->ctx_next = mega_ctx[currentCor].ring_head;
    switch_to_ctx(mega_ctx[currentCor].current_ctx);
  } else {
    /* we set the new context we are going to look at */
    struct ctx_s * ctx = mega_ctx[currentCor].current_ctx->ctx_next;

    /* we are going to try to find a context we can deal with */
    while(1){
      if(ctx->ctx_state == CTX_RDY)
        break;
      if(ctx->ctx_state == CTX_EXQ)
        break;
      if(ctx->ctx_state == CTX_DISQUE || ctx->ctx_state == CTX_STP){
        ctx = ctx->ctx_next;
        continue;
      }
      /* if the context we are looking at was finished, we have to kill it */
      if(ctx->ctx_state == CTX_END){
        /* ctx = ctx->ctx_next;	 */
	printf("Deleting ctx \n");
	mega_ctx[currentCor].current_ctx->ctx_next = ctx->ctx_next;
	free(ctx->ctx_stack);
	free(ctx);
	/* si on est en tete de liste */
	if(mega_ctx[currentCor].ring_head == ctx) 
	  mega_ctx[currentCor].ring_head = mega_ctx[currentCor].current_ctx;
	ctx = mega_ctx[currentCor].current_ctx->ctx_next;
	break;
      }
    }

    switch_to_ctx(ctx);
  }
}



void reset_ctx_disque(){
  int currentCor = _in(CORE_ID);

  irq_disable();
  if(DEBUG){
    printf(BOLDMAGENTA"\nENTERING reset_ctx_disque()\n"RESET);  
    printf("\tReset_ctx_disque : Le ctx qui a reçu l'IRQ :  ");
    print_ctx(mega_ctx[currentCor].ctx_disque);
  }
  assert(mega_ctx[currentCor].ctx_disque != NULL);
  mega_ctx[currentCor].ctx_disque->ctx_next = mega_ctx[currentCor].current_ctx;
  mega_ctx[currentCor].current_ctx = mega_ctx[currentCor].ctx_disque;
  mega_ctx[currentCor].current_ctx->ctx_state = CTX_EXQ;
  irq_enable();
}


void wait_disque(){
  int currentCor = _in(CORE_ID);

  irq_disable();
  if(DEBUG)
    printf(BOLDYELLOW"\nENTERING wait_disque()\n"RESET);
  if(DEBUG)
    printf("\nWait_disque(): le ctx demandeur :  ");
  if(DEBUG)
    print_ctx(mega_ctx[currentCor].current_ctx);
  mega_ctx[currentCor].ctx_disque = mega_ctx[currentCor].current_ctx;
  mega_ctx[currentCor].ctx_disque->ctx_state = CTX_DISQUE;
  yield();
}



/* void sem_init(struct sem_s *sem, unsigned int val, char* name){ */
/*   printf(BOLDGREEN"[init semaphore disque]"RESET GREEN"\n"RESET); */
/*   assert(sem); */
/*   sem->init_cpt = sem->sem_cpt = val; */
/*   sem->sem_head = NULL; */
/*   sem->sem_name = name; */
/*   assert(sem); */
/* } */

/* void sem_up(struct sem_s *sem){ */

/*   irq_disable(); */
/*   if(DEBUG){ */
/*     printf(BOLDMAGENTA"\n[Tentative de rendu de semaphre] %s avec comme etat : %d\n"RESET,ctx_disque->ctx_name,sem->sem_cpt); */
/*   } */
/*   sem->sem_cpt++; */
/*   if(sem->sem_cpt > 0){ */
/*     struct ctx_s *ctx_tmp = sem -> sem_head->ctx_next; */
/*     sem->sem_head->ctx_state = CTX_EXQ; */
/*     sem->sem_head->ctx_next = ring_head; */
/*     current_ctx = sem->sem_head; */
/*     sem->sem_head = ctx_tmp; */
/*     yield(); */
/*   } */
/*   if(DEBUG){ */
/*     printf(BOLDMAGENTA"\n[SEMAPHORE DISQUE RENDU PAR] %s avec comme nouvel etat : %d\n"RESET,ctx_disque->ctx_name,sem->sem_cpt); */
/*   } */

/*   irq_enable(); */


/* } */


/* void sem_down(struct sem_s *sem){ */
/*   irq_disable(); */
/*   if(DEBUG) */
/*     printf(BOLDMAGENTA"\n[Tentative de prise de semaphre] %s avec comme etat : %d\n"RESET,current_ctx->ctx_name,sem->sem_cpt); */

/*   if(sem->sem_cpt == 0){ */
/*     printf("FREEZE"); */
/*     current_ctx->ctx_state = CTX_STP; */
/*     if(!sem->sem_head){ */
/*       current_ctx->ctx_sem_next = sem->sem_last; */
/*       sem->sem_head = current_ctx; */
/*     }else{ */
/*       current_ctx->ctx_sem_next = sem->sem_head; */
/*       sem->sem_last->ctx_next = current_ctx; */
/*       sem->sem_last = current_ctx; */
/*     } */
/*     yield(); */
/*   } */
/*   sem->sem_cpt--; */
/*   if(DEBUG) */
/*     printf(BOLDMAGENTA"\n[SEMAPHORE DISQUE PRIS PAR] %s avec comme nouvel etat : %d\n"RESET,current_ctx->ctx_name,sem->sem_cpt); */

/*   irq_enable(); */
/*   yield(); */
/* } */

