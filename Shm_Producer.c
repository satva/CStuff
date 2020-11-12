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
char buffer[BUFSIZ];
int shmid;


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


while(running) {
while(shared_buf->written_by_you == 1) {
sleep(1);
printf("Waiting for Client.....\n");
}
 printf("\nEnter Some Text\n");
fgets(buffer,BUFSIZ,stdin);

strncpy(shared_buf->some_text,buffer,TXT_SZ);
shared_buf->written_by_you = 1;
if(strncmp(buffer,"end",3) == 0) {
  running = 0;
}
}

if(shmdt(shared_memory) == -1){
perror("shmdt failed\n");
exit(EXIT_FAILURE);
}
return 0;
} 
