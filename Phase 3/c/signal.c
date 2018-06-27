/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */

extern pcb *sleepQ;

extern void sigtramp(funcptr handler_func, void *cntx){
  unsigned long *old_esp = cntx;
  //kprintf("In Trampline, ESP was: %ld \n", cntx);
  handler_func();
  syssigreturn(old_esp);
}

extern int signal(int pid, int sig_no){
    pcb *p = findPCB(pid);
    int result = 0;
    //kprintf("In signal, p->pid is %d, state is %d\n", p->pid, p->state);
    if(p->state == STATE_BLOCK){
      result = -2;
      // remove this process from the queue of the process which this process is waiting 
      if(p->waiting){
        removeFromBlock(p, p->waiting);
      }
      ready(p);
      p->waiting = NULL;
      p->ret = -362;
    }else if(p->state == STATE_SLEEP){
      int remain_time = 0;
      pcb *temp = sleepQ;
      while(temp != p){
        remain_time += temp->sleepdiff;
        temp = temp->next;
      }
      // return the remaining sleep time
      p->ret = remain_time;
      removeFromSleep(p);
      ready(p); 
    }
    /*else if(p->state == STATE_READY){
      removeFromReady(p);
    }*//*do not need to remove from ready queue because we need it to be ready to run*/
    handler *temp_handler = findHandler(p, sig_no);
    temp_handler->signalled = 1;/*setting the signalled bit without giving blockee means to run it now*/
  return result;
}
