/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

struct pcb main_block[32];
struct pcb idle;
struct pcb *idle_ptr;
struct message message_buffer[32];

 struct pcb *ready_queue;
 struct pcb *ready_tail;
 struct pcb *block_queue;
 struct pcb *block_tail;
 struct pcb *free_queue;
 struct pcb *free_tail;
 struct message *buffer;
 struct message *buffer_tail;
 struct message *free_buffer;
 struct message *free_buffer_tail;

 struct pcb *sleep_queue;

extern struct memHeader *memSlot;
extern struct memHeader *headSlot; /*The head of the memSlot*/
extern struct memHeader *tailSlot; /*The tail of the memSlot*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{
  
  char str[1024];
  int a = sizeof(str);
  int b = -69;
  int i; 

  kprintf( "\n\nCPSC 415, 2016W1 \n32 Bit Xeros 0.01 \nLocated at: %x to %x\n", 
	   &entry, &end); 
  
  kprintf("Some sample output to illustrate different types of printing\n\n");

  /* A busy wait to pause things on the screen, Change the value used 
     in the termination condition to control the pause
   */

  for (i = 0; i < 3000000; i++);

  /* Build a string to print) */
  sprintf(str, 
      "This is the number -69 when printed signed %d unsigned %u hex %x and a string %s.\n      Sample printing of 1024 in signed %d, unsigned %u and hex %x.",
	  b, b, b, "Hello", a, a, a);

  /* Print the string */

  kprintf("\n\nThe %dstring is: \"%s\"\n\nThe formula is %d + %d = %d.\n\n\n", 
	  a, str, a, b, a + b);

  for (i = 0; i < 4000000; i++);
  /* or just on its own */
  kprintf(str);

  /* Add your code below this line and before next comment */


  kmeminit();
  contextinit();

  int block_num = 0;
  /*initialize main_block which is the 32 pcb we have*/
  free_queue = &main_block[block_num];
  while(block_num != 32){
      main_block[block_num].next = &main_block[block_num + 1];/*linking the pcbs*/
      message_buffer[block_num].next = &message_buffer[block_num+1];
    main_block[block_num].state = STOP;
    main_block[block_num].esp = 0;
    main_block[block_num].parent_id = 0;
    struct context_frame new_cf;
    main_block[block_num].cf = &new_cf;
    main_block[block_num].args = 0;
    
    /**/
    message_buffer[block_num].dest_pid = 0;
    message_buffer[block_num].num = 0;
    message_buffer[block_num].from_pid = 0;
    
    /**/

    block_num++;
  }

  /*pointing the tail of free queue to the last block*/
  free_tail = &main_block[31];
  block_queue = NULL;
  block_tail = NULL;

  buffer = NULL;
  buffer_tail = NULL;
  free_buffer = &message_buffer[0];
  free_buffer_tail = &message_buffer[31];
  ready_queue = NULL;
  ready_tail = ready_queue;

  idle_ptr = &idle;  

  sleep_queue = NULL;
/*

  kprintf("Starting kmem test..\n");/*all testing cases are described in testing.txt*/
/*
  void* macbook = kmalloc(9999999999);
  if(macbook == 0){
    kprintf("Asking for mem larger than whats available works as it should. \n");
  }
  void *facebook = kmalloc(4568);
  void *whatsapp = kmalloc(7846);
  void *wechat = kmalloc(7844);
  void *twitter = kmalloc(9657);
  void *snapchat = kmalloc(4568);
  void *word = kmalloc(7846);
  void *camera = kmalloc(4568);
  void *steam = kmalloc(7846);
  
  if(wechat != 0)
    kfree(wechat);
  if(camera != 0)
    kfree(camera);
  if(snapchat != 0)
    kfree(snapchat);
  if(facebook != 0)
    kfree(facebook);
  if(whatsapp != 0)
    kfree(whatsapp);
  if(twitter != 0)
    kfree(twitter);
  if(word != 0)
    kfree(word);
  if(steam != 0)
    kfree(steam);
  

  kprintf("Allocated and freed 8 apps, calculating memSlot integrity...\n");
  if(headSlot == (long)freemem && tailSlot == ((long)maxaddr - (long)HOLEEND));
    kprintf("HeadSlot tailSlot matches freemem (maxaddr-HOLEEND), mem is working fine.\n");

*/
  kprintf("Creating root...");
  create(&root, 4096);
  kprintf("-root created-");
  create_idle(&idle_proc, 4096);
  kprintf("-idle create Success-");
  dispatch();


  for (i = 0; i < 2000000; i++);
  /* Add all of your code before this comment and after the previous comment */
  /* This code should never be reached after you are done */
  kprintf("\n\nWhen the kernel is working properly ");
  kprintf("this line should never be printed!\n");
  for(;;) ; /* loop forever */
}


