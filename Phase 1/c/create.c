/* create.c : create a process
 */

#include <xeroskernel.h>

/* Your code goes here. */
extern struct pcb main_block[32];
extern struct pcb *free_queue;
extern struct pcb *ready_queue;
extern struct pcb *ready_tail;
extern struct pcb *free_tail;


extern int create(void (*func) (void),int stack){
	
	if (free_queue == NULL){
		return 0;/*if the free queue is empty, we return 0 to indicate the failure of process creation*/
	}
	
	struct pcb* new_proc = free_queue;/*else we just point a pcb pointer to the free queue*/
	free_queue = free_queue->next;/*update queue*/
	
	new_proc->state = YIELD;/*setting its fields*/
	new_proc->next = NULL;
    if(stack <= 4096){/*if the stack size required is smaller than 4k, i give 4k b/c im a nice guy, also because it is safe to give that much as it need to use it for cf and arguments and so on*/
        stack = 4096;
    }
    new_proc->stack_aloc = kmalloc(stack);
    if(new_proc->stack_aloc == 0){
        kfree(new_proc->stack_aloc);
    }
	    
	new_proc->esp = new_proc->stack_aloc + stack/sizeof(unsigned long);/*setting stack pointer*/
	new_proc->cf->iret_eip = func;	
	new_proc->result = 1;
	new_proc->stack = stack;

	struct context_frame *new_cf = (struct context_frame *)(new_proc->esp - sizeof(struct context_frame)/sizeof(unsigned long) - 128/sizeof(unsigned long));/*pointing the cf to stack pointer - the size of cf*/
	new_cf->edi = 0;/*initializing everything*/
	new_cf->esi = 0;
	new_cf->ebp = 0;
	new_cf->esp = 0;
	new_cf->ebx = 0;
	new_cf->edx = 0;
	new_cf->ecx = 0;
	new_cf->eax = 0;
	new_cf->iret_eip = (unsigned long )func;
	new_cf->iret_cs = getCS();
	new_cf->eflags = 0;
	new_proc->esp = (unsigned long *) new_cf;
	
	if (ready_tail == NULL){/*if queue empty, put it in queue as head and tail*/
		ready_tail = new_proc;
		ready_queue = new_proc;
	}
	else {/*put it to tail and update tail*/
		ready_tail->next = new_proc;
		ready_tail = ready_tail->next;
	}
	return 1;
}