/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>

extern int syscall(int call, ... ){
    
    int result = 0; /*reault if to store the call no for later return*/
    va_list ap;
    int pid = -1;
    va_start( ap, call );

    __asm __volatile("  \
        movl %2, %%eax  \n\
        movl %3, %%edx   \n\
        int $128    \n\
        movl %%eax, %0   \n\
        movl %%edi, %1     \n\
        "
        : "=g" (result), "=g" (pid)
        : "g" (call), "g" (ap)
        : "%eax"
    );/*we put call in asm as input field and result as output. we store call in eax, fun in edx and stack size in ecx for ctsw to use*/

    va_end( ap );
    if(call == GETPID){
        return pid;
    }
    return result;
}

extern unsigned int syscreate( void (*func)(void), int stack ){
    return syscall( CREATE, *func, stack );
}


extern void sysyield( void ){
    syscall(YIELD);
}

extern void sysstop( void ){
    syscall(STOP);
}

extern int sysgetpid( void ){
    return syscall(GETPID);
}

extern void sysputs( char *str ){
    syscall(PUTS, str);
}

extern int syskill( int pid ){
   return syscall(KILL, pid);
}

extern int syssend(int dest_pid, unsigned long num){
    return syscall(SEND, dest_pid, num);
}

extern int sysrecv(unsigned int *from_pid, unsigned long *num){
    return syscall(RECV, from_pid, num);
}

extern unsigned int syssleep(unsigned int milliseconds){
    return syscall(SLEEP, milliseconds);
}

