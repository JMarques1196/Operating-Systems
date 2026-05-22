/*
**  file: pth03.c
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define N      1081080   /* N=2*3*4*5*7*9*11*13 */

double soma(double x[], int n); /* funcao que soma os elementos de um vector */ 
void *tarefa1(void *arg);       /* child thread */
void *tarefa2(void *arg);       /* sub-child thread */

double v[N],S;                  /* vector de dados e soma total */


int main()
{
   int i,r;
   pthread_t      trfid;     /* variavel para ID da tarefa */
   pthread_attr_t trfatr;    /* variavel para atributos da tarefa */
   
   printf("\nA iniciar a tarefa principal: PID=%d\n", (int) getpid());
   for(i=0; i<N; i++)
      v[i]= i+1;
   /* inicializar variavel de atributos com valores por defeito */
   pthread_attr_init(&trfatr);
   /* modificar estado de desacoplamento para "detached" */
   pthread_attr_setdetachstate(&trfatr, PTHREAD_CREATE_DETACHED);
   /* criar e iniciar execução de tarefa */
   r= pthread_create(&trfid, &trfatr, tarefa1, (void*) NULL);
   if( r ) {
      /* erro ! */
      perror("Erro na criacao da tarefa1!");
      exit(1);
   }
   sleep(4);     /* dar tempo para as sub-tarefas terminarem */

   printf("A terminar a tarefa principal.\n\n");
   return 0;
}

void *tarefa1(void *arg)
{
   int r;
   pthread_t      trfid;     /* variavel para ID da tarefa */
   pthread_attr_t trfatr;    /* variavel para atributos da tarefa */

   printf("A iniciar a sub-tarefa1: PID=%d\n", (int) getpid());
   /* inicializar variavel de atributos com valores por defeito */
   pthread_attr_init(&trfatr);
   /* modificar estado de desacoplamento para "detached" */
   pthread_attr_setdetachstate(&trfatr, PTHREAD_CREATE_DETACHED);
   /* criar e iniciar execução de tarefa */
   r= pthread_create(&trfid, &trfatr, tarefa2, (void*) NULL);
   if( r ) {
      /* erro ! */
      perror("Erro na criacao da tarefa2!");
      exit(1);
   }
   
   printf("A terminar a sub-tarefa1.\n");
   return (void*) NULL;
}

void *tarefa2(void *arg)
{
   printf("A iniciar a sub-sub-tarefa2: PID=%d\n", (int) getpid());
   S= soma(v,N);
   printf(" Soma= %.0lf\n",S);
   printf("A terminar a sub-sub-tarefa2.\n");
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
