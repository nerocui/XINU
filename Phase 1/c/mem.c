/* mem.c : memory manager
 */

#include <xeroskernel.h>
#include <i386.h>
#include <stdio.h>
#include <limits.h>

/* Your code goes here */

extern long freemem;
extern char* maxaddr;

extern struct memHeader *memSlot;
extern struct memHeader *headSlot;
extern struct memHeader *tailSlot;

extern void kmeminit(void){

    struct memHeader *befhole;/*this is the memSlot before the hole*/
    struct memHeader *afthole;/*thie is after the hole*/
    befhole = freemem;
    afthole = HOLEEND;
    befhole->size = (long)HOLESTART - (long)freemem;
    befhole->next = afthole;
    
    
    afthole->size = (long)maxaddr - (long)HOLEEND;
    afthole->prev = befhole;

    memSlot = befhole;
    headSlot = memSlot;
    headSlot->prev = NULL;
    tailSlot = afthole;
    tailSlot->next = NULL;
    /*kprintf("%ld\n", befhole->size);*/
}

extern void* kmalloc(int size){
    /*kprintf("\n The size you want is %d.\n", size);*/
    long amnt;
    amnt = (size)/16 + ((size)?1:0);
    amnt = amnt*16 + sizeof(struct memHeader);
    //kprintf("The amount we are giving you is %ld.", amnt);

    /*search for suitable size of memory*/
    long closest = LONG_MAX;
    struct memHeader *currSlot = headSlot;
    struct memHeader *wantSlot = headSlot;
    while(currSlot != NULL){
        if((currSlot->size - amnt) > 0 && (currSlot->size - amnt) < closest){
            wantSlot = currSlot;
            closest = currSlot->size - amnt;
        }
        
        currSlot = currSlot->next;
    }
    if(closest == LONG_MAX){
        kprintf("Your required amount of memory is not available, go buy more RAM.");
        return 0;
    }
    /*end*/


    /*cut the wantSlot and update the linkedlist*/
    struct memHeader *newSlot;
    newSlot = wantSlot;
    newSlot += amnt/16;
    newSlot->size = wantSlot->size - amnt;
    if(wantSlot->prev != NULL){
        wantSlot->prev->next = newSlot;
        newSlot->prev = wantSlot->prev;
    }
    if(wantSlot->next != NULL){
        wantSlot->next->prev = newSlot;
        newSlot->next = wantSlot->next;
    }
    /*end*/
    if(wantSlot == headSlot){
        headSlot = newSlot;
        //kprintf("We cut the head.");
    }
    if(wantSlot == tailSlot){
        tailSlot = newSlot;
        //kprintf("We cut the Tail.");
    }
    wantSlot->size = amnt;
    return (char*)(wantSlot + 1);
}

extern void kfree(void *ptr){/*im just gonna use all the commented out kprints for comment*/
    //kprintf("The returning ptr is %ld. \n", ptr);
    struct memHeader *realSlot;
    realSlot = ptr - 16;
    //kprintf("The just contructed realSlot has an addr of %ld.\n", realSlot);
    //kprintf("%ld\n",realSlot->size);
    int done = 0;

    struct memHeader *currSlot;
    currSlot = headSlot;
    //kprintf("The currSlot has an addr of %ld.\n", currSlot);
    
    //kprintf("The thing is!!!!! %ld.\n", (long)(realSlot + (realSlot->size)/16)+1);
    //kprintf("Searching for where to return the memory...\n");
    int counter = 0;
    while(done==0){
        counter++;
        /*for case such that if the returned one is continuous with some other memslot*/
        if(realSlot == (long)(currSlot + (currSlot->size)/16)){/*the returned one is from the end of the curr*/

            //kprintf("You got in end-continuous-case.\n");
            currSlot->size += realSlot->size;
            //kprintf("The returned memory is from the end of %d memory.\n", counter);
            //kprintf("Its addr is %ld. \n", realSlot);
            //kprintf("The addr of the one in front of it is %ld. \n", currSlot);
            //kprintf("The size of the one in front of it is %ld. \n", currSlot + (currSlot->size)/16);
            done = 1;
        }
        else if((long)(realSlot + (realSlot->size)/16) == currSlot){/*the returned one is form the start of the curr*/
            //kprintf("You got in start-continuous-case.\n");
            if(currSlot->prev != NULL){                    /*need to chage pointers of the from-start-case*/ 
                currSlot->prev->next = realSlot;           /*because pointers points to/from the start, no*/
            }                                              /*need to modify pointers for from-end-case.*/
            if(currSlot->next != NULL){
                currSlot->next->prev = realSlot;
            }
            //kprintf("The returned memory is from the start of %d memory.\n", counter);
            //kprintf("Its addr is %ld. \n", realSlot);
            //kprintf("The returned size is %lu. \n", realSlot->size);
            //kprintf("The addr of the one behind it is %ld. \n", currSlot);
            realSlot->size += currSlot->size;
            done = 1;
        }
        /*end*/

        /*for case such that the returned one is between the curr and curr->next*/
        else if(realSlot > currSlot + (currSlot->size)/16 && realSlot + (realSlot->size)/16 < currSlot->next){
            //kprintf("You got in between-non-continuous-case.\n");
            currSlot->next->prev = realSlot;
            currSlot->next = realSlot;
            //kprintf("The returned memory is between memory %d and %d. And they are not connected.\n", counter, counter + 1);
            //kprintf("Its addr is %ld. \n", realSlot);
            //kprintf("The addr of the one in front of it is %ld. \n", currSlot);
            //kprintf("The size of the one in behind it is %ld. \n", currSlot->next);
            done = 1;
        }
        
        else if(realSlot + (realSlot->size)/16 < headSlot && !(long)(realSlot + (realSlot->size)/16+1 == currSlot)){ /*for case such that the returned is before the head*/
            //kprintf("You got in head-non-continuous-case.\n");
            realSlot->next = headSlot;            /*and it is not continuous*/
            headSlot->prev = realSlot;
            //kprintf("The returned memory is from the head, and they are not connected. \n");
            //kprintf("Its addr is %ld. \n", realSlot);
            //kprintf("The addr of the head is %ld. \n", headSlot);
            headSlot = realSlot;
            //kprintf("The addr of the head is now %ld. \n", headSlot);
            done = 1;
        }

        else if(realSlot > tailSlot + (tailSlot->size)/16 && !(long)(realSlot == currSlot + (currSlot->size)/16+1)){/*for case such that the returned is after the tail*/
            //kprintf("You got in tail-non-continuous-case.\n");
            tailSlot->next = realSlot;           /*and not continuous*/
            realSlot->prev = tailSlot;
            //kprintf("The returned memory is from the tail, and they are not connected. \n");
            //kprintf("Its addr is %ld. \n", realSlot);
            //kprintf("The addr of the tail is %ld. \n", tailSlot);
            tailSlot = realSlot;
            //kprintf("The addr of the tail is now %ld. \n", tailSlot);
            done = 1;
        }

        else currSlot = currSlot->next;


        /*end*/

    }
}