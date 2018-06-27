/* sleep.c : sleep device 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */

extern struct pcb *sleep_queue;
extern struct pcb *ready_queue;
extern struct pcb *ready_tail;
extern struct pcb *idle_ptr;

extern int sleep(void* process, unsigned int millisec){

	struct pcb *proc1 = process;
	if(proc1->PID == 999){
		kprintf("----We can not sleep the idle process");
		return;
	}
	kprintf("----We are in sleep----");

	if(proc1->state == YIELD){
		if(!ready_queue->next && ready_queue->PID != 999){
			ready_queue = idle_ptr;
	 	    ready_queue->next = NULL;
	 	    ready_tail = idle_ptr;
	 	    ready_tail->next = NULL;
		}
	 	
	 	proc1->state = SLEEP;
	 	kprintf("----Process was yield, now is sleep----");
	 }

	// take_out(ready_queue,proc1);
	// if(ready_queue == NULL){
	// 	ready_queue = idle_ptr;
	// 	ready_queue->next = NULL;
	// 	ready_tail = idle_ptr;
	// 	ready_tail->next = NULL;
	// }
	
	// if(proc1->state == RUNNING){
	// 	proc1->state = SLEEP;
	// 	kprintf("----Process was running, now is sleep----");
	// }

	proc1->sleep_time = millisec_to_tick(millisec);
	proc1->relative_sleep_time = millisec_to_tick(millisec);
	struct pcb *origin_next;

	// No process in the sleep queue, just add proc1 to the queue
	if(!sleep_queue){
		sleep_queue = proc1;
		kprintf("----Sleep queue was null, now proc1 is in it----");
	}

	else{
		// add to the front
		if(proc1->sleep_time < sleep_queue->sleep_time){
			proc1->next = sleep_queue;
	 		sleep_queue = proc1;
      		sleep_queue->next->relative_sleep_time = sleep_queue->next->sleep_time - sleep_queue->sleep_time;
      		kprintf("----Add it to the front----");
		}
		else{

			// add it between 2 processes
			// Since we do not want to modify the head of ready queue in this case, we create a check points to sleep queue
			struct pcb *check = sleep_queue;
			
			while(check->next){
				if(proc1->sleep_time >= check->sleep_time && 
					proc1->sleep_time < check->next->sleep_time){
					origin_next = check->next; // store the original next because we need to update it later
					check->next = proc1;
					proc1->relative_sleep_time = proc1->sleep_time - check->sleep_time;
					proc1->next = origin_next;
					origin_next->relative_sleep_time = origin_next->sleep_time - proc1->sleep_time;
					kprintf("----Add it to the middle----");
					break;
				}
				check = check->next;
			}

			// add to the end
			if(!check->next){
				check->next = proc1;
				proc1->relative_sleep_time = proc1->sleep_time - check->sleep_time;
				kprintf("----Add it to the end----");
			}
		}
	}
	return 0;
}

int millisec_to_tick(int millisec){
	int clock_ticks = 0;
    if (millisec <= 10) {
    	return 1;
    }
	clock_ticks = (millisec)/10 + ((millisec % 10)?1:0);
	return clock_ticks;
}

extern void tick(){
	if(sleep_queue == NULL){
		//kprintf("---- Function tick: nothing in sleep queue ----");
		return;
	}
	
	sleep_queue->relative_sleep_time--;
	struct pcb *temp = sleep_queue;
	struct pcb *temp_next;
	while(temp->relative_sleep_time == 0 && temp != NULL){
		temp_next = temp->next;
		ready(temp);
		kprintf("---- Sleeping process is awaked ----");
		temp = temp_next;
	}
	sleep_queue = temp;
}