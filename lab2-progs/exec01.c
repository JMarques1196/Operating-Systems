/*
Create a new process using fork() + exec()
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
   pid_t pid, pidw;
   // pid = the result from the fork
   // pidw = return from wait (PID from the finished children)
   int status; // used by wait() to get the status from the children process
   
   printf("\nExemplo de aplicacao 01 das funcoes fork()+exec()\n");
   pid= fork(); // create a children process

   // Error check
   if ( pid==-1 ) {
      perror("Erro na funcao fork()");
      exit(1); // terminates the process
   }
   /* Fork return:
      On parent process: Fork returns a number greater than 0
      On a children process: Fork returns 0
      Error: Fork returns -1

      After fork(), the program is literally being executed twice, but now in two completely separate processes, 
      each with its own memory, own variables, own call stack, and its own execution path.

      Why we get both 0 and >0 returns from 1 fork:
      As soon as we fork we get 2 processes, father and children. So in one process pid will be > 0 (father)
      and in another = 0 (children)

   */
   if ( pid ) {
      /* pid>0, codigo para o processo pai 
         Here pid contains the pid from the created process
      */
      printf("Codigo do Pai  :  PID=%5d  PPID=%5d\n", \
         (int) getpid(), (int) getppid());
      printf("Codigo do Pai  :  Iniciado wait()\n");
      pidw= wait(&status); // Blocks untill the children is finshed executing
      /*
      wait is a system call wrapper provided by the C library (glibc on Linux, libSystem on macOS) that a parent process uses to:
      block until one of its child processes finishes, and
      collect that child’s termination status. 
       */
   }
   else {
      /* pid=0, codigo para o processo filho */
      printf("Codigo do Filho:  Substituir imagem do processo " \
         "pela do comando ls e executar!\n");
      execl("/bin/ls","ls","-al",NULL);// the operating system throws away the entire memory/image of the child process and replaces it with the code of /bin/ls.
      // first argument is the path, second is the name of the program. After execl none of the original program remains
      printf("Se esta mensagem aparecer ocorreu um erro!"); // If the execl suceeds it wont return. It will jump to the
      // first line of the new program so this line will never appear
   }
   
   printf("Codigo do Pai  :  Comando ls -al executado!\n");

   return 0;
}
 /* 
 Diagram
 Parent:
    fork()
        |
        +-- Child:
        |       print("Vou substituir a imagem")
        |       exec("/bin/ls", "ls", "-al")
        |             |
        |             +-- REPLACED by new program "ls"
        |
        +-- Parent:
                print("Iniciado wait()")
                wait()   ← waits for child to finish ls
                print("Comando ls -al executado!")
 
 */