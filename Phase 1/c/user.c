/* user.c : User processes
 */

#include <xeroskernel.h>

/* Your code goes here */

int counter;

extern void consumer(void){
	int i;
	for(i = 0; i < 12; i++){
		kprintf("Happy 101st ");
		sysyield();
	}
	sysstop();
}

extern void producer(void){
	int i;
	for(i = 0; i < 15; i++){
		kprintf("birthday UBC ");		
		sysyield();
	}
	sysstop();	
}




extern void realfunc2(){
	int i;
	for(i = 0; i < 15; i++){
		kprintf("Im process 1 and I say: %d --", counter);
		counter ++;
		sysyield();
	}
	sysstop();
}

extern void realfunc3(){
	int i;
	for(i = 0; i < 15; i++){
		kprintf("Im process 2 and I say: %d --", counter);
		counter ++;
		sysyield();
	}
	sysstop();
}

extern void realfunc(){
	syscreate(*realfunc2, 6523);
	syscreate(*realfunc3, 4623);
	for(;;){
		sysyield();
	}

}
extern void root(void){
	counter = 0;
	syscreate(*consumer, 5624);
	syscreate(*producer, 2235);

	syscreate(*realfunc, 2531);
	for(;;){
		sysyield();
	}
}
