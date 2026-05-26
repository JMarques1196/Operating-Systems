# EFolio B - Operating Systems
**Evaluation:** 25% of the final grade

## SO efB-25: Multithreaded Merge Sort Algorithm

### Problem Description
Write a multithreaded program in standard **C99** and according to the **POSIX** standard, named `mtms.c`. The program must sort a list of integers (values between 0 and 65535) provided in a file, using the original Merge Sort algorithm variant described below.

### Input
The name of the input file to be processed is provided to the program via **standard input** (the first string without spaces).

### Data Processing Flow
The data processing follows the diagram below:

`Input File -> Reader Task -> Sorting Buffers -> Sorting Tasks (nt) -> Merge Buffer -> Merge Task`

1.  **Reader Task (1):** Reads the input file containing an informative header and the data.
2.  **Sorting Buffers:** Buffers composed of pointers to arrays (each containing up to 16 integers).
3.  **Sorting Tasks (nt):** Multiple threads that sort the sub-arrays.
4.  **Merge Buffer:** Buffer that stores sorted sub-arrays.
5.  **Merge Task (1):** Performs the final fusion of all sub-arrays.

### Task Specifications

#### 1. Reader Task
* **Header Parsing:** The file contains lines specifying the number of sorting tasks (`nt`), dimensions for the sorting and merge buffers, and the total number of integers (`n`).
* **Format Example:**
    ```text
    nt=2
    dim_buffer_ordenacao=2
    dim_buffer_fusao=10
    n=12
    # data
    3 21 18 8 4 4 32 17 23 1 5 31
    ```
* **Constraints:** Maximum $n = 20,000$. Maximum 12 integers per line. Duplicate numbers may occur.
* **Behavior:** After loading data into memory, it divides them into sub-arrays of up to 16 numbers and places their addresses in the sorting buffers. It must choose a buffer **randomly**. If the buffer is busy, it must yield the CPU (`sched_yield()`) and retry. It must fill the buffer completely when access is granted.

#### 2. Sorting Tasks
* Must choose a sorting buffer **randomly**.
* If busy, yield the CPU and retry.
* When accessing a buffer, it retrieves **one address at a time**, sorts the respective sub-array, and places it into the **Merge Buffer**.

#### 3. Merge Task
* Maintains an internal array of size $n$, initialized as empty.
* Every time it accesses the Merge Buffer, it merges its internal array with the new sub-array.
* This repeats until the entire input vector is sorted.

### Synchronization & CPU Management
* **Busy Waiting:** Sorting Tasks and the Merge Task must use **busy waiting** to access the Merge Buffer.
* **Yielding:** To yield the CPU, use the `sched_yield()` function:
    ```c
    #include <sched.h>
    int sched_yield(void);
    ```

### Output Report
Upon completion, the main task must print a report to **standard output** following this format:

```text
nt=XXX
n=XXX
T. Leitora:
XXX ints in XXX+XXX buffers
T. Ordenadoras(nt):
T_0: XXX ints in XXX buffers
T_1: XXX ints in XXX buffers
T. Fusão:
XXX ints in XXX merges
# sorted data
1 3 4 4 5 8 17 18 21 23 31 32
