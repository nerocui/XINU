/* user.c : User processes
 */

#include <xeroskernel.h>

/* Your code goes here */

int counter;
extern struct pcb *sleep_queue;

extern void consumer(void){
	syssleep(25000);
	int i;
	// for(i = 0; i < 12; i++){
	// 	sysputs("Happy 101st ");
	// 	sysyield();
	// }
	sysputs("consssssss");
}

extern void producer(void){
	syssleep(20000);
	int i;
	// for(i = 0; i < 12; i++){
	// 	sysputs("birthday UBC ");		
	// 	sysyield();
	// }
	sysputs("produccccccccccccccc");
}

extern void dealer(void){
	syssleep(20000);
	int i;
	for(i = 0; i < 12; i++){
		sysputs("Yeaaa ");		
		sysyield();
	}
}




extern void realfunc2(){
	int i;
	for(i = 0; i < 15; i++){
		if(i == 7){
			syskill(5);
		}
		
		char string[] = "Im process 2 and I say: ";
		char *ptr = &string;
		sysputs(string);
		sysyield();
	}
}

extern void realfunc3(){
	int i;
	for(i = 0; i < 15; i++){
		
		char string[] = "Im process 2 and I say: ";
		char *ptr = &string;
		sysputs(ptr);
		sysyield();
	}
}

extern void realfunc(){
	syscreate(*realfunc2, 6523);
	syscreate(*realfunc3, 4623);

}

extern void sender_proc(){
	kprintf("-got in sender-");
	syssend(3, 1);
}

extern void receiver_proc(){
	kprintf("-got in revcr_PROC-");
	unsigned long num;
	unsigned long *ptr = &num;
	int from_pid = 2;
	int *from_ptr = &from_pid;
	int result = sysrecv(from_ptr, ptr);
	sysputs("-NNNNNNever GOT BACK???-");
	if(result != 0){
		kprintf("-recv error code: %d", result);
	}
	if(num ==  1)
		kprintf("got that message");
	else
		sysputs("-didnt get message-");
}


extern void sender_proc2(){
	kprintf("-got in sender-");
	syssend(5, 1);
}

extern void receiver_proc2(){
	kprintf("-got in revcr-");
	unsigned long num;
	unsigned long *ptr = &num;
	int from_pid = 4;
	int result = sysrecv(&from_pid, ptr);
	if(result != 0){
		kprintf("-recv error code: %d", result);
	}
	if(num ==  1)
		kprintf("got that message");
	else
		sysputs("-didnt get message-");
}

extern void sender_proc3(){
	kprintf("-got in sender-");
	if(syssend(6, 1) == -2)
		sysputs("-4.1.3, error code -2, sending to self-");
}

extern void receiver_proc3(){
	kprintf("-got in revcr-");
	unsigned long num;
	unsigned long *ptr = &num;
	int from_pid = 7;
	int result = sysrecv(&from_pid, ptr);
	if(result != 0){
		kprintf("-recv error code: %d", result);
	}
	if(num ==  1)
		kprintf("got that message");
	else if(result == -2)
		sysputs("-4.1.4, error code -2, receiving from self-");
}

extern void root(void){
	/*
	syscreate(*sender_proc, 4096);
	syscreate(*receiver_proc, 4096);
	*/
	syscreate(*receiver_proc2, 4096);
	syscreate(*sender_proc2, 4096);
/*
	syscreate(*sender_proc3, 4069);
		
	syscreate(*receiver_proc3, 4096);
	*/
	//sysputs("-4.1.5, call old producer consumer without yielding, if printed, then works-");
	//syscreate(*producer, 4096);
	//syscreate(*consumer, 4096);
}


extern void idle_proc(){
	sysputs("-----Im in edle-----");
	for(;;){
	}
}

extern void wrapper(void (*func)(void)){
	func();
	//kprintf("Im gonna stop");
	sysstop();
}