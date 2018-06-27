/* ctsw.c : context switcher
 */

#include <xeroskernel.h>
#include <i386.h>


void _KernelEntryPoint(void);
void _TimerEntryPoint(void);
void _KeyboardEntryPoint(void);
extern void sigtramp(funcptr handler_func, void *cntx);

static void               *saveESP;
static unsigned int        rc;
static int                 trapNo;
static long                args;

int contextswitch( pcb *p ) {
/**********************************/

    /* keep every thing on the stack to simplfiy gcc's gesticulations
     */
     if(has_signal(p)){
         deal_with_sig(p);
     }
    saveESP = p->esp;
    rc = p->ret; 
 
    /* In the assembly code, switching to process
     * 1.  Push eflags and general registers on the stack
     * 2.  Load process's return value into eax
     * 3.  load processes ESP into edx, and save kernel's ESP in saveESP
     * 4.  Set up the process stack pointer
     * 5.  store the return value on the stack where the processes general
     *     registers, including eax has been stored.  We place the return
     *     value right in eax so when the stack is popped, eax will contain
     *     the return value
     * 6.  pop general registers from the stack
     * 7.  Do an iret to switch to process
     *
     * Switching to kernel
     * 1.  Push regs on stack, set ecx to 1 if timer interrupt, jump to common
     *     point.
     * 2.  Store request code in ebx
     * 3.  exchange the process esp and kernel esp using saveESP and eax
     *     saveESP will contain the process's esp
     * 4a. Store the request code on stack where kernel's eax is stored
     * 4b. Store the timer interrupt flag on stack where kernel's eax is stored
     * 4c. Store the the arguments on stack where kernel's edx is stored
     * 5.  Pop kernel's general registers and eflags
     * 6.  store the request code, trap flag and args into variables
     * 7.  return to system servicing code
     */
     
    __asm __volatile( " \
        pushf \n\
        pusha \n\
        movl    rc, %%eax    \n\
        movl    saveESP, %%edx    \n\
        movl    %%esp, saveESP    \n\
        movl    %%edx, %%esp \n\
        movl    %%eax, 28(%%esp) \n\
        popa \n\
        iret \n\
   _TimerEntryPoint: \n\
        cli   \n\
        pusha \n\
        movl    $1, %%ecx \n\
        jmp     _CommonJumpPoint \n \
   _KeyboardEntryPoint: \n\
        cli   \n\
        pusha \n\
        movl    $-2, %%ecx \n\
        jmp     _CommonJumpPoint \n \
   _KernelEntryPoint: \n\
        cli \n\
        pusha  \n\
        movl   $0, %%ecx \n\
   _CommonJumpPoint: \n \
        movl    %%eax, %%ebx \n\
        movl    saveESP, %%eax  \n\
        movl    %%esp, saveESP  \n\
        movl    %%eax, %%esp  \n\
        movl    %%ebx, 28(%%esp) \n\
        movl    %%ecx, 24(%%esp)\n      \
        movl    %%edx, 20(%%esp) \n\
        popa \n\
        popf \n\
        movl    %%eax, rc \n\
        movl    %%ecx, trapNo \n\
        movl    %%edx, args \n\
        "
        : 
        : 
        : "%eax", "%ebx", "%edx"
    );

    /* save esp and read in the arguments
     */
    p->esp = saveESP;
    if( trapNo == 1) {
    /* return value (eax) must be restored, (treat it as return value) */
    p->ret = rc;
    rc = SYS_TIMER;
    } else if(trapNo == -2){/*keyboard interrupt*/
        //kprintf("in -2\n");
        p->ret = rc;
        return SYS_KBDINT;
    }
    else {
        p->args = args;
    }
    return rc;
}

void contextinit( void ) {
/*******************************/
  kprintf("Context init called\n");
  set_evec( KERNEL_INT, (int) _KernelEntryPoint );
  set_evec( TIMER_INT,  (int) _TimerEntryPoint );
  set_evec( KBD_INT,  (int) _KeyboardEntryPoint );
  initPIT( 100 );

}

int has_signal(pcb *p){
    handler *temp_handler = p->hndlr;
    while(temp_handler){
        if(temp_handler->signalled){
            return 1;
        }
        temp_handler = temp_handler->next;
    }
    //kprintf("Process %d is not signalled. \n", p->pid);
    return 0;
}

void deal_with_sig(pcb *p){
    handler *para_handler = p->hndlr;
    unsigned long    *cntx = p->esp;
    //kprintf("the old esp is: %ld \n", p->esp);
     while(para_handler){
        if(para_handler->signalled){
            /*found the handler and it is been signalled*/
            /*put trampoline with parameters on stack*/
            cntx--;
            *cntx = p->esp;
            //kprintf("The cntx is put on: %ld \n", cntx);
            cntx--;
            *cntx = para_handler->handler_func;
            //kprintf("The addr of handler func: %ld\n", para_handler->handler_func);
            //kprintf("The handler_func is put: %ld \n", cntx);
            cntx--;
            *cntx = 0;
            //kprintf("the fake ret addr is put: %ld \n", cntx);
            context_frame *cf = cntx - 11;
            //kprintf("The cf is put on: %ld \n", cf);
            
            memset(cf, 0xA5, sizeof( context_frame ));
            cf->iret_cs = getCS();
            //kprintf("The addr of sigtramp is: %ld \n", &sigtramp);
            cf->iret_eip = &sigtramp;
            para_handler->signalled = 0;
            cf->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;
            cf->stackSlots[0] = (int) sysstop;
            p->esp = (void*)cf;
            //kprintf("the new esp is: %ld \n", cf);
            
            /*unsignal it, if the process that signalled */
            /*this signal number to this this process is blocked, unblock it and ready it*/
            /*once done with the highest priority signal trampoling, return*/
            
            return;
        }
        para_handler = para_handler->next;
    }
}