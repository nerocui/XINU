#include <xeroskernel.h>
#include <kbd.h>
#include <i386.h>
#include <stdarg.h>

int i;
int j;
int ret;
extern device_table dev_table[2];
int di_open(pcb *p, int device_no){

	if (device_no < 0 || device_no > 1){
		// there are only 2 devices
		// check the device num to be either 0 or 1
		return -1;
	}

	for( i = 0; i < 4; i++){//one process can open four device,
							//can not open the same one twice
		if (p->fd_table[i].major_num == device_no){
			return -1;
		}
	}

/*commented out b/c only one process running, 
no need to check if other process has the same table open
	for( i = 0; i < MAX_PROC; i++ ){
		if (proctab[i].state != STATE_STOPPED) {
			for ( j = 0; j < 4; j++ ){
				if (proctab[i].fd_table[j].major_num == device_no){
					return -1;
				}
			}
		}
	}
*/
	for( i = 0; i < 4; i++ ){
		if (p->fd_table[i].status == 0){
			//find the first available fd table entry
			p->fd_table[i].major_num = device_no;
			p->fd_table[i].minor_num = 0;
			p->fd_table[i].status = 1;
			// set the status to 1 means it is open
			p->fd_table[i].ptr = &dev_table[device_no];
			device_table *devptr;
			devptr = p->fd_table[i].ptr;
			(devptr->dvopen)(p, device_no);
			// call the corresponding function pointer in device table which
			// pointers to kbd_open (the real function for open)
			ret = i;
			// return the number of the file descriptor table
		    return ret;
		}
	}

	return -1;	
}

int di_close(pcb *p, int fd){

	// check if the number of fd is between 0 to 3
	if(fd < 0 || fd > 3){
		return -1;
	}

	if(p->fd_table[fd].status == 0){
		// already closed
		return -1;
	}

	// calling the close func
	device_table *devptr;
	devptr = p->fd_table[fd].ptr;
	(devptr->dvclose)(p, fd);

	// reset the value in this fd table since we close it
	p->fd_table[fd].major_num = 0;
	p->fd_table[fd].minor_num = 0;
	p->fd_table[fd].status = 0;
    p->fd_table[fd].ptr = NULL;


	return 0;
}

int di_write(pcb *p, int fd, void* buff, int bufflen){

	// check if the number of fd is between 0 and 3
	if (fd < 0 || fd > 3){
		return -1;
	}

	// check if this fd table is open
	if (p->fd_table[fd].status == 0){
		return -1;
	}

	// call the real write function
	device_table *devptr;
	devptr = p->fd_table[fd].ptr;

	// should return what kbd_write returns
	return (devptr->dvwrite)(p, fd, buff, bufflen);
}

int di_read(pcb *p, int fd, void* buff, int bufflen){

	//kprintf("in di_read\n");

	// check if the number of fd is between 0 and 3
	if (fd < 0 || fd > 3){
		kprintf("fd invalid\n");
		return -1;
	}

	// check if this fd table has been open
	if (p->fd_table[fd].status == 0){
		kprintf("not open\n");
		return -1;
	}

	// call the real read function
	device_table *devptr;
	devptr = p->fd_table[fd].ptr;

	return (devptr->dvread)(p, fd, buff, bufflen);
}

int di_ioctl(pcb *p, int fd, int num_of_arg, ...){

	// checking fd
	if (fd < 0 || fd > 3){
		return -1;
	}
	//kprintf("In di ioctl, num of arg is: %d\n", num_of_arg);
	// this function can take multiple parameter 
	va_list ap;
	va_start(ap, num_of_arg);
	unsigned long command = va_arg(ap, unsigned long);


	device_table *devptr;
	devptr = p->fd_table[fd].ptr;

	// check if the new_eof is not null, if not null then 
	// we need to set the new EOF
	if(num_of_arg == 2){
		int eof = va_arg(ap, int);
		return (devptr->dvioctl)(num_of_arg, command, eof);
	}else if(num_of_arg == 1){
		return (devptr->dvioctl)(num_of_arg, command, 0);
	}
	return -1;
}
