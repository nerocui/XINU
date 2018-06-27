/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <kbd.h>

extern  int entry( void );  /* start of kernel image, use &start    */
extern  int end( void );    /* end of kernel image, use &end        */
extern  long  freemem;  /* start of free memory (set in i386.c) */
extern char *maxaddr; /* max memory address (set in i386.c) */
device_table dev_table[2];


/*------------------------------------------------------------------------
 *  The idle process 
 *------------------------------------------------------------------------
 */
void idleproc( void )  
{
    int i;
    //kprintf("I");
    for( i = 0; ; i++ ) {
      //kprintf("1");
       sysyield();
    }
}



/************************************************************************/
/***        NOTE:             ***/
/***                      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***                      ***/
/***                      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )       /* The beginning */
{
  kprintf( "\n\nCPSC 415, 2016W1 \n32 Bit Xeros 0.01 \nLocated at: %x to %x\n",&entry, &end); 
  
  /* Your code goes here */
  
  kprintf("Max addr is %d %x\n", maxaddr, maxaddr);
  
  kmeminit();
  kprintf("memory inited\n");
  
  dispatchinit();
  kprintf("dispatcher inited\n");
  
  contextinit();
  kprintf("context inited\n");

  kbdinit();
  kprintf("Keyboard inited\n");

  // initialize the device table
  int i;
  for(i = 0; i < 2; i++){
    dev_table[i].dvnum = i;
    dev_table[i].dvminor = 0;

    // set the function pointers to the address of the actual function
    dev_table[i].dvopen = &kbd_open;
    dev_table[i].dvclose = &kbd_close;
    dev_table[i].dvread = &kbd_read;
    dev_table[i].dvwrite = &kbd_write;
    dev_table[i].dvioctl = &ioctl_kbd;
  }
  
  // WARNING THE FIRST PROCESS CREATED MUST BE THE IDLE PROCESS.
  // See comments in create.c
  
  // Note that this idle process gets a regular time slice but
  // according to the A2 specs it should only get a time slice when
  // there are no other processes available to run. This approach 
  // works, but will give the idle process a time slice when other 
  // processes are available for execution and thereby needlessly waste
  // CPU resources that could be used by user processes. This is 
  // somewhat migigated by the immediate call to sysyield()
  kprintf("Creating Idle Process\n");

  create_idle(idleproc, PROC_STACK);
  
  create( root, PROC_STACK );
  kprintf("create inited\n");
  
  dispatch();
  
  
  kprintf("Returned to init, you should never get here!\n");
  
  /* This code should never be reached after you are done */
  for(;;) ; /* loop forever */
}

