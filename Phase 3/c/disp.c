/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <i386.h>
#include <xeroslib.h>
#include <stdarg.h>
#include <kbd.h>

static pcb      *head = NULL;
static pcb      *tail = NULL;
/*
extern pcb      *kbd_block_head;
extern pcb      *kbd_block_tail;
extern char      kbd_buffer[4];
extern int       state;
extern int       device_num;
*/

void     dispatch( void ) {
/********************************/

    pcb         *p;
    int         r;
    funcptr     fp;
    int         stack, fd, bufflen;
    unsigned int port_60, port_64;
    unsigned int        value;
    va_list     ap;
    char        *str;
    int         len;
    void        *buff;
    for( p = next(); p ;) {
      //      kprintf("Process %x selected stck %x\n", p, p->esp);

      r = contextswitch( p );

      switch( r ) {

      case( SYS_CREATE ):
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg( ap, int ) );
        stack = va_arg( ap, int );
        p->ret = create( fp, stack );
        break;
      
      case( SYS_YIELD ):
        ready( p );
        p = next();
        break;
      
      case( SYS_STOP ):
        p->state = STATE_STOPPED;
        pcb *blockee = findBlockee(p->pid);
        if(blockee){
          //kprintf("Found blockee %d, it was waiting for process %d\n", blockee->pid, blockee->waiting->pid);
            removeFromBlock(blockee, p);
            blockee->waiting = NULL;
            ready(blockee);
        }
        else{
          //kprintf("didn't find blockee\n");
        }
        p = next();
        break;
      
      case ( SYS_KILL ):
        //kprintf("In syskill\n");
        // need to be updated
        ap = (va_list)p->args;
        //p->ret = kill(p, va_arg( ap, int ) );
        int pid = va_arg( ap, int );
        int signal_num = va_arg( ap, int );
        int direct_kill = va_arg(ap, int);
        if(direct_kill == -1){
          p->ret = kill(pid);
          //kprintf("In KILL\n");
          break;
        }
        pcb *target_proc = findPCB(pid);
        if(!target_proc || (target_proc->state == STATE_STOPPED || target_proc->pid == 0)){    // if target process does not exist
          
        kprintf("In target non exist\n");
          p->ret = -712;
          break;
        }

        if(signal_num > 31 || signal_num < 0){  // if signal num is not valid

          kprintf("In sinal invalid\n");
          p->ret = -651;
          break;
        }
        
        p->ret = signal(pid, signal_num);// if success return 0, if sleep, return remain sleep time
        break;

      case (SYS_CPUTIMES):
        ap = (va_list) p->args;
        p->ret = getCPUtimes(p, va_arg(ap, processStatuses *));
        break;
      
      case( SYS_PUTS ):
        ap = (va_list)p->args;
        str = va_arg( ap, char * );
        kprintf( "%s", str );
        p->ret = 0;
        break;
      
      case( SYS_GETPID ):
        p->ret = p->pid;
        break;
      
      case( SYS_SLEEP ):
        ap = (va_list)p->args;
        len = va_arg( ap, int );
        sleep( p, len );
        p = next();
        break;
      
      case( SYS_TIMER ):
        tick();
        //kprintf("T");
        p->cpuTime++;
        ready( p );
        p = next();
        end_of_intr();
        break;

      case( SYS_SIGHANDLER ):

        ap = (va_list)p->args;
        int signal_no = va_arg( ap, int );
        if (signal_no < 0 || signal_no > 31){   // test if signal is valid
          p->ret = -1;
          break;
        } 
        funcptr *newhandler_func = va_arg(ap, funcptr*);
        funcptr **oldhandler_func = va_arg(ap, funcptr**);

        handler *handler_found = findHandler(p, signal_no);
        if (handler_found->handler_func){     // check if the handler already exists
          oldhandler_func = handler_found->handler_func;    // yes then put it into oldhandler
        }
        else {
          oldhandler_func = NULL;       // no then set oldhandler to NULL
        }
        handler_found->handler_func = newhandler_func;    // update the handler to the newhandler
        p->ret = 0;
        break;

      case( SYS_SIGRET ):

        ap = (va_list)p->args;
        void *old_esp = va_arg(ap, void*);
        p->esp = old_esp;
        break;

      case( SYS_WAIT ): 
        //kprintf("in syswait\n");
        ap = (va_list)p->args;
        int blocker_pid = va_arg(ap, int);
        pcb *blocker = findPCB(blocker_pid);
        if(!blocker){
            p->ret = -1;
            kprintf("There is no blocker, you cannot wait\n");
        }
        else{
            //kprintf("Process %d is being blocked for process %d\n", p->pid, blocker->pid);
            block(p, blocker);
            p->ret = 0;
            p = next();
        }
        break;

      case( SYS_OPEN ): 
        ap = (va_list)p->args;
        int device_no = va_arg(ap, int);
        p->ret = di_open(p, device_no);
        break;

      case( SYS_CLOSE ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        p->ret = di_close(p, fd);
        break;

      case( SYS_WRITE ): 
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void*);
        bufflen = va_arg(ap, int);
        p->ret = di_write(p, fd, buff, bufflen);
        break;

      case( SYS_READ ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void*);
        bufflen = va_arg(ap, int);
        p->ret = di_read(p, fd, buff, bufflen);
        if(p->state == STATE_KBD_BLOCK)
          p = next();
        break;

      case( SYS_IOCTL ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        int num_of_arg = va_arg(ap, int);
        //kprintf("In disp, num of arg is: %d\n", num_of_arg);
        unsigned long command = va_arg(ap, unsigned long);
        if(num_of_arg == 2){
          int new_eof = va_arg(ap, int);
          p->ret = di_ioctl(p, fd, num_of_arg, command, new_eof);
        }
        // check whether we need to set the new EOF or not
       
        else if(num_of_arg == 1){

          p->ret = di_ioctl(p, fd, num_of_arg, command, 0);
        }
        break;

      case(SYS_KBDINT):
        //kprintf("in kbdint\n");

        // go to port 64
        port_64 = inb(0x64);
        port_64 &= 1;/*check if there is data*/
        pcb *kbd_p = kbd_block_head;
        if(port_64){
          value = inb(0x60);/*read data*/

          //value = 0x1e;
        }
        else {
          kprintf("port_64 valid\n");
          break;
        }/*there is no data*/
        value  = kbtoa(value);
        char ascii_value = (char)value;
        if(value != 256 ){/*check is it is nochar*/
          //kprintf("NotNOCHAR\n");
          if(value == eof){
            //kprintf("EOF\n");
            if(kbd_block_head){
              //kprintf("Gonna go eof_break\n");
              goto eof_break;
            }
          }
          if(state1)/*if echo, print*/{
            kprintf("%c", ascii_value);
          }
        }
        else {
          //kprintf("NOCHAR\n");
          goto kbd_break;
        }
        
        if(!kbd_buffer[0] && kbd_block_head){/*buffer is empty and there is process blocked*/
          /*ready p on block queue, and write to buffer for the p*/
          //kprintf("in this\n");
          //print_pcb(kbd_block_head);
          kbd_p = kbd_block_head;
          //kprintf("In disp, p->buffer %s\n", kbd_p->buffer - 1);
          //kprintf("In disp, p->bufferlen %d\n", kbd_p->bufferlen);
          if(kbd_p->bufferlen != 0){
            //kprintf("kbd_p->bufferlen is not 0\n");
            char *pcb_buff = kbd_p->buffer;
            *pcb_buff = ascii_value;
            //kprintf("4545445+++++++\n");
            kbd_p->buffer++;
            //kprintf("4545445=======\n");
            kbd_p->bufferlen--;
            if(!kbd_p->bufferlen ){
                eof_break:
                //print_pcb(kbd_block_head);
                kbd_p->bufferlen = 0;
                kbd_p->buffer = NULL;
                //kprintf("In eof, p->buffer %s\n", kbd_p->buffer);
                //kprintf("In eof, p->bufferlen %d\n", kbd_p->bufferlen);
                //kprintf("Gonna unblock and ready, whatcha gonna do\n");
                kbd_unblock(kbd_p);
                ready(kbd_p);
            }
            //print_pcb(head);
            //print_pcb(kbd_block_head);
            end_of_intr();
            break;
          }
        }
        
        else if(kbd_buffer[3] == 0){/*buffer not empty, not full, write into it*/
          //kprintf("In store in keyboard case\n");
          char *temp = kbd_buffer[0];
          while(temp)
            temp++;
            *temp = ascii_value;
        }
        kbd_break:
        //kprintf("at kbd break\n");
        end_of_intr();
      break;
      default:
        kprintf( "Bad Sys request %d, pid = %d\n", r, p->pid );
      }
    }

    kprintf( "Out of processes: dying\n" );
    
    for( ;; );
}

extern void dispatchinit( void ) {
/********************************/

  //bzero( proctab, sizeof( pcb ) * MAX_PROC );
  memset(proctab, 0, sizeof( pcb ) * MAX_PROC);
}



extern void     ready( pcb *p ) {
/*******************************/
    p->next = NULL;
    p->state = STATE_READY;

    if( tail ) {
        tail->next = p;
        p->prev = tail;
        // Modified
    } else {
        head = p;
        p->prev = NULL;
        // Modified
    }

    tail = p;
}

extern void block(pcb *p, pcb *blocker){

    // the block for waiting
    p->next = NULL;
    p->state = STATE_BLOCK;
    if(blocker->block_tail){
      blocker->block_tail->next = p;
      p->prev = blocker->block_tail;
    }else{
      blocker->blockee = p;
      p->prev = NULL;
    }
    p->waiting = blocker;
    blocker->block_tail = p;
    kprintf("Blockee on blocker %d is %d, waiting on blockee is %d\n", blocker->pid, blocker->block_tail->pid, p->waiting->pid);
    return;
}

extern pcb *findBlockee(int pid){

    // find the process which is a blockee
    int i;
    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].waiting->pid == pid ) {
            return( &proctab[i] );
        }
    }

    return( NULL );
}



extern pcb      *next( void ) {
/*****************************/

    pcb *p;

    p = head;

    if( p ) {
        head = p->next;
        if( !head ) {
            tail = NULL;
        }
    } else { // Nothing on the ready Q and there should at least be the idle proc.
        kprintf( "BAD\n" );
        create_idle(idleproc, PROC_STACK);
    }

    p->next = NULL;
    p->prev = NULL;
    return( p );
}



extern pcb *findPCB( int pid ) {
/******************************/

    int i;

    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].pid == pid ) {
            return( &proctab[i] );
        }
    }

    return( NULL );
}


// This function takes a pointer to the pcbtab entry of the currently active process. 
// The functions purpose is to remove the process being pointed to from the ready Q
// A similar function exists for the management of the sleep Q. Things should be re-factored to 
// eliminate the duplication of code if possible. There are some challenges to that because
// the sleepQ is a delta list and something more than just removing an element in a list
// is being preformed. 


extern void removeFromReady(pcb * p) {

  if (!head) {
    kprintf("Ready queue corrupt, empty when it shouldn't be\n");
    return;
  }

  if (head == p) { // At front of list
    // kprintf("Pid %d is at front of list\n", p->pid);
    head = p->next;

    // If the implementation has idle on the ready list this next statement
    // isn't needed. However, it is left just in case someone decides to 
    // change things so that the idle process is kept separate. 

    if (head == NULL) { // If the implementation has idle process  on the 
      create_idle(idleproc, PROC_STACK);
    }
  } else {  // Not at front, find the process.
    pcb * prev = head;
    pcb * curr;
    
    for (curr = head->next; curr!=NULL; curr = curr->next) {
      if (curr == p) { // Found process so remove it
  // kprintf("Found %d in list, removing\n", curr->pid);
  prev->next = p->next;
  if (tail == p) { //last element in list
      tail = prev;
      // kprintf("Last element\n");
  }
  p->next = NULL; // just to clean things up
  break;
      }
      prev = curr;
    }
    if (curr == NULL) {
      kprintf("Kernel bug: Ready queue corrupt, process %d claimed on queue and not found\n",
        p->pid);
      
    }
  }
}

extern void removeFromBlock(pcb *p, pcb *blocker) {/*!!!!!!!!!!!TODO!!!!!!!!!!!*/
  //kprintf("In removeFromBlock\n");
  if (blocker->blockee == p) { // At front of list
    //kprintf("Pid %d is at front of list\n", p->pid);
    blocker->block_tail = p->next;

    // If the implementation has idle on the ready list this next statement
    // isn't needed. However, it is left just in case someone decides to 
    // change things so that the idle process is kept separate. 

    if (!blocker->blockee) { // If the implementation has idle process  on the 
      blocker->block_tail = blocker->blockee;      // ready list this should never happen
      //kprintf("Block queue empty.\n");
    }
  } else {  // Not at front, find the process.
    pcb * prev = blocker->blockee;
    pcb * curr;
    
    for (curr = blocker->blockee->next; curr!=NULL; curr = curr->next) {
      if (curr == p) { // Found process so remove it
        //kprintf("Found %d in list, removing\n", curr->pid);
        prev->next = p->next;
        if (blocker->block_tail == p) { //last element in list
            blocker->block_tail = prev;
            // kprintf("Last element\n");
        }
        p->next = NULL; // just to clean things up
        break;
        }
      prev = curr;
    }
    if (curr == NULL) {
      //kprintf("Process %d does not contain the given blockee.\n",
        p->pid;
      
    }
  }
}

// This function takes 2 paramenters:
//  currP  - a pointer into the pcbtab that identifies the currently running process
//  pid    - the proces ID of the process to be killed.
//
// Note: this function needs to be augmented so that it delivers a kill signal to a 
//       a particular process. The main functionality of the this routine will remain the 
//       same except that when the process is located it needs to be put onto the readyq
//       and a signal needs to be marked for delivery. 
//

int  kill(int pid) {
  pcb * targetPCB = findPCB(pid);
  if(pid > MAX_PROC || pid < 1 ){
    kprintf("No such process\n");
    return -1;
  }
  if( targetPCB->state == STATE_STOPPED || 
     (targetPCB->state != STATE_SLEEP && 
      targetPCB->state != STATE_BLOCK && 
      targetPCB->state != STATE_KBD_BLOCK && 
      targetPCB->state != STATE_READY && 
      targetPCB->state != STATE_RUNNING)){
    kprintf("No such process\n");
    return -1;
  }
  
  if (targetPCB->state == STATE_SLEEP) {
    // kprintf("Target pid %d sleeping\n", targetPCB->pid);
    removeFromSleep(targetPCB);
  }

  if (targetPCB->state == STATE_READY) {
    // remove from ready queue
    // kprintf("Target pid %d is ready\n", targetPCB->pid);
    removeFromReady(targetPCB);
  }

  if (targetPCB->state == STATE_BLOCK) {
    // add it because we have state block :)
    removeFromBlock(targetPCB, targetPCB->waiting);
  }

  // Check other states and do state specific cleanup before stopping
  // the process 
  // In the new version the process will not be marked as stopped but be 
  // put onto the readyq and a signal marked for delivery. 
  targetPCB->state == STATE_STOPPED;
  targetPCB->next = NULL;
  targetPCB->prev = NULL;
  kprintf("Just hard killed process %d \n", targetPCB->pid);
  return 0;
}
  

// This function is the system side of the sysgetcputimes call.
// It places into a the structure being pointed to information about
// each currently active process. 
//  p - a pointer into the pcbtab of the currently active process
//  ps  - a pointer to a processStatuses structure that is 
//        filled with information about all the processes currently in the system
//

extern char * maxaddr;
  
int getCPUtimes(pcb *p, processStatuses *ps) {
  
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) 
    return -1;

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(processStatuses)) > maxaddr)  
    return -2;

  // There are probably other address checks that can be done, but this is OK for now


  for (i=0; i < MAX_PROC; i++) {
    if (proctab[i].state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = proctab[i].pid;
      ps->status[currentSlot] = p->pid == proctab[i].pid ? STATE_RUNNING: proctab[i].state;
      ps->cpuTime[currentSlot] = proctab[i].cpuTime * MILLISECONDS_TICK;
    }
  }

  return currentSlot;
}


// extern int sig_is_masked(pcb *p, int signal){

//     /*check if the given signal number is masked*/

//     handler *temp = p->hndlr;
//     while(temp){
//         if(temp->sig_no == signal)
//             return 1;
//         else temp = temp->next;
//     }
//     return 0;
//     /*return 1 is masked, 0 otherwise*/
// }


extern handler *findHandler(pcb *p, int signal){

    /*given the signal is masked, return the corresponding handler*/
    
    handler *temp = p->hndlr;
    while(temp){
        if(temp->sig_no == signal){
            //kprintf("Found the handler in process, it is: %ld", temp->handler);
            return temp;
        }
        else temp = temp->next;
    }
    return NULL;
    /*return NULL if not found*/
}

extern void print_pcb(pcb *p){
  kprintf("PCB PRINT\n");
  kprintf("===============================================\n");
  while(p){
    char *state = "";
    switch(p->state){
      case(STATE_STOPPED):
        state = "STATE_STOPPED";
      break;
      case(STATE_READY):
        state = "STATE_READY";
      break;
      case(STATE_SLEEP):
        state = "STATE_RUNNING";
      break;
      case(STATE_BLOCK):
        state = "STATE_BLOCK";
      break;
      case(STATE_KBD_BLOCK):
        state = "STATE_KBD_BLOCK";
      break;
      default:
      break;
    }
    kprintf("PID: %d, State: %s\n", p->pid, state);
    p = p->next;
  }
}
