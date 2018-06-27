/* syscall.c : syscalls
 */

#include <xeroskernel.h>

/* Your code goes here */

extern int syscall(int call, void (*func)(void), int stack){/*this one has three parameters for only syscreate*/
    int result = 0; /*reault if to store the call no for later return*/
    __asm __volatile("  \
        movl %1, %%eax  \n\
        movl 12(%%ebp), %%edx   \n\
        movl 16(%%ebp), %%ecx   \n\
        int $128    \n\
        movl %%eax, %0   \n\
            "
        :"=r"(result)
        :"r"(call)
        :"%eax"
    );/*we put call in asm as input field and result as output. we store call in eax, fun in edx and stack size in ecx for ctsw to use*/

    return result;
}

extern int syscall1(int call){/*this one has one parameter for sysyield and sysstop*/
    int result = 0;
    __asm __volatile("  \
        movl %1, %%eax  \n\
        int $128    \n\
        movl %%eax, %0   \n\
            "
        :"=r"(result)
        :"r"(call)
        :"%eax"
    );

    return result;
}

extern unsigned int syscreate(void (*func)(void), int stack){
    return syscall(CREATE, *func, stack);
}


extern void sysyield(void){
    syscall1(YIELD);
}

extern void sysstop(void){
    syscall1(STOP);
}
