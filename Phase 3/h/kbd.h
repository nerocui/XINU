#include <xeroskernel.h>

int device_num;
//0 is non-echo
//1 is echo
int state1;
int eof;
char kbd_buffer[4];
pcb *kbd_block_head;
pcb *kbd_block_tail;


void kbdinit(void);
int kbd_open( int device_no);
int kbd_close(int fd);
int kbd_write(int fd, void *buff, int bufflen);
int kbd_read(pcb *p, int fd, void *buff, int bufflen);
int copy_all(void *buff, int bufflen);
void sort_kbd_buffer(void);
void print_kbd_buffer(void);
int kbd_unblock(pcb *p);
int ioctl_kbd(int num_of_arg, ...);