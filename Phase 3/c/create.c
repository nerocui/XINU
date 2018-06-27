/* create.c : create a process
 */

#include <xeroskernel.h>
#include <xeroslib.h>

pcb     proctab[MAX_PROC];

/* make sure interrupts are armed later on in the kernel development  */




// Another bit of a hack. Th PID value of 0 is reserved for the 
// NULL/idle process. The underlying assumption is that the 
// idle process will be the first process created. IF that isn't
// the case the system will break. 

static int      nextpid = 1;



int create( funcptr fp, size_t stackSize ) {
/***********************************************/

    context_frame       *cf;
    pcb                 *p = NULL;
    int                 i;


    /* PID has wrapped and we can't have -ve numbers 
     * this means that the next PID we handout can't be
     * in use. To find such a number we have to propose a 
     * new PID and then scan to see if it is in the table. If it 
     * is then we have to try again. We know we will succeed 
     * because the process table size is smaller than PID space.
     * However, this code does not do that and just returns an 
     * error.
     */


    if (nextpid < 0) 
      return CREATE_FAILURE;

    // If the stack is too small make it larger
    if( stackSize < PROC_STACK ) {
        stackSize = PROC_STACK;
    }

    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].state == STATE_STOPPED ) {
            p = &proctab[i];
            break;
        }
    }

    //    Some stuff to help wih debugging
    //    char buf[100];
    //    sprintf(buf, "Slot %d empty\n", i);
    //    kprintf(buf);
    //    kprintf("Slot %d empty\n", i);
    
    if( !p ) {
        return CREATE_FAILURE;
    }

    cf = kmalloc( stackSize );
    if( !cf ) {
        return CREATE_FAILURE;
    }

    // initialize the handler table and let hndlr pointers to the address 
    // of the first handler in the table

    p->hndlr = &(p->handler_table[0]);
    for( i = 0; i < MAX_SIGNAL; i++){

        // we let the first handler to be handler for signal num 31 
        // because it is convinient for us to check the priority later 
        p->handler_table[i].sig_no = 31 - i;
        p->handler_table[i].signalled = 0;
        p->handler_table[i].handler_func = NULL;
        if (i < 31){
            p->handler_table[i].next = &(p->handler_table[i + 1]);
        }
        else {
            p->handler_table[i].next = NULL;
        }
    }

    // initialize the file descriptor table

    for( i = 0; i < 4; i++ ){
        p->fd_table[i].major_num = 0;
        p->fd_table[i].minor_num = 0;
        p->fd_table[i].status = 0;
        p->fd_table[i].ptr = NULL;
    }

    // The -4 gets us one extra stack spot for the return address
    cf = (context_frame *)((unsigned char *)cf + stackSize - 4);
    cf--;

    memset(cf, 0xA5, sizeof( context_frame ));

    cf->iret_cs = getCS();
    cf->iret_eip = (unsigned int)fp;
    cf->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;

    cf->esp = (int)(cf + 1);
    cf->ebp = cf->esp;
    cf->stackSlots[0] = (int) sysstop;
    p->esp = (unsigned long*)cf;
    p->state = STATE_READY;
    p->pid = nextpid++;
    p->cpuTime = 0;

    // this is used to store the process which this process is waiting
    p->waiting = NULL;
    
    ready( p );
    return p->pid;
}

int create_idle( funcptr fp, size_t stackSize ) {
/***********************************************/

    context_frame       *cf;
    pcb                 *p = NULL;
    int                 i;


    /* PID has wrapped and we can't have -ve numbers 
     * this means that the next PID we handout can't be
     * in use. To find such a number we have to propose a 
     * new PID and then scan to see if it is in the table. If it 
     * is then we have to try again. We know we will succeed 
     * because the process table size is smaller than PID space.
     * However, this code does not do that and just returns an 
     * error.
     */


    if (nextpid < 0) 
      return CREATE_FAILURE;

    // If the stack is too small make it larger
    if( stackSize < PROC_STACK ) {
        stackSize = PROC_STACK;
    }

    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].state == STATE_STOPPED ) {
            p = &proctab[i];
            break;
        }
    }

    //    Some stuff to help wih debugging
    //    char buf[100];
    //    sprintf(buf, "Slot %d empty\n", i);
    //    kprintf(buf);
    //    kprintf("Slot %d empty\n", i);
    
    if( !p ) {
        return CREATE_FAILURE;
    }

    cf = kmalloc( stackSize );
    if( !cf ) {
        return CREATE_FAILURE;
    }

    p->hndlr = &(p->handler_table[0]);
    for( i = 0; i < MAX_SIGNAL; i++){
        p->handler_table[i].sig_no = 31 - i;
        p->handler_table[i].signalled = 0;
        p->handler_table[i].handler_func = NULL;
        if (i < 31){
            p->handler_table[i].next = &(p->handler_table[i + 1]);
        }
        else {
            p->handler_table[i].next = NULL;
        }
    }

    for( i = 0; i < 4; i++ ){
        p->fd_table[i].major_num = 0;
        p->fd_table[i].minor_num = 0;
        p->fd_table[i].status = 0;
        p->fd_table[i].ptr = NULL;
    }

    // The -4 gets us one extra stack spot for the return address
    cf = (context_frame *)((unsigned char *)cf + stackSize - 4);
    cf--;

    memset(cf, 0xA5, sizeof( context_frame ));

    cf->iret_cs = getCS();
    cf->iret_eip = (unsigned int)fp;
    cf->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;

    cf->esp = (int)(cf + 1);
    cf->ebp = cf->esp;
    cf->stackSlots[0] = (int) sysstop;
    p->esp = (unsigned long*)cf;
    p->state = STATE_READY;
    p->pid = 0;
    p->cpuTime = 0;

    p->waiting = NULL;
    
    ready( p );
    return p->pid;
}