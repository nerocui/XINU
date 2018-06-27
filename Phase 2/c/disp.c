/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */

extern struct pcb *ready_queue;
extern struct pcb *block_queue;
extern struct pcb *block_tail;
extern struct pcb *free_queue;
extern struct pcb *ready_tail;
extern struct pcb *free_tail;
extern struct pcb main_block[32];
extern struct pcb idle;
extern struct pcb *idle_ptr;
extern struct pcb *sleep_queue;

struct pcb* process;
struct message *temp_m;
int t_pid;
extern struct message *buffer;

extern void dispatch(void){
	int temp;/*this temp is for storing the create function's result*/
	process = next();
	kprintf("finish next");
 
	int stack;
	void* fp;
	va_list ap;

	for( ;; ) {
		int request = contextswitch( process );
		switch(request) {
			case( CREATE ):/*if the syscall is CREATE, we call create*/
			
			ap = (va_list) process->args;
			fp = (void*) (va_arg( ap, int ));
			stack = va_arg( ap, int );

			temp = create(fp, stack); 
			process->result = temp;/*and we now can store the temp in pcb's result.*/
			break; 
			
			case( YIELD ): 
			ready( process ); /*in the case of YIELD, we call ready and then switch to the next pcb*/
			process = next(); 
			break; 
			
			case( STOP ): 

			//struct message *at_ptr = NULL;/*message that needs our attention, ptr used to index*/
			//struct message *at_head = NULL;/*the queue to store m that needs attention*/
			//at_ptr->next = NULL;
			//at_ptr->previous = NULL;
			//at_head->next = NULL;
			//at_head->previous = NULL;

			temp_m = buffer;
			t_pid = process->PID;
			while(temp_m != NULL){/*Find all the message that needs our attention*/
				struct pcb *proc_to_deal;
				if(t_pid == temp_m->dest_pid && t_pid == temp_m->from_pid){/*I died before I can receive*/
					proc_to_deal = get_proc(t_pid);
					proc_to_deal->result = -3;
					ready(proc_to_deal);
				}
				temp_m = temp_m->next;
			}
			cleanup( process ); /*upon STOP, we kill the pcb with cleanup*/
			//kprintf("I cleaned");
			process = next();
			break;
			
			case(GETPID):
			break;
			
			case(PUTS):
			ap = (va_list) process->args;
			kprintf("%s",va_arg(ap, long));
			break;
			
			case(KILL): 
			ap = (va_list) process->args;
			int pid_to_kill = va_arg(ap, int);
			if(pid_to_kill == process->PID){
				process->result = -2;
				break;
			}
			temp_m = buffer;
			t_pid = pid_to_kill;
			while(temp_m != NULL){
				struct pcb *proc_to_deal;
				if(t_pid == temp_m->dest_pid && t_pid == temp_m->from_pid){
					proc_to_deal = get_proc(t_pid);
					proc_to_deal->result = -3;
					ready(proc_to_deal);
				}
				temp_m = temp_m->next;
			}
			struct pcb *proc_to_kill = get_proc(pid_to_kill);
			
			if(proc_to_kill == NULL){
				process->result = -1;
				goto bottom;
			}
			cleanup(proc_to_kill);
			bottom:
			break;

			case(SEND):
			kprintf("-got in SEND-");
			ap = (va_list) process->args;
			int dest_pid = va_arg(ap, int);
			int num = va_arg(ap, int);
			unsigned long *buffer_ptr = &num;
			struct pcb *recvr = get_proc(dest_pid);
			process->result = send(dest_pid, buffer_ptr, process);
			if(recvr->state != BLOCK){
				block(process);
				kprintf("-blocked sender-");
			}
			else{	
					ready(recvr);
					ready(process);
			}
			print_queue();
			process = next();
			kprintf("-INNNNNN SEEENNNDDD PIIIIDDDD is: %d-", process->PID);
			before_break:
			break;

			case(RECV):
			kprintf("-got in RECV-");
			ap = (va_list) process->args;
			unsigned int *from_pid = va_arg(ap, unsigned int*);
			unsigned long *buffer_pntr = va_arg(ap, unsigned long*);
			if(*from_pid == 0){
				block(process);
				goto bottom_t;
			}
			struct pcb *sender = get_proc(*from_pid);

			int not_block_for_me = 0;
			struct message *match_m = buffer;
			while(match_m != NULL){
				if(sender->PID == match_m->from_pid){
					goto outside;
				}
				kprintf("-1-");
				match_m = match_m->next;
			}
			outside:
			if(sender->state == BLOCK && match_m->dest_pid != process->PID){
				not_block_for_me = 1;
			}
			if(sender->state != BLOCK || not_block_for_me){
				block(process);
				kprintf("-blocked recvr, not_block_for_me is: %d-", not_block_for_me);
			}
			else{
				kprintf("-sender was blocked-");
				ready(sender);
				ready(process);
				void *pp = process;
				process->result = recv(from_pid, buffer_pntr, pp);
				kprintf("-back from recv, and the message is: %ld, and the addr of ptr is: %ld-", *buffer_pntr, buffer_pntr);
			}
			print_queue();
			bottom_t:
			process = next();
			break;
			
			case(TIMER_INT):
			tick();
			//kprintf("-ticked-");
			//print_sleep_queue();
			ready(process);
			process = next();
			end_of_intr();
			break;

			case(SLEEP):
			ap = (va_list) process->args;
			unsigned int millisec = va_arg(ap, unsigned int);
			process->result = sleep(process, millisec);
			process = next();
			break;
			
			default:
			break;
			
		}
		
	}
}

extern int no_proc(){
	int num_of_proc = 0;
	int i;
	for(i = 0; i < 32; i++){
		if(main_block[i].state == YIELD || main_block[i].state == RUNNING){
			num_of_proc++;
		}
	}
	kprintf("----there are %d processes----", num_of_proc);
	if(num_of_proc == 0)
		return 1;
	else return 0;

}

extern void* get_proc(int pid){
	int i;
	struct pcb *result;
	struct pcb *pcb_ptr = ready_queue;
	while(pcb_ptr != NULL){
		if(pcb_ptr->PID == pid){
			result = pcb_ptr;
			return result;
		}
		pcb_ptr = pcb_ptr->next;
	}
	if(pcb_ptr == NULL){
		pcb_ptr = block_queue;
		while(pcb_ptr != NULL){
		if(pcb_ptr->PID == pid){
			result = pcb_ptr;
			return result;
		}
		pcb_ptr = pcb_ptr->next;
		}
	}
}

extern void* next(void){
	struct pcb *next_p;
	if(ready_queue != NULL && ready_tail != NULL){
		//kprintf("-ready queue NOT NULL in next-");
		next_p = ready_queue;
	}
	else {
		kprintf("-next set IDLE-");
		next_p = &idle;
		next_p->state = RUNNING;
		return next_p;
	}
	next_p->state = RUNNING;
	if(ready_queue->next == NULL){
		ready_queue = idle_ptr;
		ready_tail = idle_ptr;
		ready_tail->next = NULL;
		return next_p;
	}
	ready_queue = ready_queue->next;
	kprintf("-Nexxxxxxttttt PID is: %d", next_p->PID);
	return next_p;

}

extern void ready(void* process){
	struct pcb *next_p = process; 

	if (ready_queue == ready_tail && next_p == ready_queue){
		return;/*if the queue and tail are the same and also same as the process we are readying, we return*/
	}

	if(ready_queue == idle_ptr){
		ready_queue = NULL;
		ready_tail = NULL;
	}

	if(next_p->state == RUNNING){
		next_p->state = YIELD;
		if(ready_queue == NULL && ready_tail == NULL){
			ready_queue = next_p;
			ready_tail = next_p;
			ready_tail->next = NULL;
		}
		else{
			ready_tail->next = next_p;
			ready_tail = ready_tail->next;
			ready_tail->next = NULL;
		}
	}

	else if(next_p->state == BLOCK){
		take_out(block_queue, next_p);
		next_p->state = YIELD;
		if(ready_queue == NULL && ready_tail == NULL){
			ready_queue = next_p;
			ready_tail = next_p;
			ready_tail->next = NULL;
		}
		else{
			ready_tail->next = next_p;
			ready_tail = ready_tail->next;
			ready_tail->next = NULL;
		}
	}

	else if(next_p->state == YIELD){
		take_out(ready_queue, next_p);
		next_p->state = YIELD;
		if(ready_queue == NULL && ready_tail == NULL){
			ready_queue = next_p;
			ready_tail = next_p;
			ready_tail->next = NULL;
		}
		else{
			ready_tail->next = next_p;
			ready_tail = ready_tail->next;
			ready_tail->next = NULL;
		}
	}

	return;
}

extern void block(void *process){
	struct pcb *next_p = process;
	next_p->state = BLOCK; 
	struct pcb *curr_p = block_queue;
	
	if(next_p->state == RUNNING){
		if (block_queue == block_tail && next_p == block_queue){
		return;/*if the queue and tail are the same and also same as the process we are readying, we return*/
		}

		if(block_queue == NULL && block_tail == NULL){
			block_queue = next_p;
			block_tail = next_p;
			block_tail->next = NULL;
		}
		else {
			block_tail->next = next_p;
			block_tail = block_tail->next;
			block_tail->next = NULL;
		}
	}
	else if(next_p->state == YIELD || next_p->state == CREATE){
		struct pcb *temp;
		if(ready_queue->next == NULL && ready_queue->PID != 999){
			temp = ready_queue;
			ready_queue = idle_ptr;
			ready_tail - idle_ptr;
			ready_tail->next = NULL;
		}else{
			temp = take_out(ready_queue, process);
		}
		block_tail->next = temp;
		block_tail = block_tail->next;
		block_tail->next = NULL;
		return;

	}
}

extern void* take_out(void *queue, void *next_p){
		struct pcb *np = next_p;
		struct pcb *curr = queue;
		struct pcb *next = curr->next;
		struct pcb *temp;
		if(next_p == curr)
			goto there;
		while(next != NULL){
			if(next->PID == np->PID){
				break;
			}
			curr = next;
			if(next->next != NULL)
				next = next->next;
			else next = NULL;
		}
		goto further;
	there:	
		if(queue == ready_queue)
			ready_queue = curr->next;
		else if(queue == block_queue)
			block_queue = curr->next;
		temp = curr;
		goto later;
	further:
		temp = next;
		curr->next = next->next;
	later:
		return temp;
}


extern void cleanup(void* process){
	kprintf("-KILL KILL KILL-");
	struct pcb *tmp = process;
	if(ready_queue->PID == 999){
		//print_queue();
		tmp->state = YIELD;
		return;
	}
	else if (tmp == ready_queue){/*if the process we are killing is the start of ready queue, and also there are only one in the queue, we set it to NULL*/
		if (ready_queue == ready_tail){
			//kprintf("-tmp=ready&q=t-");
			//print_queue();
			ready_queue->state = STOP;
			ready_tail = idle_ptr;
			ready_queue = idle_ptr;
			ready_tail->next = NULL; 
			kfree(tmp->stack_aloc);
			return;
		}
		ready_queue = ready_queue->next;
		free_tail->next = tmp;
		free_tail = free_tail->next;
		free_tail->next = NULL;
		tmp->state = STOP;/*change its state to STOP*/
		free_queue = tmp;
		kfree(tmp->stack_aloc);/*free the memSLot for the stack*/
	}
	else if(tmp->state == RUNNING){
		//kprintf("-RUNNING-");
		//print_queue();
		tmp->state = STOP;
		free_tail->next = tmp;
		free_tail = free_tail->next;
		free_tail->next = NULL;
		kfree(tmp->stack_aloc);
		return;
	}
	else { 
		//print_queue();
		//kprintf("--maybe take_out fucked up");
		if(ready_queue->next == NULL){
			free_tail->next = tmp;
			free_tail = free_tail->next;
			free_tail->next = NULL;
			kfree(tmp->stack_aloc);
			return;
		}

		tmp = take_out(ready_queue, tmp);
		//kprintf("--no it didnt");
		free_tail->next = tmp;
		free_tail = free_tail->next;
		free_tail->next = NULL;
		tmp->state = STOP;
		kfree(tmp->stack_aloc);
		return;
	}
}

void print_queue(){
	struct pcb *temp = ready_queue;
	int counter = 0;
	while(temp != NULL){
		kprintf("--%d th proc, pid: %d, state: %d", counter, temp->PID, temp->state);
		counter++;
		temp = temp->next;
	}
}

extern void print_sleep_queue(){
	struct pcb *temp = sleep_queue;
	int counter = 0;
	while(temp != NULL){
		kprintf("--%d th proc, pid: %d, state: %d", counter, temp->PID, temp->state);
		counter++;
		temp = temp->next;
	}
}