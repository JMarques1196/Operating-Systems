/*
Multiple Forks
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define NITER  2

int main()
{
   pid_t pid; // pid_t is a type used to store processes
   int i;
   
   // The code follows the same logic as fork01, except it loops, and forks for each iteration.
   printf("\nExemplo de aplicacao 02 da funcao fork()\n");
   printf("Processo pai inicial tem PID=%5d\n", (int) getpid());
   for(i=1; i<=NITER; i++) {
      // For each i we create a new fork.
      // Fork is called for every process created in the iteration.
      // First iteration, it's called just from the parent
      // Second Iteration, it's called by the parent and the first children created by the fork, and so on
/*            P0 (34387)
            /           \
      F1 (34388)      F2 (34389)
           \
           F3 (34390) 
           
*/
      pid= fork();
      if ( pid==-1 ) {
         perror("Erro na funcao fork()");
         exit(1);
      }
      if ( pid ) {
         /* pid>0, codigo para o processo pai */
         printf("Codigo do Pai  (i=%2d):  PID=%5d  PPID=%5d\n", \
            i, (int) getpid(), (int) getppid());
      }
      else {
         /* pid=0, codigo para o processo filho */
         printf("Codigo do Filho(i=%2d):  PID=%5d  PPID=%5d\n", \
            i, (int) getpid(), (int) getppid());
      }
   }
   return 0;
}

/* 
The expected number of processes is 2ˆNITER

OUTPUT:
Processo pai inicial tem PID=34387
Codigo do Pai  (i= 1):  PID=34387  PPID=31435 // 1st Process
Codigo do Pai  (i= 2):  PID=34387  PPID=31435
Codigo do Filho(i= 1):  PID=34388  PPID=34387 // 2nd Process
Codigo do Filho(i= 2):  PID=34389  PPID=    1 // 3rd Process
Codigo do Pai  (i= 2):  PID=34388  PPID=    1
Codigo do Filho(i= 2):  PID=34390  PPID=    1 // 4th Process

2ˆ2 = 4, which is verified.

*/