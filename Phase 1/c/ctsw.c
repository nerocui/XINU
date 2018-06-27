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
static unsigned long ECX;
void _ISREntryPoint();

extern void contextinit(){
	set_evec(128, (unsigned long)&_ISREntryPoint);
	// Random num between 48 and 255, so picked 128
}
	
extern int contextswitch(void *p) {
    struct pcb *proc = p;
	ESP = proc->esp;/*store the stack pointer of the process in the global variable ESP*/
	__asm __volatile( " \
		pushf  \n\
		pusha  \n\
		movl  %%esp, k_stack   \n\
        movl  ESP, %%esp  \n\
        popa  \n\
        movl  EAX, %%eax  \n\
        iret  \n\
        _ISREntryPoint:  \n\
            movl %%eax, EAX \n\
            movl %%edx, EDX \n\
            movl %%ecx, ECX \n\
            pusha   \n\
            movl  %%esp, ESP  \n\
            movl  k_stack, %%esp \n\
            popa \n\
            popf \n\
                "
        :
        :
        : "%eax"
        );/*after entrypoint entered from syscall, we store eas which is cann number in EAX, stack size in ECX and func in EDX, and then we put them in the pcb to have the process have its info back */
    proc->esp = ESP;
    proc->stack = (int)ECX;
    proc->cf->iret_eip = (void*)EDX;
    return EAX;
    /*fix it*/

   // Reach eax from esp
}
