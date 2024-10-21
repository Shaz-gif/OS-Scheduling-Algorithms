# Scheduling Algorithms in C

## Objective
The goal of this assignment is to deepen the understanding of CPU scheduling algorithms through implementation. It involves creating offline and online schedulers using various algorithms.

## Tasks

### Task 1: Offline Scheduling
In offline scheduling, the list of processes will be provided as input before the program starts. Implement the following algorithms in C, as specified in `offline_schedulers.h`:
1. **First-Come, First-Served (FCFS)**
2. **Round Robin (RR)**
3. **Multi-level Feedback Queue (MLFQ)** with three priority levels:
   - Q0: High priority
   - Q1: Medium priority
   - Q2: Low priority

#### Example
For FCFS, given a process array with commands: `run p1`, `run p2`, and `run p3`, the scheduler should execute:
1. `run p1`, followed by a context switch
2. `run p2`, followed by a context switch
3. `run p3`, followed by a context switch

### Task 2: Online Scheduling
In online scheduling, processes are taken as input in real-time. Implement the algorithms in `online_schedulers.h`:
1. **Multi-level Feedback Queue (MLFQ)** with adaptive features:
   - **Initial Priority Assignment**: Assign a medium priority to each process when it first arrives.
   - **Priority Adjustment**: Update a process's priority based on the average burst time using historical data.
2. **Shortest Job First (SJF)**
   - **Initial Burst Time Assignment**: Set the default burst time to 1 second initially.
   - **Historical Data**: Update the burst time using past executions if available.
3. **Shortest Remaining Time First (SRTF)**

#### Example
For SJF, given commands: `p1`, `p2`, `p2`, and `p1`, the scheduler will execute them based on their burst times.

## Output Requirements
- After each context switch, print: `<Command>|<Start Time>|<End Time>`
- Upon process termination, print relevant process details to a CSV file named `result_<type>_<scheduler>.csv` (e.g., `result_offline_RR.csv`).

### CSV File Columns
1. Command
2. Finished (`Yes`/`No`)
3. Error (`Yes`/`No`)
4. Completion Time (milliseconds)
5. Turnaround Time (milliseconds)
6. Waiting Time (milliseconds)
7. Response Time (milliseconds)

## Submission Requirements
Submit a zipped folder named `<Entry_Number>` containing:
- `offline_schedulers.h`
- `online_schedulers.h`

## Evaluation Criteria
1. **Correctness of Implementation**: Algorithms should function as specified.
2. **Code Clarity and Readability**: Code should be well-documented and formatted.
