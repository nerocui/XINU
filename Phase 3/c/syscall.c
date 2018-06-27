/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>


int syscall( int req, ... ) {
/**********************************/

    va_list     ap;
    int         rc;

    va_start( ap, req );

    __asm __volatile( " \
        movl %1, %%eax \n\
        movl %2, %%edx \n\
        int  %3 \n\
        movl %%eax, %0 \n\
        "
        : "=g" (rc)
        : "g" (req), "g" (ap), "i" (KERNEL_INT)
        : "%eax" 
    );
 
    va_end( ap );

    return( rc );
}

int syscreate( funcptr fp, size_t stack ) {
/*********************************************/

    return( syscall( SYS_CREATE, fp, stack ) );
}

void sysyield( void ) {
/***************************/
  syscall( SYS_YIELD );
}

 void sysstop( void ) {
/**************************/

   syscall( SYS_STOP );
}

unsigned int sysgetpid( void ) {
/****************************/

    return( syscall( SYS_GETPID ) );
}

void sysputs( char *str ) {
/********************************/

    syscall( SYS_PUTS, str );
}

unsigned int syssleep( unsigned int t ) {
/*****************************/

    return syscall( SYS_SLEEP, t );
}

// update it according to the assignment
int syskill(int pid, ...) {
    va_list ap;
    va_start(ap, pid);
    int signalNumber = va_arg(ap, int);
    int direct_kill = va_arg(ap, int);
    return syscall(SYS_KILL, pid, signalNumber, direct_kill);
}

int sysgetcputimes(processStatuses *ps) {
  return syscall(SYS_CPUTIMES, ps);
}

int syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *)) {
    return syscall(SYS_SIGHANDLER, signal, newhandler, oldHandler);
}

void syssigreturn(void *old_sp) {
    return syscall(SYS_SIGRET, old_sp);
}

int syswait(int pid) {
    return syscall(SYS_WAIT, pid);
}

int sysopen(int device_no) {
    return syscall(SYS_OPEN, device_no);
}

int sysclose(int fd) {
    return syscall(SYS_CLOSE, fd);
}

int syswrite(int fd, void *buff, int bufflen) {
    return syscall(SYS_WRITE, fd, buff, bufflen);
}

int sysread(int fd, void *buff, int bufflen) {
    return syscall(SYS_READ, fd, buff, bufflen);
}

int sysioctl(int fd, int num_of_args, ...) {
    //kprintf("in sysioctl, command is: %ld\n", command);
    va_list ap;
    va_start(ap, num_of_args);
    unsigned long command = va_arg(ap, unsigned long);
    if(num_of_args == 2){
        int new_eof = va_arg(ap, int);
        return syscall(SYS_IOCTL, fd, num_of_args, command, new_eof);
    }
    else{
        va_end(ap);
        return syscall(SYS_IOCTL, fd, num_of_args, command, 0);
    }
}