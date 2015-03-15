#include "sched.h"


/* rendu
Pour chaque points dans chaque scéances :
- ce que l'on a fait
- ce qui marche
- ce qui ne marche pas
- explicaiton de ce que notre exemple fait

 */





/* manages locks */
/* basically, each time we use mega_ctx, we make sure that no other core can change it */
void klock() {
  /* int i = 0; */
  /* /\* we check that the lock is free *\/ */

  /* if(_in(CORE_LOCK) == 1) { */
  /*   /\* we take the lock *\/ */
  /*   printf("CONTNIUE 1\n"); */
  /*   _out(CORE_LOCK, 0x2); */
  /*   printf("CONTNIUE2\n"); */

  /* } */
  /* /\* if it not free, we wait a bit and retry *\/ */
  /* else { */
  /*   while(i < 0xFFFF) { */
  /*     i++; */
  /*   } */
  /*   klock(); */
  /* } */
  while(_in(CORE_LOCK) != 1);
  _out(CORE_LOCK, 0x2);
  /* printf("klock \n"); */
  
}

void kunlock() {
  /* printf("kunlock\n"); */
  _out(CORE_UNLOCK, 0x1);
}




void print_ctx(struct ctx_s *ctx){
  printf("|> ");
  printf(BOLDWHITE"%s with state %d\n"RESET,ctx->ctx_name,ctx->ctx_state);
}

void print_pile_ctx(){
  int currentCor = _in(CORE_ID);
  unsigned int i;
  struct ctx_s *ctx;
  irq_disable();
  klock();
  assert(mega_ctx[currentCor].ring_head != NULL);
  ctx = mega_ctx[currentCor].ring_head;

  printf(BOLDGREEN"\n=============================\n"RESET);
  printf(BOLDGREEN"%d ctx\n"RESET,mega_ctx[currentCor].nb_ctx);
  printf("\n>><<\n");
  for(i=0;i<mega_ctx[currentCor].nb_ctx;i++){
    print_ctx(ctx);
    ctx = ctx->ctx_next;
  }
  kunlock();
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
    printf(BOLDGREEN"\nProcesseur n°%d,%d contexts: \n"RESET,i,mctx->nb_ctx);
    if(!mctx->nb_ctx){/*Si aucune tache sur ce processeurs on affiche ce message et passe au processeur suivant*/
      printf(BOLDWHITE"\tAucun context sur ce processeur\n"RESET);
      continue;
    }
    ctx = mctx->ring_head;
    /* Boucle parcourant les contexts de la ring_head */
    for(j=0;j<mctx->nb_ctx;j++){
      printf("\t%s\n",ctx->ctx_name);
      ctx= ctx->ctx_next;
    }
  }
}

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f,struct parameters * args,char *name){
  ctx->ctx_stack = (char*) calloc(stack_size,sizeof(char));
  if ( ctx->ctx_stack == NULL) {
    return 1;
  }
  int corToInit = randRob%CORE_NCORE;

  printf(BOLDGREEN"[init_ctx] corToInit = %d, %s\n"RESET, corToInit, name);

  ctx->ctx_name = name;
  ctx->ctx_state = CTX_RDY;
  ctx->ctx_size = stack_size;
  ctx->ctx_f = f;
  ctx->ctx_arg = args;

  /* QUESTION : comment peut-on initialiser le rsp et le rbp ici car nous n'avons pas encore sauvegardé les contexte ? */
  /* ctx->ctx_rsp = &(ctx->ctx_stack[stack_size-16]); */
  /* ctx->ctx_rbp = ctx->ctx_rsp; */
  /* fin de ma question */
  ctx->ctx_magic = CTX_MAGIC;
  ctx->ctx_next = ctx;
  mega_ctx[corToInit].nb_ctx++;

  if(DEBUG)
    printf(BOLDBLUE"\n%d ) creating ctx %s on \n"RESET,mega_ctx[corToInit].nb_ctx,name);

  return 0;
}


int create_ctx(int size, func_t f, struct parameters * args,char *name){
  irq_disable();
  klock();
  struct ctx_s* new_ctx = (struct ctx_s*) calloc(1,sizeof(struct ctx_s));
  int corToInit = ++randRob%CORE_NCORE;
  /* int corToInit = randRob; */

  assert(new_ctx);

  if(init_ctx(new_ctx, size, f, args ,name)){ 
    /* error */ 
    return 1; 
  }
  /* if this is the first ctx that is initialised, we initialise the ring_head */
  if(!mega_ctx[corToInit].ring_head){
    mega_ctx[corToInit].ring_head = new_ctx;
    mega_ctx[corToInit].ring_head->ctx_next = new_ctx;
  }
  else {
    new_ctx->ctx_next = mega_ctx[corToInit].ring_head->ctx_next;
    mega_ctx[corToInit].ring_head->ctx_next = new_ctx;
  }
  kunlock();
  irq_enable();
  return 0;
}


void start(){
  printf("Entering in start for schedule");
  yield();
}



void switch_to_ctx(struct ctx_s *new_ctx){
  int currentCor = _in(CORE_ID);

  assert(new_ctx != NULL);
  assert(new_ctx->ctx_magic == CTX_MAGIC);

  /* if we have not set a context, we initialise the return adress where we are supposed to go */
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
    kunlock();
    (*mega_ctx[currentCor].current_ctx->ctx_f)(mega_ctx[currentCor].current_ctx->ctx_arg);
    irq_disable();
    klock();
    mega_ctx[currentCor].current_ctx->ctx_state = CTX_END;
    irq_enable();
    kunlock();
    /* yield(); */
  }
  kunlock();
  irq_enable();
}



void yield(){

  int currentCor = _in(CORE_ID);
  irq_disable();

  _out(TIMER_ALARM,TIMER);  /* alarm at next tick (at 0xFFFFFFFF) */
  klock();
  /* we reinitialise the timer's interuption */

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

  /* we check that we initialised a context before */
  if(mega_ctx[currentCor].ring_head == NULL) {
    irq_enable();
    kunlock();
    return;
  }

  /* if the current context of the thread has no jobs to do */
  if(!mega_ctx[currentCor].current_ctx){
    mega_ctx[currentCor].current_ctx = mega_ctx[currentCor].ring_head;
    mega_ctx[currentCor].current_ctx->ctx_next = mega_ctx[currentCor].ring_head;
    irq_enable();
    kunlock();

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
      /* if there are no more contexts to deal with */
      if(mega_ctx[currentCor].nb_ctx == 0){
	irq_enable();
	kunlock();
      	return;
      }
      /* if the context we are looking at was finished, we have to kill it */
      if(ctx->ctx_state == CTX_END){
        ctx = ctx->ctx_next;
	mega_ctx[currentCor].current_ctx->ctx_next = ctx->ctx_next;
	free(ctx->ctx_stack);
	free(ctx);
	/* si il n'y a plus de contexte */
	
	/* si on est en tete de liste */
	if(mega_ctx[currentCor].ring_head == ctx) 
	  mega_ctx[currentCor].ring_head = mega_ctx[currentCor].current_ctx;
	ctx = mega_ctx[currentCor].current_ctx->ctx_next;
	mega_ctx[currentCor].nb_ctx--;
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


