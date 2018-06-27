/* create.c : create a process
 */

#include <xeroskernel.h>

/* Your code goes here. */
extern struct pcb main_block[32];
extern struct pcb idle;
extern struct pcb *idle_ptr;
extern struct pcb *free_queue;
extern struct pcb *ready_queue;
extern struct pcb *ready_tail;
extern struct pcb *free_tail;
extern int pid = 1;

extern void wrapper(void (*func)(void));

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
	new_proc->result = 1;

	struct real_func *real = (struct real_func *)(new_proc->esp - 8);
	real->return_addr = 0;
	real->func_r = func;
	//kprintf("--func in create is:%ld",real->func_r);
	//kprintf("--esp in create is: %ld", new_proc->esp);
	//kprintf("--func in create is:%ld",func);
	new_proc->esp = (unsigned long *) real;
	//kprintf("--esp after real in create is:%ld",new_proc->esp);
	
	struct context_frame *new_cf = (struct context_frame *)(new_proc->esp - 44);/*pointing the cf to stack pointer - the size of cf*/
	new_cf->edi = 0;/*initializing everything*/
	new_cf->esi = 0;
	new_cf->ebp = 0;
	new_cf->esp = 0;
	new_cf->ebx = 0;
	new_cf->edx = 0;
	new_cf->ecx = 0;
	new_cf->eax = 0;
	new_cf->iret_eip = (unsigned long) *wrapper;
	new_cf->iret_cs = getCS();
	new_cf->eflags = 0x00003200;
	// modify it in pre-emption
	new_proc->esp = (unsigned long *) new_cf;
	
	//kprintf("--esp after cf in create is:%ld",new_proc->esp);
	new_proc->PID = pid;
	pid++;


	if(ready_queue->PID == 999){
		new_proc->next = NULL;
		ready_queue = new_proc;
		ready_tail = new_proc;
	}
	else if(ready_tail == NULL && ready_queue == NULL){
		//kprintf("--the ready queue is null while creating--");
		ready_queue = new_proc;
		ready_tail = ready_queue;
		ready_tail->next = NULL;
		/*if(ready_queue == NULL)
			kprintf("--the ready queue is still null even after putting stuff in it--");
			*/
	}
	else{
		//kprintf("--the ready queue is not null while creating--");
		ready_tail->next = new_proc;
		ready_tail = ready_tail->next;
		ready_tail->next = NULL;
		/*if(ready_queue == NULL)
			kprintf("--the ready queue somehow became null after inserting in an already populated queue--");
	*/
	}
	return new_proc->PID;
}


extern int create_idle(void (*func) (void),int stack){
	
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
	new_proc->result = 1;
	
	struct real_func *real = (struct real_func *)(new_proc->esp - 8);
	real->return_addr = 0;
	real->func_r = func;
	//kprintf("--func in create is:%ld",&func);
	new_proc->esp = (unsigned long *) real;

	struct context_frame *new_cf = (struct context_frame *)(new_proc->esp - 44);/*pointing the cf to stack pointer - the size of cf*/
	new_cf->edi = 0;/*initializing everything*/
	new_cf->esi = 0;
	new_cf->ebp = 0;
	new_cf->esp = 0;
	new_cf->ebx = 0;
	new_cf->edx = 0;
	new_cf->ecx = 0;
	new_cf->eax = 0;
	new_cf->iret_eip = (unsigned long) *wrapper;
	new_cf->iret_cs = getCS();
	new_cf->eflags = 0x00003200;
	// modify it in pre-emption
	new_proc->esp = (unsigned long *) new_cf;
	new_proc->PID = 999;
	
	idle_ptr = new_proc;
	return new_proc->PID;
}