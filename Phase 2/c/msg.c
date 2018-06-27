/* msg.c : messaging system 
   This file does not need to modified until assignment 2
 */
 #include <xeroskernel.h>
/* Your code goes here */

extern struct message *buffer;
extern struct message *buffer_tail;
extern struct message *free_buffer;
extern struct message *free_buffer_tail;

extern int send(int dest_pid, unsigned long *buffer, void *sender){
	struct pcb *sendr = sender;
	struct pcb *recvr = get_proc(dest_pid);
	if(recvr->state == STOP)
		return -1;
	else if(dest_pid == sendr->PID)
		return -2;
	if(free_buffer == NULL)
		return -3;
	
 	struct message *m = free_buffer;
	free_buffer = free_buffer->next;

  	m->dest_pid = dest_pid;
  	m->num = *buffer;
  	m->from_pid = sendr->PID;
  	insert(m);
  	/*return value is complecated due to stuff*/
	return 0;


}

extern int recv(unsigned int *from_pid, unsigned long *buffer_ptr, void *recvr){
	struct pcb *sender = get_proc(*from_pid);
	struct pcb *receiver = recvr;
	kprintf("-the sender ID is: %d , the recvr ID is: %d -", *from_pid, receiver->PID);
	if(sender->state == STOP)
		return -1;
	else if(*from_pid == receiver->PID)
		return -2;
	struct message *temp;
	temp = buffer;
	if(buffer == NULL){
		kprintf("-recv returned -1-");
		return -1;
	}
	while(temp!=NULL){
		if((temp->dest_pid == receiver->PID && temp->from_pid == *from_pid)
			||
		  (temp->dest_pid == receiver->PID && *from_pid == 0)){
			break;
		}
		else
			temp = temp->next;
	}

	*buffer_ptr = temp->num;
	delete(temp);
	kprintf("-recv returned 0, and the message is: %ld , and the addr of ptr is: %ld-", *buffer_ptr, buffer_ptr);
	return 0;
}

extern void insert(void *msg){
	struct message *m = msg;
	
	if(buffer == NULL && buffer_tail == NULL){
		buffer = m;
		buffer_tail = m;
		buffer_tail->next = NULL;
		buffer->previous = NULL;
		return;
	}
	else{
		buffer_tail->next = m;
		buffer_tail = buffer_tail->next;
		buffer_tail->next = NULL;
		return;
	}

}



extern void delete(void *msg){
	struct message *m = msg;
	struct message *prev = m->previous;
	struct message *next = m->next;
	if(prev == NULL && next == NULL)
		return;
	else if(buffer == m && buffer_tail == m){/*only has one*/
		free_buffer_tail->next = m;
		m->previous = free_buffer_tail;
		free_buffer_tail = free_buffer_tail->next;
		free_buffer_tail->next = NULL;
		buffer = NULL;
		buffer_tail = NULL;
	}
	else if(prev == NULL){/*deleting head*/
		free_buffer_tail->next = m;
		free_buffer_tail = free_buffer_tail->next;
		free_buffer_tail->next = NULL;
		buffer = buffer->next;
		buffer->previous = NULL;
	}
	else if(next == NULL){
		free_buffer_tail->next = m;
		free_buffer_tail = free_buffer_tail->next;
		free_buffer_tail->next = NULL;
		buffer_tail = buffer_tail->previous;
		buffer_tail->next = NULL;
	}
}