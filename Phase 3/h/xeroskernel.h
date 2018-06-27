/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define FALSE   0       /* Boolean constants             */
#define TRUE    1
#define EMPTY   (-1)    /* an illegal gpq                */
#define NULL    0       /* Null pointer for linked lists */
#define NULLCH '\0'     /* The null character            */

#define CREATE_FAILURE -1  /* Process creation failed     */



/* Universal return constants */

#define OK            1         /* system call ok               */
#define SYSERR       -1         /* system call failed           */
#define EOF          -2         /* End-of-file (usu. from read) */
#define TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define INTRMSG      -4         /* keyboard "intr" key pressed  */
                                /*  (usu. defined as ^B)        */
#define BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


/* Some constants involved with process creation and managment */
 
   /* Maximum number of processes */      
#define MAX_PROC        64           
   /* Kernel trap number          */
#define KERNEL_INT      80
   /* Interrupt number for the timer */
#define TIMER_INT      (TIMER_IRQ + 32)
   /* Minimum size of a stack when a process is created */
#define PROC_STACK      (4096 * 4)    
   /* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10

   /* Maximu number of signals  */
#define MAX_SIGNAL      32            

#define KBD_INT         33
    /*int number for keyboard*/          


/* Constants to track states that a process is in */
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_SLEEP     22
#define STATE_RUNNING   23
#define STATE_BLOCK     333
#define STATE_KBD_BLOCK 344
#define STARTING_EFLAGS         0x00003000
#define ARM_INTERRUPTS          0x00000200
/* System call identifiers */
#define SYS_STOP        10
#define SYS_YIELD       11
#define SYS_CREATE      22
#define SYS_TIMER       33
#define SYS_GETPID      144
#define SYS_PUTS        155
#define SYS_SLEEP       166
#define SYS_KILL        177
#define SYS_CPUTIMES    178

#define SYS_SIGHANDLER  188
#define SYS_SIGRET      199
#define SYS_WAIT        222

#define SYS_OPEN        233
#define SYS_CLOSE       244
#define SYS_WRITE       255
#define SYS_READ        266
#define SYS_IOCTL       277
#define SYS_KBDINT      288

/* A typedef for the signature of the function passed to syscreate */
typedef void    (*funcptr)(void);
/* Structure to track the information associated with a single process */
typedef struct struct_pcb pcb;
typedef struct struct_handler handler;
typedef struct devsw device_table;
struct struct_handler{                 /*the handler installed to a process*/
  int sig_no;                          /*the signal number associated with it*/
  int signalled;
  funcptr *handler_func;                       /*the handler method passed by the process*/
  handler *next;         /*the pointer to the next handler if applicable*/
};

typedef struct file_descriptor_table fd_table;
struct file_descriptor_table{
  int major_num;
  int minor_num;
  int status;           // 1 means open, 0 means close
  device_table *ptr;    // pointer the device table
};


typedef struct devsw device_table;
struct devsw {
  int dvnum;            // device num 
  int dvminor;          // device minor num, did not really use it

  int (*dvopen)();      // only have the 5 function pointers which we need to 
  int (*dvclose)();     // use in this assignment
  int (*dvread)();
  int (*dvwrite)();
  int (*dvioctl)();

};

struct struct_pcb {
  void        *esp;    /* Pointer to top of saved stack           */
  pcb         *next;   /* Next process in the list, if applicable */
  pcb         *prev;   /* Previous proccess in list, if applicable*/
  int          state;  /* State the process is in, see above      */
  unsigned int pid;    /* The process's ID                        */
  int          ret;    /* Return value of system call             */
                       /* if process interrupted because of system*/
                       /* call                                    */
  long         args;   
  unsigned int otherpid;
  char        *buffer;
  int          bufferlen;
  int          sleepdiff;
  long         cpuTime;  /* CPU time  consumed                    */
  handler       handler_table[32];   // handler table for 32 signal
  handler      *hndlr;               // pointer to the address of the first handler
  int          is_signalled;         // if 0 not signalled, otherwise signaled
  pcb          *waiting;             // the process which this process is waiting
  pcb          *blockee;             // the queue of processes which is wating for this process
  pcb          *block_tail;          // the tail of the above queue 

  fd_table     fd_table[4];          // file descriptor table
};

typedef struct struct_ps processStatuses;
struct struct_ps {
  int  pid[MAX_PROC];      // The process ID
  int  status[MAX_PROC];   // The process status
  long  cpuTime[MAX_PROC]; // CPU time used in milliseconds
};

typedef struct struct_argument argument;    // use for trampoline, but did not use it eventually
struct struct_arguments{
  unsigned long ret_addr;
  funcptr *handler_func;
  unsigned long *cntx;
};




/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
  unsigned long        edi;
  unsigned long        esi;
  unsigned long        ebp;
  unsigned long        esp;
  unsigned long        ebx;
  unsigned long        edx;
  unsigned long        ecx;
  unsigned long        eax;
  unsigned long        iret_eip;
  unsigned long        iret_cs;
  unsigned long        eflags;
  unsigned long        stackSlots[];
} context_frame;


/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

void     kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( size_t );





/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, size_t stack );
int      create_idle( funcptr fp, size_t stackSize );
void     idleproc( void );
void     set_evec(unsigned int xnum, unsigned long handler);
void     printCF (void * stack);  /* print the call frame */
int      syscall(int call, ...);  /* Used in the system call stub */
void     sleep(pcb *, unsigned int);
void     removeFromSleep(pcb * p);
void     removeFromBlock(pcb *p, pcb *blocker);
void     removeFromReady(pcb * p);
void     tick( void );
int      getCPUtimes(pcb * p, processStatuses *ps);
pcb     *findPCB(int pid);
handler *findHandler(pcb *p, int signal);
void     sigtramp(funcptr handler_func, void *cntx);
pcb     *findBlockee(int pid);
void     print_pcb(pcb *p);
extern unsigned int kbtoa( unsigned char code );

/* Function prototypes for system calls as called by the application */
int          syscreate( funcptr fp, size_t stack );
void         sysyield( void );
void         sysstop( void );
unsigned int sysgetpid( void );
unsigned int syssleep(unsigned int);
void         sysputs(char *str);
//int          syskill(int pcb);
int          sysgetcputimes(processStatuses *ps);

int          syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *));
void         syssigreturn(void *old_sp);
int          syskill(int pid, ...); 
int          syswait(int pid);

int          sysopen(int device_no);
int          sysclose(int fd);
int          syswrite(int fd, void *buff, int bufflen); 
int          sysread(int fd, void *buff, int bufflen);
int          sysioctl(int fd, int num_of_args, ...);

int          di_open(pcb *p, int device_no);
int          di_close(pcb *p, int fd);
int          di_write(pcb *p, int fd, void* buff, int bufflen);
int          di_read(pcb *p, int fd, void* buff, int bufflen);
int          di_ioctl(pcb *p, int fd, int num_of_args, ...);

/* The initial process that the system creates and schedules */
void     root( void );




void           set_evec(unsigned int xnum, unsigned long handler);


/* Anything you add must be between the #define and this comment */
#endif

