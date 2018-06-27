/* ctsw.c : context switcher
 */

#include <xeroskernel.h>

/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
*/

static void *k_stack;
static unsigned long ESP;/*global variables for these four registers to use in ctsw*/
static unsigned long EAX;
static unsigned long EDX;
static unsigned long EDI;
static int rc, interrupt;
//static unsigned long ECX;
void _ISREntryPoint();
void _InterruptEntryPoint();
extern void contextinit(){
	set_evec(128, (unsigned long)&_ISREntryPoint);
    set_evec(32, (unsigned long)&_InterruptEntryPoint);
	// Random num between 48 and 255, so picked 128
    initPIT(100);
    //a process will be given a time slice of 10 milliseconds, before being pre-empted.
}

extern int contextswitch(void *p) {
    struct pcb *proc = p;
    ESP = proc->esp;/*store the stack pointer of the process in the global variable ESP*/
    EDI = proc->PID;
    rc = proc->result;
    __asm __volatile( " \
        pushf  \n\
        pusha  \n\
        movl  %%esp, k_stack   \n\
        movl  ESP, %%esp  \n\
        popa  \n\
        movl  rc, %%eax  \n\
        movl  EDI, %%edi    \n\
        iret  \n\
        _InterruptEntryPoint:   \n\
            cli     \n\
            pusha   \n\
            movl    $1, %%ecx   \n\
            jmp     _CommonJump     \n\
        _ISREntryPoint:  \n\
            cli     \n\
            pusha   \n\
            movl  $0, %%ecx    \n\
        _CommonJump:     \n\
            movl %%eax, rc \n\
            movl %%edx, EDX \n\
            movl  %%esp, ESP  \n\
            movl  k_stack, %%esp \n\
            movl  %%eax, 28(%%esp)  \n\
            movl  %%ecx, 24(%%esp)  \n\
            popa \n\
            popf \n\
            movl  %%eax, rc    \n\
            movl  %%ecx, interrupt  \n\
                "
        :
        :
        : "%eax" , "%ecx"
        );/*after entrypoint entered from syscall, we store eas which is cann number in EAX, stack size in ECX and func in EDX, and then we put them in the pcb to have the process have its info back */
    
    if(interrupt){
        proc->result = rc;
        rc = TIMER_INT;
    }
    proc->esp = ESP;
    //proc->stack = (int)ECX;
    //proc->cf->iret_eip = (void*)EDX;
    proc->args = EDX;
    return rc;
    /*fix it*/

   // Reach eax from esp
}