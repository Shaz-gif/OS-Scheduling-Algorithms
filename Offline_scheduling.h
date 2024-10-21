/*
 *  ****************************************************
 *  *                                                  *
 *  *        Name : Shashwat Raj                       *
 *  *        Entry Number: 2021MT10259                 *
 *  *                                                  *
 *  ****************************************************
 */

/*
 * Assignment 2: Implementation of Scheduling Algorithms in C
 * 
 * Objective:
 * The purpose of this assignment is to deepen understanding of CPU scheduling algorithms 
 * through implementation.
 * 
 * Tasks:
 * 
 * Task 1: Offline Scheduling
 * The list of processes will be provided as input before the program starts. 
 * Refer to offline_schedulers.h for the necessary data structures and function declarations.
 * 
 * Implement the following CPU scheduling algorithms in C:
 * 1. First-Come, First-Served (FCFS)
 * 2. Round Robin (RR)
 * 3. Multi-level Feedback Queue (MLFQ), with three Queues:
 *    - Q0: High priority
 *    - Q1: Medium priority
 *    - Q2: Low priority
 * 
 */




#pragma once

/*THIS SECTION CONTAINS MACROS, MICROS, DEFINITIONS*/

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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h> 


#define CSV_FILE_NAME1 "result_offline_FCFS.csv"
#define CSV_FILE_NAME2 "result_offline_FCFS.csv"
#define CSV_FILE_NAME3 "result_offline_MLFQ.csv"


/* THIS SECTION CONTIAINS STRUCT */

// Struct 1

typedef struct {

    
    char *command;
    bool finished;  
    bool error;   
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    uint64_t cpu_time;
    bool started; 
    int process_id;

} Process;

// Struct 2

typedef struct Node {
    Process *process;
    struct Node *next;
} Node;

typedef struct Queue {
    Node *front;
    Node *rear;
} Queue;

// Function to create a new queue
Queue* createQueue() {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to add a process to the queue (enqueue)
void enqueue(Queue *queue, Process *process) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->process = process;
    new_node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}


// Function to remove a process from the queue (dequeue)
Process* dequeue(Queue *queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    Node *temp = queue->front;
    Process *process = temp->process;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    return process;
}

// Function to check if a queue is empty
bool isQueueEmpty(Queue *queue) {
    return (queue->front == NULL);
}

// Function to destroy the queue and free memory
void destroyQueue(Queue *queue) {
    while (!isQueueEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

/*THIS SECTION CONATINS HELPER FUNCTIONS*/

// Function 1


// Function to write Header for CSV
void write_csv_header(int fd) {
    const char *header = "Command,Finished,Error,Completion Time (ms),Turnaround Time (ms),Waiting Time (ms),Response Time (ms)\n";
    write(fd, header, strlen(header));
}

// Function 2


//Function to write csv rows
void write_csv_row(int fd, Process *p) {
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "%s,%s,%s,%lu,%lu,%lu,%lu\n",
        p->command,
        p->finished ? "Yes" : "No",
        p->error ? "Yes" : "No",
        p->completion_time,
        p->turnaround_time,
        p->waiting_time,
        p->response_time
    );
    write(fd, buffer, len);
}


// Function 3


// Function to Parse Command
void parse_command(const char *command_str, char **cmd, char ***args) {
    
    char *command_dup = strdup(command_str);
    if (!command_dup) {
        perror("strdup error");
        exit(EXIT_FAILURE);
    }

   
    char *token = strtok(command_dup, " ");
    if (token != NULL) {
        *cmd = strdup(token);
        if (!*cmd) {
            perror("strdup error");
            exit(EXIT_FAILURE);
        }

        
        size_t arg_count = 0;
        
        *args = (char **)malloc(sizeof(char*) * (strlen(command_str) / 2 + 2)); 

        if (!*args) {
            perror("malloc error");
            exit(EXIT_FAILURE);
        }

        (*args)[arg_count++] = *cmd;
        
        while ((token = strtok(NULL, " ")) != NULL) {
            (*args)[arg_count++] = strdup(token);
            if (!(*args)[arg_count - 1]) {
                perror("strdup error");
                exit(EXIT_FAILURE);
            }
        }
        (*args)[arg_count] = NULL;
    }

    free(command_dup);
}

// Function 4


// Function for Precise Time Calculation
void precise_sleep(int quantum_ms) {
    struct timespec req = {0};
    req.tv_sec = quantum_ms / 1000;
    req.tv_nsec = (quantum_ms % 1000) * 1000000L;
    nanosleep(&req, (struct timespec *)NULL);
}

// Function 5


// Function to get Current Time
uint64_t current_time(uint64_t start_time)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (long long)time.tv_sec * 1000 + time.tv_usec / 1000 - start_time;
}


// Function 6 


// Function to simulate RR like behaviuor within each Queue in MLFQ Scheduler
void RR_fasion(Queue *Q, uint64_t quantum, uint64_t Arrival_time, int q_NUM, Queue *Q0, Queue *Q1, Queue *Q2, int  csv_fd ){

    while(!isQueueEmpty(Q)){
        Process *current_process = dequeue(Q);

        if(!current_process->started){

            current_process->process_id = fork();

            if (current_process->process_id == 0) {
                // Child process: Execute the command
                char *cmd = NULL;
                char **args = NULL;
                parse_command(current_process->command, &cmd, &args);  // Parse command and arguments
                execvp(cmd, args);  // Execute the command
                perror("Process Failed ....");  // If execvp fails

                // Free dynamically allocated memory
                free(cmd);
                for (int j = 0; args[j] != NULL; j++) free(args[j]);
                free(args);

                exit(EXIT_FAILURE);

            } else if (current_process->process_id > 0) {
                    // Parent process: Record the start time and mark the process as started
                current_process->start_time = current_time(Arrival_time);
                current_process->response_time = current_process->start_time;
                current_process->started = true;
                current_process->error = false;
                   
            } else {
                    // Fork failed
                perror("fork error");
                exit(EXIT_FAILURE);
            }
        }

        // Already Started

        uint64_t start_time = current_time(Arrival_time);

        // Handle the process execution in the parent process
        int status;
        if (kill(current_process->process_id, SIGCONT) == -1) {
            if (errno != ESRCH) { 
                perror("Error sending SIGCONT");
            } else {
                    // Process has already finished
                current_process->finished = true;
               
                
            }
        }

        precise_sleep(quantum);  // Run for 'quantum' time slice

        if (kill(current_process->process_id, SIGSTOP) == -1 && errno != ESRCH) {
            perror("Error sending SIGSTOP");
        }
        current_process->cpu_time += quantum;
            // Check if the process has completed
        pid_t wait_result = waitpid(current_process->process_id, &status, WNOHANG);


        uint64_t finish_time = current_time(Arrival_time);

        printf("%s | start-Time %li | End-Time %li \n", current_process->command, start_time, finish_time);

        if (wait_result == -1) {
            perror("Error in waitpid");
        } else if (wait_result == 0) {

                
                if(q_NUM==0){
                
                    enqueue(Q1,current_process);
                } else if(q_NUM==1){
                    
                    enqueue(Q2, current_process);
                } else{
                    
                    enqueue(Q2, current_process);
                
                    
                }
            
            
        } else {
                // Process has finished
            
            if (WIFEXITED(status)) {
                    
                current_process->finished = true;
                current_process->completion_time = current_time(Arrival_time) ;
                current_process->turnaround_time = current_process->completion_time;
                current_process->waiting_time = current_process->turnaround_time - current_process->cpu_time;
                current_process->response_time = current_process->start_time;

                    
                printf("%s completed | Start: %lu ms | Completion: %lu ms\n",
                           current_process->command, current_process->start_time, current_process->completion_time);
                write_csv_row(csv_fd, current_process);
            } else if (WIFSIGNALED(status)) {
                    
                current_process->error = true;
                current_process->finished = true;
                current_process->completion_time = current_time(Arrival_time) ;
                current_process->turnaround_time = current_process->completion_time;
                current_process->waiting_time = current_process->turnaround_time - current_process->cpu_time;
                current_process->response_time = current_process->start_time;

                   
                printf("%s error occurred | Start: %lu ms | Completion (Error detection): %lu ms\n",
                           current_process->command, current_process->start_time, current_time(Arrival_time));
                write_csv_row(csv_fd, current_process);
            }
        }
    }


    
}




/*THIS SECTION CONTIANS SCHEDULERS*/

// Scheduler 1 : First-Come-First-Served

void FCFS(Process p[], int n) {

    
    // Opend CSV
    int csv_fd = open(CSV_FILE_NAME1, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (csv_fd < 0) {
        perror("Error opening CSV file");
        return;
    }
    // Write Header in CSV
    write_csv_header(csv_fd);

    uint64_t Arrival_time = current_time(0);  // Arrival Time


    // Loop through each process and run them till completion

    for (int i = 0; i < n; ++i) {

        p[i].start_time = current_time(Arrival_time);
        p[i].started = true;

        
        pid_t pid = fork();
        if (pid == 0) { 
           
            char *cmd;
            char **args;
            parse_command(p[i].command, &cmd, &args);

            
            execvp(cmd, args);
           
            perror("Error executing command");
            exit(EXIT_FAILURE);
        } else if (pid > 0) { 
            
            int status;
            waitpid(pid, &status, 0);

            p[i].completion_time = current_time(Arrival_time);
            p[i].finished = WIFEXITED(status) && WEXITSTATUS(status) == 0;
            p[i].error = !p[i].finished;
            p[i].turnaround_time = p[i].completion_time;
            p[i].waiting_time = p[i].start_time;
            p[i].response_time = p[i].start_time;

            printf("%s|%lu|%lu\n",
                   p[i].command,
                   p[i].start_time,
                   p[i].completion_time);

            write_csv_row(csv_fd, &p[i]);
        } else {
            perror("Error creating child process");
        }
    }

    close(csv_fd);
}



// Scheduler 2 : Round-Robin


void RoundRobin(Process p[], int n, int quantum) {

    uint64_t cpu_time[n];
    for(int i = 0; i < n; i++){
        cpu_time[i] = 0;
    }

    int completed_processes = 0;  // Count of completed processes


    // Open CSV file and write the header
    int csv_fd = open(CSV_FILE_NAME2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (csv_fd == -1) {
        perror("CSV file open error");
        exit(EXIT_FAILURE);
    }
    write_csv_header(csv_fd);

    for(int i = 0; i <n ; i ++){
        p[i].started = false; p[i].finished = false;
    }

    uint64_t Arrival_time = current_time(0);  

    

    // Loop until all processes are completed
    while (completed_processes < n) {
        

        for (int i = 0; i < n; i++) {
            if (p[i].finished) {

               
                continue;
            }

            if (!p[i].started) {
                
                p[i].process_id = fork();
                if (p[i].process_id == 0) {
                    // Child process: Execute the command
                    char *cmd = NULL;
                    char **args = NULL;
                    parse_command(p[i].command, &cmd, &args);  
                    execvp(cmd, args);  
                    perror("Process Failed ....");  // If execvp fails

                    // Free dynamically allocated memory
                    free(cmd);
                    for (int j = 0; args[j] != NULL; j++) free(args[j]);
                    free(args);

                    exit(EXIT_FAILURE);

                } else if (p[i].process_id > 0) {

                    // Parent process: Record the start time and mark the process as started
                    p[i].start_time = current_time(0) - Arrival_time;
                    p[i].response_time = p[i].start_time;
                    p[i].started = true;
                    p[i].error = false;

                    
                } else {

                    // Fork failed
                    perror("fork error");
                    exit(EXIT_FAILURE);
                }
            }

            // Handle the process execution in the parent process

            uint64_t start_time = current_time(Arrival_time);
            int status;
            if (kill(p[i].process_id, SIGCONT) == -1) {
                if (errno != ESRCH) {  // ESRCH means the process doesn't exist
                    perror("Error sending SIGCONT");
                } else {

                    // Process has already finished
                    p[i].finished = true;
                    completed_processes++;
                    continue;
                }
            }

            precise_sleep(quantum);  // Run for 'quantum' time slice

            if (kill(p[i].process_id, SIGSTOP) == -1 && errno != ESRCH) {
                perror("Error sending SIGSTOP");
            }
            cpu_time[i] += quantum;

            // Check if the process has completed

            pid_t wait_result = waitpid(p[i].process_id, &status, WNOHANG);

            printf("%s | Start-Time %li | End-Time %li \n", p[i].command , start_time, current_time(Arrival_time));

            if (wait_result == -1) {
                perror("Error in waitpid");
            } else if (wait_result == 0) {

                // Process is still running
                
                continue;
            } else {

                // Process has finished
                completed_processes++;
                if (WIFEXITED(status)) {
                    
                    p[i].finished = true;
                    p[i].completion_time = current_time(0) - Arrival_time;
                    p[i].turnaround_time = p[i].completion_time;
                    p[i].waiting_time = p[i].turnaround_time - cpu_time[i];
                    p[i].response_time = p[i].start_time;

                    
                    printf("%s completed | Start: %lu ms | Completion: %lu ms\n",
                           p[i].command, p[i].start_time, p[i].completion_time);
                    write_csv_row(csv_fd, &p[i]);
                } else if (WIFSIGNALED(status)) {
                    
                    p[i].error = true;
                    p[i].finished = true;

                   
                    printf("%s error occurred | Start: %lu ms | Current: %lu ms\n",
                           p[i].command, p[i].start_time, current_time(Arrival_time));
                    write_csv_row(csv_fd, &p[i]);
                }
            }
        }

    
    }

    if (close(csv_fd) == -1) {
        perror("Error closing CSV file");
      
    }
}



// Scheduler 3 : Multi-Level-Feedback-Queue



void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime) {

    int csv_fd = open(CSV_FILE_NAME3, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (csv_fd == -1) {
        perror("CSV file open error");
        exit(EXIT_FAILURE);
    }
    write_csv_header(csv_fd);

  
    // Create the three priority queues (Q0: high, Q1: medium, Q2: low)
    Queue *Q0 = createQueue();
    Queue *Q1 = createQueue();
    Queue *Q2 = createQueue();



    for (int i = 0; i < n; i++) {
        p[i].started = false;
        p[i].finished = false;
        p[i].cpu_time = 0;
    }
    
    
    // Enqueue all processes into the highest priority queue (Q0)
    for (int i = 0; i < n; i++) {
        enqueue(Q0, &p[i]);  
    }
    
    // Time variables for tracking execution
    uint64_t Arrival_time = current_time(0);
    uint64_t current_t = current_time(Arrival_time);
    uint64_t Time_laps = 0;
    uint64_t prev_t = current_t;
    

    while (!isQueueEmpty(Q0) || !isQueueEmpty(Q1) || !isQueueEmpty(Q2)) {

        

        Process *current_process = NULL;
        int quantum = 0;

        // Check the highest priority queue that is not empty and set the quantum accordingly
     
        if (!isQueueEmpty(Q0)) {

        

            quantum = quantum0;

            RR_fasion(Q0, quantum, Arrival_time, 0,Q0,Q1,Q2, csv_fd);


        } else if (!isQueueEmpty(Q1)) {

      

            quantum = quantum1;

            RR_fasion(Q1, quantum, Arrival_time, 1,Q0,Q1,Q2, csv_fd);

        } else if (!isQueueEmpty(Q2)) {

        

            quantum = quantum2;

            RR_fasion(Q2, quantum, Arrival_time, 2,Q0,Q1,Q2, csv_fd);
        } 

        // Boosting
        prev_t = current_t;
        current_t = current_time(Arrival_time);
        Time_laps =   current_t - prev_t;
        

        if(Time_laps>= boostTime){
            printf("Boasting ... \n");
            int x = 0;
            while(!isQueueEmpty(Q1)){
                x++;
                Process *P = dequeue(Q1);
                enqueue(Q0, P);
            }
            while(!isQueueEmpty(Q2)){
                x++;
                Process *P = dequeue(Q2);
                enqueue(Q0, P);
            }
            if(x==0){
                printf("Nothing to Boost \n");
            }

            Time_laps -= boostTime;
            printf("Boosting Ended \n");
        }



        
    }

    // Cleanup: free the queues
    destroyQueue(Q0);
    destroyQueue(Q1);
    destroyQueue(Q2);
}
