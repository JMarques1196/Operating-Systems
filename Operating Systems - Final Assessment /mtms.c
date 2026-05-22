#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

// 1. Structural Definitions
typedef struct
{
    int *array;
    int size;
    volatile int occupied; // Slot status: 0 = Free, 1 = Ready, 2 = Reserved
} Slot;

typedef struct
{
    char file_name[100];
    int nt;              // Number of sorting threads
    int sort_buffer_dim; // Sorting buffer capacity per thread
    int n;               // Total integers to read from file
    int *data_array;     // Raw input data container array
    Slot *sort_buffer;   // Pointer to sorting buffer slots
    int ints_read;       // Tracks total integers parsed
    int buffers_filled;  // Tracks total sorting blocks populated
} ReaderArgs;

typedef struct
{
    Slot *sort_buffer;    // Pointer to sorting buffer array
    Slot *merge_buffer;   // Pointer to merge buffer array
    int nt;               // Count of sorting threads running
    int sort_buffer_dim;  // Sorting buffer array size
    int merge_buffer_dim; // Merge buffer array size
    int buffers_sorted;   // Blocks sorted by this thread
    int ints_sorted;      // Total numbers sorted by this thread
} SorterArgs;

typedef struct
{
    Slot *merge_buffer;   // Pointer to merge buffer array
    int merge_buffer_dim; // Merge buffer array size
    int *final_array;     // Target array holding full ordered output
    int n_total;          // Grand total of expected integers
    int n_current;        // Current accumulated count of merged elements
    int merge_operations; // Total execution count of merge operations
} MergeArgs;

// 2. Function Prototypes
void *reader_task(void *arg);
void *sorter_task(void *arg);
void *merge_task(void *arg);
int all_slots_empty(Slot *buffer, int size);
int compare(const void *a, const void *b);
void merge_blocks(int *destination, int n_current, int *new_block, int n_new);

// Global State & Synchronization
int reading_completed = 0;                               // Set to 1 when reader thread finishes parsing input file
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER; // Global lock for buffer operations

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <data_file>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    // 1. Parse File Header Configurations
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror("Error opening file");
        return 1;
    }

    int nt, d_sort, d_merge, n;
    fscanf(f, "nt=%d\n", &nt);
    fscanf(f, "dim_buffer_ord=%d\n", &d_sort);
    fscanf(f, "dim_buffer_fus=%d\n", &d_merge);
    fscanf(f, "n=%d\n", &n);
    fclose(f);

    // 2. Memory Allocation
    int *data_array = malloc(n * sizeof(int));
    int *final_array = malloc(n * sizeof(int));
    Slot *sort_buffer = malloc(nt * d_sort * sizeof(Slot));
    Slot *merge_buffer = malloc(d_merge * sizeof(Slot));

    // Initialize all thread-communication slots to Free
    for (int i = 0; i < nt * d_sort; i++)
        sort_buffer[i].occupied = 0;
    for (int i = 0; i < d_merge; i++)
        merge_buffer[i].occupied = 0;

    // 3. Thread Arguments Setup
    ReaderArgs args_read;
    strcpy(args_read.file_name, argv[1]);
    args_read.nt = nt;
    args_read.sort_buffer_dim = d_sort;
    args_read.n = n;
    args_read.data_array = data_array;
    args_read.sort_buffer = sort_buffer;
    args_read.ints_read = 0;
    args_read.buffers_filled = 0;

    MergeArgs args_merge;
    args_merge.merge_buffer = merge_buffer;
    args_merge.merge_buffer_dim = d_merge;
    args_merge.final_array = final_array;
    args_merge.n_total = n;
    args_merge.n_current = 0;
    args_merge.merge_operations = 0;

    SorterArgs args_sort[nt];
    for (int i = 0; i < nt; i++)
    {
        args_sort[i].sort_buffer = sort_buffer;
        args_sort[i].merge_buffer = merge_buffer;
        args_sort[i].nt = nt;
        args_sort[i].sort_buffer_dim = d_sort;
        args_sort[i].merge_buffer_dim = d_merge;
        args_sort[i].buffers_sorted = 0;
        args_sort[i].ints_sorted = 0;
    }

    // 4. Thread Dispatching Pipeline
    pthread_t th_read, th_merge, th_sort[nt];
    pthread_create(&th_read, NULL, reader_task, &args_read);
    pthread_create(&th_merge, NULL, merge_task, &args_merge);
    for (int i = 0; i < nt; i++)
        pthread_create(&th_sort[i], NULL, sorter_task, &args_sort[i]);

    // 5. Thread Synchronization Barrier (Join)
    pthread_join(th_read, NULL);
    for (int i = 0; i < nt; i++)
        pthread_join(th_sort[i], NULL);
    pthread_join(th_merge, NULL);

    // 6. Output Final Performance Metrics Report
    printf("nt=%d\nn=%d\n", nt, n);
    printf("Reader Thread:\n%d ints in %d buffers\n", args_read.ints_read, args_read.buffers_filled);
    printf("Sorter Threads(%d):\n", nt);
    for (int i = 0; i < nt; i++)
        printf("T_%d: %d ints in %d buffers\n", i, args_sort[i].ints_sorted, args_sort[i].buffers_sorted);
    printf("Merge Thread:\n%d ints in %d merges\n", args_merge.n_current, args_merge.merge_operations);

    // 7. Result Presentation & File Generation
    FILE *fs = fopen("output.txt", "w");
    printf("# sorted data\n");
    if (fs)
        fprintf(fs, "# sorted data\n");

    int total_lines = (n + 11) / 12;
    for (int i = 0; i < n; i++)
    {
        int current_line = i / 12;
        // Truncation rules logic: print ellipsis if dataset contains more than 120 elements
        if (n > 120 && current_line >= 5 && current_line < total_lines - 5)
        {
            if (i % 12 == 0)
                printf("...\n");
            i = (total_lines - 5) * 12 - 1; // Instantly advance loop index to the remaining 5 rows
            continue;
        }

        printf("%d%s", final_array[i], ((i + 1) % 12 == 0 || i == n - 1) ? "" : " ");
        if (fs)
            fprintf(fs, "%d%s", final_array[i], ((i + 1) % 12 == 0 || i == n - 1) ? "" : " ");

        if ((i + 1) % 12 == 0 || i == n - 1)
        {
            printf("\n");
            if (fs)
                fprintf(fs, "\n");
        }
    }
    if (fs)
        fclose(fs);

    // Clean up all allocated memory blocks and destroy synchronization primitives
    free(data_array);
    free(final_array);
    free(sort_buffer);
    free(merge_buffer);
    pthread_mutex_destroy(&buffer_lock);
    return 0;
}

// --- Task Implementations ---

// Reader Worker: Loads inputs into memory, parcels data into small segments, and transfers them into sort_buffer
void *reader_task(void *arg)
{
    ReaderArgs *a = (ReaderArgs *)arg;
    FILE *fp = fopen(a->file_name, "r");
    char garbage[100];

    // Skip configuration header labels
    for (int i = 0; i < 6; i++)
        fscanf(fp, "%s", garbage);

    // Bulk load raw sequential values into main data array block
    for (int i = 0; i < a->n; i++)
    {
        fscanf(fp, "%d", &(a->data_array[i]));
        a->ints_read++;
    }
    fclose(fp);

    int position = 0;
    while (position < a->n)
    {
        int size = (a->n - position >= 16) ? 16 : (a->n - position);
        int r = rand() % (a->nt * a->sort_buffer_dim);

        pthread_mutex_lock(&buffer_lock); // Safely secure mutual exclusion lock before reading slot status

        if (a->sort_buffer[r].occupied == 0)
        {
            a->sort_buffer[r].array = &(a->data_array[position]);
            a->sort_buffer[r].size = size;
            a->sort_buffer[r].occupied = 1; // Dispatch status: ready for sorting
            position += size;
            a->buffers_filled++;

            pthread_mutex_unlock(&buffer_lock); // Release lock post data allocation
        }
        else
        {
            pthread_mutex_unlock(&buffer_lock); // Release lock if randomly targeted slot is occupied
            sched_yield();                      // Yield CPU control to give sorter threads execution priority
        }
    }
    reading_completed = 1;
    return NULL;
}

// Sorter Worker: Hunts for populated slots inside sorting buffer, handles qsort, and loads them into merge_buffer
void *sorter_task(void *arg)
{
    SorterArgs *a = (SorterArgs *)arg;
    int total = a->nt * a->sort_buffer_dim;

    while (1)
    {
        int r = rand() % total;
        int found = 0;

        pthread_mutex_lock(&buffer_lock);
        if (a->sort_buffer[r].occupied == 1)
        {
            a->sort_buffer[r].occupied = 2; // Lock/Reserve current slot from other competing sorter threads
            found = 1;
        }
        pthread_mutex_unlock(&buffer_lock);

        if (found)
        {
            // Compute heavy quicksort routines out-of-lock to permit concurrent thread scheduling
            qsort(a->sort_buffer[r].array, a->sort_buffer[r].size, sizeof(int), compare);

            int delivered = 0;
            while (!delivered)
            {
                int f = rand() % a->merge_buffer_dim;

                pthread_mutex_lock(&buffer_lock);
                if (a->merge_buffer[f].occupied == 0)
                {
                    a->merge_buffer[f].array = a->sort_buffer[r].array;
                    a->merge_buffer[f].size = a->sort_buffer[r].size;

                    a->merge_buffer[f].occupied = 1; // Dispatch status: ready for final merging
                    a->sort_buffer[r].occupied = 0;  // Release input slot state back to free
                    delivered = 1;

                    a->buffers_sorted++;
                    a->ints_sorted += a->merge_buffer[f].size;
                }
                pthread_mutex_unlock(&buffer_lock);
                if (!delivered)
                    sched_yield();
            }
        }
        else
        {
            // Thread teardown condition check
            pthread_mutex_lock(&buffer_lock);
            if (reading_completed && all_slots_empty(a->sort_buffer, total))
            {
                pthread_mutex_unlock(&buffer_lock);
                break;
            }
            pthread_mutex_unlock(&buffer_lock);
            sched_yield();
        }
    }
    return NULL;
}

// Merger Worker: Pulls sorted data slices out of merge buffer and sequences them linearly inside the master array
void *merge_task(void *arg)
{
    MergeArgs *a = (MergeArgs *)arg;
    while (a->n_current < a->n_total)
    {
        int f = rand() % a->merge_buffer_dim;

        pthread_mutex_lock(&buffer_lock);
        if (a->merge_buffer[f].occupied == 1)
        {
            // Perform sub-array sequential merge inside lock parameters to safely preserve final output integrity
            merge_blocks(a->final_array, a->n_current, a->merge_buffer[f].array, a->merge_buffer[f].size);
            a->n_current += a->merge_buffer[f].size;
            a->merge_operations++;

            a->merge_buffer[f].occupied = 0; // Clear slot back to open/free status
        }
        pthread_mutex_unlock(&buffer_lock);

        sched_yield();
    }
    return NULL;
}

// --- Helper Functions ---

// Inspects selected array buffer to confirm all data flags are clear
int all_slots_empty(Slot *buffer, int size)
{
    for (int i = 0; i < size; i++)
        if (buffer[i].occupied == 1)
            return 0;
    return 1;
}

// Numeric evaluation function passed to native standard library qsort call
int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

// Linear two-pointer array sorting method combining a separate sorted array block into target destination vector
void merge_blocks(int *destination, int n_current, int *new_block, int n_new)
{
    int *temp = malloc((n_current + n_new) * sizeof(int));
    int i = 0, j = 0, k = 0;
    while (i < n_current && j < n_new)
    {
        if (destination[i] <= new_block[j])
            temp[k++] = destination[i++];
        else
            temp[k++] = new_block[j++];
    }
    while (i < n_current)
        temp[k++] = destination[i++];
    while (j < n_new)
        temp[k++] = new_block[j++];
    memcpy(destination, temp, (n_current + n_new) * sizeof(int));
    free(temp);
}