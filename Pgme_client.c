#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#define N 5

int sem, memp;
bool *Mdispo;
struct sembuf Pnfree = {0, -1, 0};
struct sembuf Vnfree = {0, 1, 0};
struct sembuf Pmutex = {1, -1, 0};
struct sembuf Vmutex = {1, 1, 0};

int Allocate()
{
  bool *Mdispo;
  int num = 0;
  /*Segment attachment*/
  Mdispo = shmat(memp, 0, 0);

  semop(sem, &Pnfree, 1);
  semop(sem, &Pmutex, 1);

  while (Mdispo[num] == 1)
  {
    num = num + 1;
  }
  Mdispo[num] = 1;

  semop(sem, &Vmutex, 1);

  /*Segment detachment*/
  shmdt(Mdispo);

  return num;
}

/**Return procedure **/
void Return(num)
{
  bool *Mdispo;

  /*Segment attachment*/
  Mdispo = shmat(memp, 0, 0);
  semop(sem, &Pmutex, 1);

  Mdispo[num] = 0;
  semop(sem, &Vmutex, 1);
  semop(sem, &Vnfree, 1);

  /*Segment detachment*/
  shmdt(Mdispo);
}

int main(void)
{

  key_t key;
  int machine;

  /*Retrieve semaphore*/
  key = ftok("main.c", 4);
  sem = semget(key, 2, 0666);
  if (sem == -1)
  {
    printf("\n Error retrieving semaphores");
    exit(2);
  }

  /*Retrieve segment*/

  key = ftok("main.c", 5);

  memp = shmget(key, N * sizeof(bool), 0666);
  if (memp == -1)
  {
    printf("\n Error retrieving shared memory");
    exit(2);
  }

  machine = Allocate();
  printf("\n I am the client with pid %d, I occupy the machine %d \n ", getpid(), machine);

  printf("\n I am the client with pid %d, I release the machine N=%d\n", getpid(), machine);

  Return(machine);

  sleep(rand() % 20);

  return 0;
}