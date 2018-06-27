
#include <xeroskernel.h>
#include <i386.h>
#include <kbd.h>
#include <stdarg.h>

extern void kbdinit(void){
    device_num = 1;
    state1 = 1;
    eof = 4;/*the value for d when control is down*/
    kbd_buffer[0] = NULL;
    kbd_buffer[1] = NULL;
    kbd_buffer[2] = NULL;
    kbd_buffer[3] = NULL;
    kbd_block_head = NULL;
    kbd_block_tail = NULL;
}

extern int kbd_open(int device_no){
    device_num = device_no;
    // enable the keyboard interrupts
    enable_irq(1,0);
    return 0;
}

extern int kbd_close(int fd){

    // disable keyboard interrupts
    enable_irq(1,1);
    return 0;
}

extern int kbd_write(int fd, void *buff, int bufflen){
    // we can not write to a keyboard
    return -1;
}

extern int kbd_read(pcb *p, int fd, void *buff, int bufflen){
    int count = bufflen;
    //kprintf("in kbd read \n");
        //print_kbd_buffer();
        //kprintf("K buffer not full\n");
        /*we sort kbd_buffer to not have empty bytes at beginning*/ 
        /*so the first byte determines the emptyness*/
        int left = copy_all(buff, bufflen);
        /*left returns opsitive number of char left in kbd_buffer*/
        /*left returns negative number of char left to be copied, kbd_buffer empty*/
        if(left == 0 && sizeof(buff) == bufflen){
            /*read over, return*/
            kprintf("finished read\n");
            return 0;
        }
        else if(left < 0){
            //kprintf("left smaller than 0\n");
            /*block q, let it know it need to copy -left many chars still*/
            p->bufferlen = -left;
            //kprintf("In KBD_READ, p->buffer before is: %s\n", buff);
            //kprintf("In KBD_READ, p->bufferlen is: %d\n", p->bufferlen);
            //kprintf("In KBD_READ, p->buffer addr before is: %ld\n", buff);
            p->buffer = buff + (bufflen + left);
            //kprintf("In KBD_READ, p->buffer after is: %s\n", buff);
            //kprintf("In KBD_READ, p->buffer addr after is: %ld\n", p->buffer);
            kbd_block(p);
            return 0;
        }
        else if(left > 0){
            /*move kbd_buffer's leftover toward the start*/
            sort_kbd_buffer();
            /*read over, return*/
            kprintf("finished read\n");
            return 0;
        }
}


extern int copy_all(void *buff,int bufflen){
    //kprintf("in copy all\n");
    int counter = 0;
    int result = 0;
    
    if(bufflen == 0)
        return 0;/*if bufferlen is 0, return 0*/
    int i;
    for(i = 0; kbd_buffer[i]; i++){/*count number of chars in temp*/
        counter++;
        //kprintf("1\n");
    }

    result = counter - bufflen;/*set result be the difference as planed*/
    //kprintf("left = %d\n", result);
    
    if(counter > bufflen){/*set counter to be the actual num_to_copy*/
        counter = bufflen;
    }
    char *temp2 = buff;
    for(i = 0; i < counter; i++){/*copy...*/
        *temp2 = kbd_buffer[i];
        kbd_buffer[i] = 0;
        temp2++;
    }
    return result;
}

extern void sort_kbd_buffer(void){
    int x_i = 0;/*x is to be blank*/
    int size_i = 4;
    char *ptr = &kbd_buffer;
    while(*ptr == 0){
        x_i++;
        ptr++;
    }
    int todo = size_i - x_i;

    char temp;
    int i;
    for(i = 0; i < todo; i++){
        temp = kbd_buffer[i];
        kbd_buffer[i] = kbd_buffer[i + x_i];
        kbd_buffer[i + x_i] = temp;
    }
}

extern void print_kbd_buffer(void){
    char *temp = &kbd_buffer;
    kprintf("KBD BUFFER: %s\n", temp);
}

int kbd_block(pcb *p){
    //kprintf("in block\n");
    p->next = NULL;
    p->state = STATE_KBD_BLOCK;
    if( kbd_block_tail ) {
        kbd_block_tail->next = p;
        p->prev = kbd_block_tail;
        // Modified
    } else {
        kbd_block_head = p;
        p->prev = NULL;
        // Modified
    }

    kbd_block_tail = p;
    //print_pcb(kbd_block_head);
    return 0;
}

extern int kbd_unblock(pcb *p){
     if (!kbd_block_head) {
        return -1;
    }

  if (kbd_block_head == p) { // At front of list
    // kprintf("Pid %d is at front of list\n", p->pid);
    kbd_block_head = p->next;

    // If the implementation has idle on the ready list this next state1ment
    // isn't needed. However, it is left just in case someone decides to 
    // change things so that the idle process is kept separate. 

    if (kbd_block_head == NULL) { // If the implementation has idle process  on the 
      kbd_block_tail = kbd_block_head;      // ready list this should never happen
    }
  }
  else {  // Not at front, find the process.
    pcb * prev = kbd_block_head;
    pcb * curr;
    
    for (curr = kbd_block_head->next; curr!=NULL; curr = curr->next) {
      if (curr == p) { // Found process so remove it
  // kprintf("Found %d in list, removing\n", curr->pid);
        prev->next = p->next;
        if (kbd_block_tail == p) { //last element in list
            kbd_block_tail = prev;
      // kprintf("Last element\n");
        }
        p->next = NULL; // just to clean things up
        break;
      }
      prev = curr;
    }
    if (curr == NULL) {
      kprintf("Kernel bug: kbd block queue corrupt, process %d claimed on queue and not found\n",
        p->pid);
      
    }
  }
}

extern int ioctl_kbd(int num_of_arg, ...){
    //TODO, add parameter
    kprintf("in ioctl_kbd\n");
    va_list ap;
    va_start(ap, num_of_arg);
    kprintf("In ioctl kbd\n");
    unsigned long command = va_arg(ap, unsigned long);
    if(num_of_arg == 2 && command == 53){
        kprintf("In 53 case\n");
        eof = va_arg(ap, int);
    }
    else if(command == 55 && num_of_arg == 1){
        kprintf("In 55 case\n");
        state1 = 0;
    }
    else if(command == 56 && num_of_arg == 1){
        kprintf("In 56 case\n");
        state1 = 1;
    }
    else {
        kprintf("In else case\n");
        return -1;
    }
}
