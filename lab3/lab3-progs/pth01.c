/*
**  file: pth01.c
This is a single task program. 
It takes a number composed of the multiplication of the first N natural numbers, sums them and prints the result.
It creates a array V with N elements, where N is the product of a given ammount of natural numbers, and initializes it with the values from 1 to N. 
Then it calls the function soma to calculate the sum of all elements in the array and prints the result.
*/

#include <stdio.h>

#define N      1081080   /* N=2*3*4*5*7*9*11*13 */

double soma(double x[], int n); /* funcao que soma os elementos de um vector */ 
 
double v[N],S;                  /* vector de dados e soma total */


int main()
{
   int i;
   
   printf("\nA iniciar a tarefa principal.\n");
   for(i=0; i<N; i++)  /* inicializar vector com numeros naturais 1,..., N */
      v[i]= i+1;
   S= soma(v,N);
   printf(" Soma= %.0lf\n",S);
   printf("A terminar a tarefa principal.\n\n");
   return 0;
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
