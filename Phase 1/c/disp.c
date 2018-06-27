/* disp.c : dispatcher
 */

#include <xeroskernel.h>

/* Your code goes here */

extern struct pcb *ready_queue;
extern struct pcb *block_queue;
extern struct pcb *free_queue;
extern struct pcb *ready_tail;
extern struct pcb *free_tail;
struct pcb* process;

extern void dispatch(void){
	int temp;/*this temp is for storing the create function's result*/
	process = next();
	kprintf("finish next");
	for( ;; ) {
		int request = contextswitch( process );
		switch(request) {
			case( CREATE ):/*if the syscall is CREATE, we call create*/
			temp = create(process->cf->iret_eip, process->stack); 
			process->cf->eax = temp;/*and we now can store the temp in pcb's eax.*/
			break; 
			case( YIELD ): ready( process ); /*in the case of YIELD, we call ready and then switch to the next pcb*/
			process = next(); 
			break; 
			case( STOP ): cleanup( process ); /*upon STOP, we kill the pcb with cleanup*/
			process = next();
			break;
		}
		
	}
}

extern void* next(void){
	if (ready_queue->next == NULL){
		return ready_queue;/*if the ready queue has only one pcb, we return that one, but keep it in the queue bc there should always be at least one pcb in it*/
	}
	
	struct pcb *temp = ready_queue;
	temp->state = RUNNING;
	ready_queue = ready_queue->next;/*update the head*/
	return temp;
}

extern void ready(void* process){
	struct pcb *next_p = process; 
	struct pcb *curr_p = ready_queue;

	if (ready_queue == ready_tail && next_p == ready_queue){
		return;/*if the queue and tail are the same and also same as the process we are readying, we return*/
	}

	if(next_p != ready_queue){/*else we put it to the tail and update the tail*/
		ready_tail->next = next_p;
		next_p->next = NULL;
		next_p->state = YIELD;
		ready_tail = ready_tail->next;
		return; 
	}

	else{
		ready_queue = ready_queue->next;
		ready_tail->next = next_p;
		next_p->state = YIELD;
		ready_tail = ready_tail->next;
		next_p->next = NULL;
		return;
	}
}

extern void cleanup(void* process){
	struct pcb *tmp = process;
	if (tmp == ready_queue){/*if the process we are killing is the start of ready queue, and also there are only one in the queue, we set it to NULL*/
		if (ready_queue == ready_tail){
			ready_tail = NULL;
		}
		ready_queue = ready_queue->next;
		tmp -> next = free_queue;
		tmp->state = STOP;/*change its state to STOP*/
		free_queue = tmp;
		kfree(tmp->stack_aloc);/*free the memSLot for the stack*/
	}
	else { 
		tmp -> next = free_queue;
		tmp->state = STOP;
		free_queue = tmp;
		kfree(tmp->stack_aloc);
	}
}