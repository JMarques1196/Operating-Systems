/*
**  file: pth02.c
**
Bi-threaded application that calculates the sum of the first N integers, where N is a product of several prime numbers. The main thread initializes an array with the integers from 1 to N and creates a detached thread to compute the sum. The main thread then sleeps for a short period to allow the detached thread to complete its computation before exiting. The program demonstrates the use of pthreads for concurrent execution and synchronization through sleeping.
Notes:
Differences between using fork() and pthreads:
- fork() will create a new process with its own memory space
- pthreads will create a new thread that shares the same memory space as the parent thread
- fork() is generally more resource-intensive than pthreads, as it involves duplicating the entire process, while pthreads only creates a new thread within the same process
- Communication between processes created with fork() typically requires inter-process communication (IPC) mechanisms, while threads can communicate directly through shared memory
- Shared memory introduces risks of race conditions and requires synchronization mechanisms (like mutexes) to ensure thread safety, while processes created with fork() are isolated and do not share memory, thus avoiding these issues.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define N      1081080   /* N=2*3*4*5*7*9*11*13 */

double soma(double x[], int n); 
void *tarefa(void *arg);        

double v[N],S;                 


int main()
{
   int i,r;
   pthread_t      trfid;     /* Task Id */
   pthread_attr_t trfatr;    /* Variable for task attributes */
   
   printf("\nA iniciar a tarefa principal: PID=%d\n", (int) getpid());
   for(i=0; i<N; i++)
      v[i]= i+1;
   /* Initialize task attributes with default values */
   pthread_attr_init(&trfatr); // pthread_attr_init() initializes the thread attributes object pointed to by trfatr with default values. This is necessary before setting any specific attributes for the thread, such as the detach state.
   /* Set detach state to detached */
   pthread_attr_setdetachstate(&trfatr, PTHREAD_CREATE_DETACHED); // pthread_attr_setdetachstate() sets the detach state attribute in the thread attributes object pointed to by trfatr. By passing PTHREAD_CREATE_DETACHED, we specify that the thread created with these attributes will be detached, meaning it will automatically release its resources upon termination and cannot be joined by other threads.
   /* create and start execution of task */
   r= pthread_create(&trfid, &trfatr, tarefa, (void*) NULL); // pthread_create() creates a new thread of execution. The first argument is a pointer to a pthread_t variable where the thread ID will be stored. The second argument is a pointer to a pthread_attr_t structure that specifies the attributes for the thread (in this case, we set it to detached). The third argument is the function that the thread will execute (tarefa), and the fourth argument is a pointer to any arguments that need to be passed to the thread function (in this case, we pass NULL since tarefa does not require any arguments).
   if( r ) { // If pthread_create() returns a non-zero value, it indicates that an error occurred during thread creation. In this case, we print an error message using perror() and exit the program with a non-zero status to indicate failure.
      /* erro ! */
      perror("Erro na criacao da tarefa!");
      exit(1);
   }
   /* Time for sub-task to complete */
   sleep(4);     /* This is actually a bad practice since it makes the program non-responsive and the child task might take longer than expected */

   printf("A terminar a tarefa principal.\n\n");
   return 0;
}

void *tarefa(void *arg)
{
   printf("A iniciar a sub-tarefa: PID=%d\n", (int) getpid());
   S= soma(v,N);
   printf(" Soma= %.0lf\n",S);
   printf("A terminar a sub-tarefa.\n");
   return (void*) NULL;
}

double soma(double x[], int n)
{
   int i;
   double s=0;
   
   for(i=0; i<n; i++)
      s+= x[i];

   return s;
}

/* EOF */
