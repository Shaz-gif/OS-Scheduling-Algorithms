/*
 *  ****************************************************
 *  *                                                  *
 *  *        Name : Shashwat Raj                       *
 *  *        Entry Number: 2021MT10259                 *
 *  *                                                  *
 *  ****************************************************
 */


/*
 * Task 2: Online Scheduling
 * 
 * In this task, processes should be taken as input in real-time. After each context switch, the 
 * terminal input must be checked to receive new processes. Any new command will be 
 * deemed a new process if it is newline ('\n') terminated. Refer to online_schedulers.h for 
 * necessary data structures and function declarations.
 * 
 * Implement the following algorithms:
 * 
 * • Multi-level Feedback Queue (MLFQ) with Adaptive Features:
 *   - Initial Priority Assignment: Assign a Medium priority to each process 
 *     when it first arrives.
 *   - Priority Adjustment: Update the process's priority based on the average 
 *     burst time when it reappears. Only consider processes that didn’t end with Error 
 *     as historical data.
 * 
 * • Shortest Job First (SJF):
 *   - Initial Burst Time Assignment: When a process first appears, assign it a 
 *     default burst time of 1 second, as the actual burst time is unknown.
 *   - Historical Data: Update the burst time for a process using historical data 
 *     when it reappears and continue updating it (to average burst time) as more 
 *     data becomes available. Only consider processes that didn’t end with Error 
 *     as historical data.
 */



/* THIS SECTION CONTAINS MICROS, MACROS, DEFINITIONS */

 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <poll.h>
#include <pthread.h>

#define MAX_PROCESSES 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define MAX_ARGUMENTS 100 

uint64_t Arrival_time;

Process processes[MAX_PROCESSES];
int process_count = 0;

ProcessHistory process_history[MAX_PROCESSES];
int history_count = 0;

Process* high_priority[MAX_PROCESSES];
Process* medium_priority[MAX_PROCESSES];
Process* low_priority[MAX_PROCESSES];
int high_count = 0, medium_count = 0, low_count = 0;

/* THIS SECTION CONTAINS STRUCT */


// Struct 1

typedef struct {
    char *full_command;
    char *command;
    char *args[MAX_ARGS];
    int arg_count;
    bool finished;
    bool error;    
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    bool started; 
    int process_id;
    double burst_time;
} Process;


// Struct 2

typedef struct {
    char *full_command;
    double avg_burst_time;
    int execution_count;
} ProcessHistory;

/* THIS SECTION CONTAINS HELPER FUNCTIONS*/


// Function 1

uint64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec * 1000000 + tv.tv_usec);
}

// Function 2

uint64_t current_time(uint64_t start_time)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (long long)time.tv_sec * 1000 + time.tv_usec / 1000 - start_time;
}

// Function 3

void update_process_history(Process *p) {
    if (p->error) return;

    for (int i = 0; i < history_count; i++) {
        if (strcmp(process_history[i].full_command, p->full_command) == 0) {
            double total_time = process_history[i].avg_burst_time * process_history[i].execution_count;
            total_time += (p->completion_time - p->start_time) / 1000000.0;
            process_history[i].execution_count++;
            process_history[i].avg_burst_time = total_time / process_history[i].execution_count;
            return;
        }
    }

    process_history[history_count].full_command = strdup(p->full_command);
    process_history[history_count].avg_burst_time = (p->completion_time - p->start_time) / 1000000.0;
    process_history[history_count].execution_count = 1;
    history_count++;
}


// Function 4

double get_estimated_burst_time(char *full_command) {
    for (int i = 0; i < history_count; i++) {
        if (strcmp(process_history[i].full_command, full_command) == 0) {
            return process_history[i].avg_burst_time;
        }
    }
    return 1.0; // Default burst time
}


// Function 5

int compare_processes(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    
    if (p1->burst_time < p2->burst_time) return -1;
    if (p1->burst_time > p2->burst_time) return 1;
    return 0;
}


// Function 6

void execute_process(Process *p) {
    p->start_time = get_current_time();
    p->started = true;

    pid_t pid = fork();
    if (pid == 0) {  // Child process
        execvp(p->command, p->args);
        fprintf(stderr, "execvp error for command '%s': %s\n", p->full_command, strerror(errno));
        exit(1);
    } else if (pid > 0) {  // Parent process
        int status;
        waitpid(pid, &status, 0);
        p->completion_time = get_current_time();
        p->finished = true;
        p->error = !WIFEXITED(status) || WEXITSTATUS(status) != 0;
        p->turnaround_time = p->completion_time - p->start_time;
        
        printf("%s | %lu | %lu \n", p->full_command, p->start_time, p->completion_time);
        
        update_process_history(p);
    } else {
        perror("fork");
    }
}


// Function 7 

void parse_input(char *input) {
    Process *p = &processes[process_count];
    char *token = strtok(input, " \n");
    int i = 0;

    p->full_command = strdup(input);
    p->full_command[strcspn(p->full_command, "\n")] = 0;  // Remove newline

    while (token != NULL && i < MAX_ARGS) {
        if (i == 0) {
            p->command = strdup(token);
            p->args[i] = p->command;
        } else {
            p->args[i] = strdup(token);
        }
        i++;
        token = strtok(NULL, " \n");
    }
    p->args[i] = NULL;
    p->arg_count = i;
    p->finished = false;
    p->error = false;
    p->started = false;
    p->burst_time = get_estimated_burst_time(p->full_command);

    process_count++;
}


// Function 8 

bool check_for_input() {
    fd_set readfds;
    struct timeval tv = {0, 0};  // Non-blocking

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        char input[MAX_COMMAND_LENGTH];
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (input[0] != '\n') {
                parse_input(input);
                return true;
            }
        }
    }
    return false;
}

// Function 9

uint64_t current_time(uint64_t start_time)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (long long)time.tv_sec * 1000 + time.tv_usec / 1000 - start_time;
}

// Function 10

void update_history(char *command, uint64_t burst_time) {
    for (int i = 0; i < history_count; i++) {
        if (strcmp(process_history[i].command, command) == 0) {
            // Update existing process history
            process_history[i].total_burst_time += burst_time;
            process_history[i].executions += 1;
            return;
        }
    }
    // If command not found in history, add new entry
    process_history[history_count].command = strdup(command);
    process_history[history_count].total_burst_time = burst_time;
    process_history[history_count].executions = 1;
    history_count++;
}

// Function 11

void remove_process_from_queue(Process* queue[], int *count, int index) {
    for (int i = index; i < *count - 1; i++) {
        queue[i] = queue[i + 1];
    }
    (*count)--;
}


// Function 12

uint64_t get_average_burst_time(char *command) {
    for (int i = 0; i < history_count; i++) {
        if (strcmp(process_history[i].command, command) == 0) {
            return process_history[i].total_burst_time / process_history[i].executions;
        }
    }
    // If process not seen before, assume initial burst time of quantum1
    return 0;
}

// Function 13


void assign_priority(Process* process, int quantum0, int quantum1, int quantum2) {
    uint64_t avg_burst_time = get_average_burst_time(process->command);

    if (avg_burst_time == 0) {
        // New process, assign to medium priority
        medium_priority[medium_count++] = process;
        process->burst_time = quantum1;
    } else if (avg_burst_time < quantum0) {
        // High priority
        high_priority[high_count++] = process;
        process->burst_time = quantum0;
    } else if (avg_burst_time < quantum1) {
        // Medium priority
        medium_priority[medium_count++] = process;
        process->burst_time = quantum1;
    } else {
        // Low priority
        low_priority[low_count++] = process;
        process->burst_time = quantum2;
    }
}

// Function 14

void execute_command(Process* process) {
    if (!process->started) {
        process->process_id = fork();
        if (process->process_id == 0) {
            // Child process
            char *args[MAX_ARGUMENTS + 2];  
            int i = 0;
            char *token = strtok(process->command, " ");
            while (token != NULL && i < MAX_ARGUMENTS) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;  
            execv(args[0], args);  
            exit(0);
        }
        process->start_time = current_time(Arrival_time);
        process->started = true;
    } else {
        // Continue if previously stopped
        kill(process->process_id, SIGCONT);
    }
}


// Function 15

void handle_input(char *input, int quantum0, int quantum1, int quantum2) {
    char *input_copy = strdup(input);  
    char *full_command = strdup(input);  

    Process *new_process = (Process *)malloc(sizeof(Process));
    new_process->command = full_command; 
    new_process->finished = false;
    new_process->error = false;
    new_process->started = false;

    // Assign priority based on burst time history
    assign_priority(new_process, quantum0, quantum1, quantum2);
    free(input_copy);
}

// Functoin 16


void *input_thread_func(void *args) {
    int *quantums = (int *)args;
    while (1) {
        char input[1024];
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // Process the input and add the command to the relevant queue
            handle_input(input, quantums[0], quantums[1], quantums[2]);
        }
    }
    return NULL;
}




/* THIS SECTION CONTAINS SCHEDULERS*/


// Scheduler 1

void ShortestJobFirst() {
    while (1) {
        // Check for new input
        check_for_input();

        // Sort processes based on estimated burst time
        qsort(processes, process_count, sizeof(Process), compare_processes);

        bool executed = false;
        for (int i = 0; i < process_count; i++) {
            if (!processes[i].started && !processes[i].finished) {
                execute_process(&processes[i]);
                executed = true;
                break;
            }
        }

        // If no process was executed, check for input again
        if (!executed) {
            usleep(100000);  // Sleep for 100ms to avoid busy-waiting
            continue;
        }

        // Remove finished processes
        int i = 0;
        while (i < process_count) {
            if (processes[i].finished) {
                // Shift remaining processes
                for (int j = i; j < process_count - 1; j++) {
                    processes[j] = processes[j + 1];
                }
                process_count--;
            } else {
                i++;
            }
        }
    }
}


// Scheduler 2


void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2) {

    pthread_t input_thread;
    int quantums[3] = {quantum0, quantum1, quantum2};

    // Create a new thread to handle input
    if (pthread_create(&input_thread, NULL, input_thread_func, (void *)quantums) != 0) {
        perror("Failed to create input handling thread");
        exit(1);
    }

    Arrival_time = current_time(0);
    while (1) {
        
        

        // Always prioritize high-priority processes first
        while (high_count > 0) {
            uint64_t start_time = current_time(Arrival_time);
            Process* p = high_priority[0];
            execute_command(p);
            sleep(quantum0);

            if (waitpid(p->process_id, NULL, WNOHANG) == 0) {
                // Process not finished, demote to medium priority
                kill(p->process_id, SIGSTOP);
                medium_priority[medium_count++] = p;
                remove_process_from_queue(high_priority, &high_count, 0);  // Remove from high priority
                printf("%s demoted to medium priority\n", p->command);
            } else {
                // Process finished
                p->completion_time = current_time(Arrival_time);
                p->turnaround_time = p->completion_time - p->start_time;
                p->finished = true;
                update_history(p->command, quantum0);
                remove_process_from_queue(high_priority, &high_count, 0);
            }   

        uint64_t finish_time = current_time(Arrival_time);
        printf("%s | start-time %li | end-time %li \n", p->command, start_time, finish_time);
        }


        // Next, handle medium-priority processes
        while (medium_count > 0) {
            uint64_t start_time = current_time(Arrival_time);
            Process* p = medium_priority[0];
            execute_command(p);
            sleep(quantum1);

            if (waitpid(p->process_id, NULL, WNOHANG) == 0) {
                // Process not finished, demote to low priority
                kill(p->process_id, SIGSTOP);
                low_priority[low_count++] = p;
                remove_process_from_queue(medium_priority, &medium_count, 0);  // Remove from medium priority
                printf("%s demoted to low priority\n", p->command);
            } else {
                // Process finished
                p->completion_time = current_time(Arrival_time);
                p->turnaround_time = p->completion_time - p->start_time;
                p->finished = true;
            update_history(p->command, quantum1);
                remove_process_from_queue(medium_priority, &medium_count, 0);
            }

            uint64_t finish_time = current_time(Arrival_time);
            printf("%s | start-time %li | end-time %li \n", p->command, start_time, finish_time);
        }


        // Finally, handle low-priority processes
        while (low_count > 0) {
            uint64_t start_time = current_time(Arrival_time);
            Process* p = low_priority[0];
            execute_command(p);
            sleep(quantum2);

            if (waitpid(p->process_id, NULL, WNOHANG) == 0) {
                // Process not finished, rotate to the end of the low priority queue
                kill(p->process_id, SIGSTOP);
                // Move this process to the end of the low priority queue
                remove_process_from_queue(low_priority, &low_count, 0);
                low_priority[low_count++] = p;
                printf("%s continues in low priority, moved to the back of the queue\n", p->command);
            } else {
                // Process finished
                p->completion_time = current_time(Arrival_time);
                p->turnaround_time = p->completion_time - p->start_time;
                p->finished = true;
                update_history(p->command, quantum2);
                remove_process_from_queue(low_priority, &low_count, 0);
            }

            uint64_t finish_time = current_time(Arrival_time);
            printf("%s | start-time %li | end-time %li \n", p->command, start_time, finish_time);
        }

    }
}

