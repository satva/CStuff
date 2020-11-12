#include<stdio.h>
#include<sys/ipc.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<unistd.h>
#include<string.h>

#include "Shared_Common.h"

int main() {
int running = 1;
void *shared_memory = (void*)0;
struct shared_text *shared_buf;
int shmid;

srand((unsigned int)getpid());

//Getting the ID of the Shared Memory Segment with Key as 1234. If that ID is not available with Kernel, Exit Baby !!
shmid = shmget((key_t)1234,sizeof(struct shared_text),0666 | IPC_CREAT);
if(shmid == -1 ) {
  perror("shmget");
  exit(EXIT_FAILURE);
}

// Now Make the Shared Memory Access to the Program
shared_memory = shmat(shmid,(void *)0,0);
if(shared_memory == (void*) -1) {
  perror("shmat");
  exit(EXIT_FAILURE);
 }
printf("Memory Attached at %x",(int*)shared_memory);

shared_buf = (struct shared_text*) shared_memory;
shared_buf->written_by_you = 0;


while(running) {
if(shared_buf->written_by_you) {
printf("The Value Written was %s\n",shared_buf->some_text);
sleep( rand() % 4);
shared_buf->written_by_you = 0;
if(strncmp(shared_buf->some_text,"end",3) == 0) {
  running = 0;
}
}
}

if( shmdt(shared_memory) == -1 ) {
perror("shmdt");
exit(EXIT_FAILURE);
}

if( shmctl(shmid,IPC_RMID,0) == -1 ) {
perror("shctl");
exit(EXIT_FAILURE);
}

return 0;
} 
