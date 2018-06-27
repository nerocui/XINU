/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

int waitee_ID;

int priority_ID;

void common_stop(){
  sysputs("gonna be killed");
  sysstop();
}

void busy( void ) {
  int myPid;
  char buff[100];
  int i;
  int count = 0;

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
  myPid = sysgetpid();
  
  for (i = 0; i < 10; i++) {
    sprintf(buff, "My pid is %d\n", myPid);
    sysputs(buff);
    if (myPid == 2 && count == 1) syskill(3, 9);
    count++;
    sysyield();
  }
}



void sleep1( void ) {
  int myPid;
  char buff[100];

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
  myPid = sysgetpid();
  sprintf(buff, "Sleeping 1000 is %d\n", myPid);
  sysputs(buff);
  syssleep(1000);
  sprintf(buff, "Awoke 1000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep2( void ) {
  int myPid;
  char buff[100];

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
  myPid = sysgetpid();
  sprintf(buff, "Sleeping 2000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(2000);
  sprintf(buff, "Awoke 2000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep3( void ) {
  int myPid;
  char buff[100];

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
  myPid = sysgetpid();
  sprintf(buff, "Sleeping 3000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(3000);
  sprintf(buff, "Awoke 3000 from my nap %d\n", myPid);
  sysputs(buff);
}








void producer( void ) {
/****************************/

    int         i;
    char        buff[100];

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);

    // Sping to get some cpu time
    for(i = 0; i < 100000; i++);

    syssleep(3000);
    for( i = 0; i < 20; i++ ) {
      
      sprintf(buff, "Producer %x and in hex %x %d\n", i+1, i, i+1);
      sysputs(buff);
      syssleep(1500);

    }
    for (i = 0; i < 15; i++) {
      sysputs("P");
      syssleep(1500);
    }
    sprintf(buff, "Producer finished\n");
    sysputs( buff );
    sysstop();
}

void consumer( void ) {
/****************************/

    int         i;
    char        buff[100];

    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
    for(i = 0; i < 50000; i++);
    syssleep(3000);
    for( i = 0; i < 10; i++ ) {
      sprintf(buff, "Consumer %d\n", i);
      sysputs( buff );
      syssleep(1500);
      sysyield();
    }

    for (i = 0; i < 40; i++) {
      sysputs("C");
      syssleep(700);
    }

    sprintf(buff, "Consumer finished\n");
    sysputs( buff );
    sysstop();
}


void waiter(){
    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
    sysputs("Im waiter and im trying to wait\n");
    syswait(waitee_ID);
    sysputs("Im waiter and Im back b/c my waitee is dead\n");
    for(;;){
      sysputs("1");
    }
}

void waitee(){
    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
    sysputs("Im the waitee and im gonna kill waiter\n");

    int kill_result = syskill(2, 9);
    sysyield();
    for(;;);
}


int string_check(char *first, char *second){
  while (*first == *second) {
      if (*first == '\0' || *second == '\0')
         break;
 
      first++;
      second++;
   }
 
   if (*first == '\0' && *second == '\0')
      return 1;
   else
      return 0;
}

int space_check(char *first, char *second){
  if(*first == second)
    return 1;

  else return 0;
}


void ps(){
  processStatuses psTab;
  int proc = sysgetcputimes(&psTab);
  sysputs("   PID               STATUS          CPUTIME\n");
  sysputs("====================================================\n");
  int i = 0;
  while(proc){
    char *status;
    switch(psTab.status[i]){
      case(0):
        status = "STATE_STOPPED";
      break;
      case(1):
        status = "STATE_READY";
      break;
      case(22):
        status = "STATE_SLEEP";
      break;
      case(23):
        status = "STATE_RUNNING";
      break;
      case(333):
        status = "STATE_WAIT_BLOCK";
      break;
      case(344):
        status = "STATE_KBD_BLOCK";
      break;
    }

    char temp[50];
    sprintf(temp, "   %d              %s         %ld\n", psTab.pid[i], status, psTab.cpuTime[i]);
    sysputs(temp);
    proc--;
    i++;
  }
}

void t(){
  for(;;){
    sysputs("T\n");
    syssleep(5000);
    
  }
}

void alarm(){
    //sysputs("In alarm\n");
    syskill(1, 15);
}

void print_alarm(){
  sysputs("ALARM ALARM ALARM\n");
}

void help(){
  sysputs("============================HELP=============================\n");
  sysputs(">ps<.................................print all living process\n");
  sysputs(">ex/Enter key<.................................exit the shell\n");
  sysputs(">k<...................press enter to launch the kill program,\n");
  sysputs(".....................and then enter the ID of the process you\n");
  sysputs("..................................................wish to end\n");
  sysputs(">a<..................press enter to launch the alarm program,\n");
  sysputs(".....................and then enter the ticks you wish to set\n");
  sysputs(">t<................simply print the letter T every 10 seconds\n");
  }

void test1(){
  syskill(priority_ID, 1);
}

void test2(){
  syskill(priority_ID, 2);
}

void test3(){
  syskill(priority_ID, 3);
}

void t_handler1(){
  sysputs("I should run last\n");
}

void t_handler2(){
  sysputs("I should run second\n");
}

void t_handler3(){
  sysputs("I should run first\n");
}

void priority_test(){
  int test_id1 = syscreate(test1, 4096);
  int test_id2 = syscreate(test2, 4096);
  int test_id3 = syscreate(test3, 4096);
  syssighandler(1, t_handler1, NULL);
  syssighandler(2, t_handler2, NULL);
  syssighandler(3, t_handler3, NULL);
  sysyield();
  sysputs("back from 3 handlers\n");
}

void open_test(){
  int open = sysopen(7);
  if(open == -1){
    sysputs("Invalid arguments\n");
  }
}

void write_test(){

  char buff_w[50];
  char *buff_ptr = &buff_w;

  sysopen(1);
  int write = syswrite(7, buff_ptr, 10);
  if(write == -1){
    sysputs("Invalid fd\n");
  }
}

void ioctl_test(){
  //sysopen(1);
  int io = sysioctl(1, 1, 77, 0);
  if(io == -1){
    sysputs("Invalid command\n");
  }
}

int parser(char *command, funcptr *func){
  //sysputs("in parser\n");
    /*if command end with &*/
    char *screenfetch_s = "screenfetch";
    char *test = command;
    char *last_char;
    char *t_s = "t";
    char *ps_s = "ps";
    char *ex_s = "ex";
    char *k_s = "k";
    char *a_s = "a";
    char *space = " ";
    char *curr_eof = "";
    char *and_s = "&";
    char *help_s = "--help";
    char *priority = "pt";
    char *open_t = "ot";
    char *write_t = "wt";
    char *ioctl_t = "it";    

    char result[50];
    while(*test != ' ' && *test != (char)0){
      *last_char = *test;
      test++;
      //sysputs("2");
    }
    if(*last_char == '&'){
      //sysputs("1");
      return -1;
    }
    else{ /*first word is the command*/

    
      char *first_arg = &result;
      char *first_arg_ptr = first_arg;
      test = command;
      while(*test != (char)0 && *test != ' '){
        *first_arg_ptr = *test;
        test++;
        first_arg_ptr++;
        //sysputs("3");
      }
      *first_arg_ptr = (char)0;
      if(string_check(first_arg, ps_s)){
        *func = &ps;
        return 0;
      }else if(string_check(first_arg, ex_s) || string_check(first_arg, curr_eof)){
        return 1;
      }else if(string_check(first_arg, k_s)){
        return 2;
      }else if(string_check(first_arg, a_s)){
        *func = &alarm;
        return 3;
      }else if(string_check(first_arg, t_s)){
        //kprintf("in t case\n");
        *func = &t;
        return 0;
      }else if(string_check(first_arg, help_s)){
        *func = &help;
        return 0;
      }else if(string_check(first_arg, priority)){
        *func = &priority_test;
        return 4;
      }else if(string_check(first_arg, open_t)){
        *func = &open_test;
        return 0;
      }else if(string_check(first_arg, write_t)){
        *func = &write_test;
        return 0;
      }else if(string_check(first_arg, ioctl_t)){
        *func = &ioctl_test;
        return 0;
      }
      else return -2;
  }
}


void     root( void ) {
/****************************/

    char  buff[100];
    char  buff2[100];
    int pids[5];
    int proc_pid, con_pid;
    int i;

    sysputs("Root has been called\n");
    void **old_ptr;
    syssighandler(9,&common_stop, old_ptr);
    // Test for ready queue removal. 

    /*
    int waiter_pid = syscreate(&waiter, 4096);
    int waitee_pid = syscreate(&waitee, 4096);
    waitee_ID = waitee_pid;
    sprintf(buff, "The waiter is %d, the waitee is %d\n", waiter_pid, waitee_pid);
    sysputs(buff);

    sysyield();
    sysyield();

    for(;;);
    */
    int fd = sysopen(1);
    sysioctl(fd, 2, 53, (int)'\n');
    enter_username:
    sysioctl(fd, 1, 56, 0);
    sysputs("Please enter user name[cpsc415]:\n");
    char *username;
    username = "cpsc415";
    sysread(fd, username, 20);
    if(username != "cpsc415"){
      goto enter_username;
    }
    enter_password:
    sysioctl(fd, 1, 55, 0);
    sysputs("Please enter password[EveryoneGetsAnA]:\n");
    char *password;
    password = "EveryoneGetsAnA";
    sysread(fd, password, 50);
    if(password != "EveryoneGetsAnA")
      goto enter_password;
    sysioctl(fd, 1, 56, 0);
    /*here*/
    prompt:
    sysputs(">");
    
    funcptr func;
    char buff5[50];
    char *command = &buff5;
    int result;
    int re;
    result = 0;
    sysread(fd, command, 50);

    char buff4[50];
    /*run command, when command returns, go back to there*/
    sprintf(buff4, "\nYour command is: %s\n", command);
    sysputs(buff4);
    result = parser(command, &func);
    if(result == 0){
      //sysputs("in result = 0 case\n");
      int func_pid = syscreate(func, 4096);
      syswait(func_pid);
    }
    else if(result == -2){
      sysputs("Command not found, use --help to see all the available commands.\n");
    }
    else if(result == 1){
        sysputs("gonna exit shell\n");
        goto exit_shell;
    }
    else if(result == 2){
      sysputs("Please enter the process ID you want to kill:\n");
      char *char_pid;
      sysread(fd, char_pid, 20);
      //int pid_to_kill = *char_pid - '0';
      int pid_to_kill = atoi(char_pid);
      //kprintf("The pid to kill is %d\n", pid_to_kill);
      int kill_result = syskill(pid_to_kill, -1, -1);
      if(kill_result == -1){
        sysputs("Kill was unsuccessful\n");
      }
    }
    else if(result == 3){
      
      //sysputs("in result = 3 case\n");
      sysputs("Please enter the ticks for your alarm:\n");
      char *ticks;
      sysread(fd, ticks, 20);
      syssighandler(15, *print_alarm, NULL);
      //kprintf("The addr of handler is: %ld\n", print_alarm);
      //int ticks_alarm = *ticks - '0';
      int ticks_alarm = atoi(ticks);
      //kprintf("sleep time is gonna be: %d\n", ticks_alarm);
      int sleep_result = syssleep(ticks_alarm);
      //sysputs("got back from sleep\n");
      int alarm_pid = syscreate(func, 4096);
      //int wait_result = syswait(alarm_pid);
      sysyield();
      //kprintf("Your wait result is: %d\n", wait_result);
      void **old_alarm;
      syssighandler(15, NULL, old_alarm);
    }
    else if(result == 4){
      priority_ID = syscreate(func, 4096);
      syswait(priority_ID);
    }


    command = NULL;
    for(re = 0; re < 50; re++){
      buff5[re] = (char)0;
    }
    goto prompt;

    exit_shell:

    for(;;);


    sysread(fd, &buff2, 10);
    sysputs(&buff2);
    sysioctl(fd, 2, 53, 32);
    char buff3[10];
    sysread(fd, &buff3, 10);
    sysputs(&buff3);
    for(;;){
      //sysputs("1");
    }

   /*
    proc_pid = syscreate(&busy, 1024);
    con_pid = syscreate(&busy, 1024);
    sysyield();
    syskill(proc_pid, 9);
    sysyield();
    syskill(con_pid, 9);

    
    for(i = 0; i < 5; i++) {
      pids[i] = syscreate(&busy, 1024);
    }

    sysyield();
    
    syskill(pids[3], 9);
    sysyield();
    syskill(pids[2], 9);
    syskill(pids[4], 9);
    sysyield();
    syskill(pids[0], 9);
    sysyield();
    syskill(pids[1], 9);
    sysyield();

    syssleep(8000);;



    kprintf("***********Sleeping no kills *****\n");
    // Now test for sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Sleeping kill 2000 *****\n");
    // Now test for removing middle sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(110);
    syskill(pids[1], 9);
    syssleep(8000);;

    kprintf("***********Sleeping kill last 3000 *****\n");
    // Now test for removing last sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syskill(pids[2], 9);
    syssleep(8000);;

    kprintf("***********Sleeping kill first process 1000*****\n");
    // Now test for first sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(100);
    syskill(pids[0], 9);
    syssleep(8000);;

    // Now test for 1 process


    kprintf("***********One sleeping process, killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syskill(pids[0], 9);
    syssleep(8000);;

    kprintf("***********One sleeping process, not killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Three sleeping processes***\n");    // 
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);


    // Producer and consumer started too
    proc_pid = syscreate( &producer, 4096 );
    con_pid = syscreate( &consumer, 4096 );
    sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
    sysputs( buff );


    processStatuses psTab;
    int procs;
    



    syssleep(500);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
        psTab.cpuTime[j]);
      kprintf(buff);
    }


    syssleep(10000);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
        psTab.cpuTime[j]);
      kprintf(buff);
    }

    */

    sprintf(buff, "Root finished\n");
    sysputs( buff );
    sysstop();
    
    for( ;; ) {
     sysyield();
    }
    
}

