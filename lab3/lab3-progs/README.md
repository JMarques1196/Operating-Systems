# Important Notes:

## There are 6 basic functions, heavily inter-related, for the creation of tasks:
    * pthread_create()
    * pthread_exit()
    * pthread_join()
    * pthread_detach()
    * pthread_attr_init()
    * pthread_attr_setdetachstate()

A process always as at least one path of execution, or task that is called the main task. It initiates its execution in main(), takes several arguments (argc, argv) and ends at the end of main() or by invoking exit(), returning in both cases a int with a return code.

In a similar way, the main task can use pthread_create() to create new tasks, that begin in designated functions, take a pointer as a single argument and terminate by the end of the respective function or by calling pthread_exit.

## Notes for multithreading:
* main() is always refered to as the main task.
* All the other tasks are simply designated by task.


## Decoupled tasks
* When creating detached threads, the child thread runs independently
* The main thread does not wait for it or collect its result
* When the child finished, it cleans itself up automatically

### Caveat
* If the main thread finishes before the detached thread, the whole program exits and the child thread is killed.

##