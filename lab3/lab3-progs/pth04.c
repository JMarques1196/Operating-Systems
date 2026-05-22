/*
**  
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N      1081080   /* N=2*3*4*5*7*9*11*13 */

double soma(double x[], int n); /* funcao que soma os elementos de um vector */ 
void *tarefa(void *arg);        /* prototipo funcao tarefa */

double v[N],S,S1;                   /* vector de dados, soma total e parcial */
typedef struct {double *x; int n;} trfarg_t;  /* tipo do argumento da tarefa */


int main()
{
   int i,r;
   pthread_t      trfid;     /* variavel para ID da tarefa */
   pthread_attr_t trfatr;    /* variavel para atributos da tarefa */
   trfarg_t       trfarg;    /* variavel para argumento da tarefa */
   double        *trfret;    /* variavel para retorno da tarefa */
   
   printf("\nA iniciar a tarefa principal.\n");
   for(i=0; i<N; i++)
      v[i]= i+1;
   /* inicializar variavel de atributos com valores por defeito */
   pthread_attr_init(&trfatr);
   /* modificar estado de desacoplamento para juntavel */
   pthread_attr_setdetachstate(&trfatr, PTHREAD_CREATE_JOINABLE);
   /* inicializar estrutura de dados que serve de argumento da tarefa */
   trfarg.x= &v[0];        /* 1a metade da soma para a tarefa a criar */
   trfarg.n= N/2;
   /* criar e iniciar execução de tarefa */
   r= pthread_create(&trfid, &trfatr, tarefa, (void*) &trfarg);
   if( r ) {
      /* erro ! */
      perror("Erro na criacao da tarefa!");
      exit(1);
   }
   
   /* 2a metade da soma para a tarefa principal */
   S= soma(&v[N/2], N/2);
   /* esperar que a tarefa criada termine */
   pthread_join( trfid, (void **) &trfret);
   
   /* calcular soma total a partir das parciais */
   S+= *trfret;
   printf(" Soma= %.0lf\n",S);
   
   printf("A terminar a tarefa principal.\n\n");
   return 0;
}

void *tarefa(void *arg)
{
   trfarg_t *a;
   
   printf("A iniciar a sub-tarefa.\n");
   a= (trfarg_t *) arg;    /* converter (void *) para o tipo certo */
   S1= soma( a->x, a->n);
   printf("A terminar a sub-tarefa.\n");
   return (void*) &S1;  /* retorna endereço de uma variavel nao local com o resultado */
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
