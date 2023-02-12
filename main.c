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

int main(void)
{
  key_t key, keySem;
  int p, err;
  bool Mdispo;
  / Creating the semaphores * /

      key = ftok("main.c", 4);
  sem = semget(key, 2, IPC_CREAT | 0666);
  if (sem == -1)
  {
    printf("\n Error in creating the semaphores");
    exit(1);
  }

  /* Initializing the semaphores */

  semctl(sem, 0, SETVAL, N); // nfree
  semctl(sem, 1, SETVAL, 1); // mutex

  /* Creating the shared memory segments */

  key = ftok("main.c", 5);

  memp = shmget(key, N * sizeof(bool), IPC_CREAT | 0666);
  if (memp == -1)
  {
    printf("\n Error in creating the shared memory");
    exit(2);
  }

  /* Attaching */
  Mdispo = shmat(memp, 0, 0);

  /* Initializing Mdispo */
  for (int i = 0; i < N; i++)
  {
    Mdispo[i] = 0;
  }

  /* Creating child processes */

  for (int i = 0; i < 10; i++)
  {
    p = fork();
    if (p == 0)
    {
      err = execl("./Pgme_client", "Pgme_client", NULL);
      printf("\n Error in loading client program N° %d\n", err);
      exit(5);
    }
  }

  p = wait(NULL);
  while (p != -1)
  {
    printf("\n client finished its work %d", p);
    p = wait(NULL);
  }
  /* Detaching the memory segment */
  shmdt(Mdispo);

  /* Destroying the semaphores and memory segments */
  semctl(sem, IPC_RMID, 0);
  shmctl(memp, IPC_RMID, 0);

  printf("\n End of the process");
  return 0;
}