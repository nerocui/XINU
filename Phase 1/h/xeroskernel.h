/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */

#define CREATE 0
#define YIELD 1
#define STOP 2
#define RUNNING 3
/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

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
void           set_evec(unsigned int xnum, unsigned long handler);
extern void    kmeminit(void);
extern void    *kmalloc(int size);
extern void    kfree(void *ptr);

extern void    dispatch(void);
extern void*    next(void);
extern void    ready(void* process);
extern void     cleanup(void *process);
extern int    contextswitch(void* ptr);

extern int create(void (*func)(void), int stack);
extern int syscall(int call, void (*func)(void), int stack);
extern int syscall1(int call);
extern unsigned int syscreate(void (*func)(void), int stack);
extern void sysyield(void);
extern void sysstop(void);
extern void producer(void);
extern void consumer(void);
extern void root(void);
extern void realfunc2(void);
extern void realfunc3(void);
extern void realfunc(void);

struct memHeader {
  unsigned long size;/*size of the mem slot*/
  struct memHeader *prev;/*the previoous one in the linked list*/
  struct memHeader *next;/*the next one*/
  char *sanityCheck;
  unsigned char dataStart[0];/*this is where the memslot start for the process*/
};

 struct memHeader *memSlot;
 struct memHeader *headSlot;
 struct memHeader *tailSlot;

struct pcb{
	int PID;/*process id*/
	int state;/*as defined at top, running, stop, yield etc..*/
	int parent_id;/*the id of the process who created this process*/
  unsigned long esp;/*stack pointer*/
  struct context_frame *cf;/*context frame*/
	struct pcb *next;/*pointer to the next on in the queue*/
  int stack;/*the size of the stack*/
  unsigned long stack_aloc;/*address for where the stack was allocated*/
  int result;/*the result from create, 1 for success, 0 failure*/
};

struct context_frame{
  unsigned long edi;
  unsigned long esi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned long iret_eip;/*the first instruction*/
  unsigned long iret_cs;/*code segment*/
  unsigned long eflags;/*flags*/
}; 


/* Anything you add must be between the #define and this comment */
#endif
