#include <stdio.h>
#include <stdlib.h>

int main()
{
   char cmd[]= "ps -f";
   /* char cmd[]= "ps -o pid,ppid,user,state,time,comm"; */

   printf("\nExecutando o comando \"%s\"\n",cmd);
   system(cmd); // The system() function is used to invoke an operating system command from a C/C++ program.
   printf("*** Fim ***\n");

   // Output Analysis:
/*    1435 (Parent — VS Code/terminal host)
      ├─ 31435 /bin/bash ...  (TTY ttys001)
      │   └─ 31931 ./cmd01    (TTY ttys001)
      └─ 30892 /bin/bash ...  (TTY ttys002)
         └─ 30914 "fishTask: Interprete ..." (TTY ttys002) 
*/


   return 0;
}
