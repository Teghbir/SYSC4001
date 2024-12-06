    //necessary includes
    #include <stdio.h> 
    #include <stdlib.h>  
    #include <string.h>
    #include "interrupts.h"
    #include <time.h>

    //initiazling the global variable
    int current_time = 0; 

    //global varibale pcb_table is an array of structures of PCB
    struct PCB pcb_table[25];


    //global varibale partitions is an array of structures of Memory_Partition
    struct Memory_Partition partitions[6];

    //file for the status table drawing
    FILE *memory_status;

    int compare_pcb_EP(const void *a, const void *b) {
        const struct PCB *pcbA = (const struct PCB *)a;
        const struct PCB *pcbB = (const struct PCB *)b;

        if (pcbA->total_cpu_time < pcbB->total_cpu_time) return -1;
        if (pcbA->total_cpu_time > pcbB->total_cpu_time) return 1;

        // If equal, compare by pid
        if (pcbA->pid < pcbB->pid) return -1;
        if (pcbA->pid > pcbB->pid) return 1;

        return 0; // Equal
    }

    int compare_pcb(const void *a, const void *b) {
        const struct PCB *pcbA = (const struct PCB *)a;
        const struct PCB *pcbB = (const struct PCB *)b;

        if (pcbA->arrival_time < pcbB->arrival_time) return -1;
        if (pcbA->arrival_time > pcbB->arrival_time) return 1;

        // If equal, compare by pid
        if (pcbA->pid < pcbB->pid) return -1;
        if (pcbA->pid > pcbB->pid) return 1;

        return 0; // Equal
    }


    int readInputFile(const char *filename, struct PCB *pcb_table) {
        FILE *file = fopen(filename, "r");
        //error handling
        if (!file) {
            printf("Cannot open the input file: %s\n", filename);
            return -1;
        }

        int i = 0; // Counter for PCBs
        char line[256];

        // Process each line of the file
        while (fgets(line, sizeof(line), file)) {
            struct PCB pcb;
            // Parse the input line into PCB fields
            if (sscanf(line, "%d, %d, %d, %d, %d, %d",
                    &pcb.pid,
                    &pcb.size,
                    &pcb.arrival_time,
                    &pcb.total_cpu_time,
                    &pcb.io_frequency,
                    &pcb.io_duration) == 6) {
                pcb_table[i++] = pcb;
            } else {
                printf("Malformed line: %s\n", line);
            }
        }

        fclose(file);


        return i; // Return the number of PCBs populated
    }

    //allocates memory in the partitions array
    int allocate_memory(int pid, int size) {
        // Check if the process is already allocated memory
        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == pid) {
                return partitions[i].partition_number; // Return the already allocated partition
            }
        }

        // Find an available partition for the process
        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == -1 && partitions[i].size >= size) {
                partitions[i].occupied_by = pid;
                return partitions[i].partition_number;
            }
        }
        return -1; // No available partition
    }

    // Function to release memory occupied by a process in the partitons array
    void release_memory(int pid) {
        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == pid) {
                partitions[i].occupied_by = -1;
            }
        }
    }

    //handles the output to memory_status.txt
    void handle_system_output(FILE *memory_status) {
        // Calculate memory used, total free, and usable free memory
        int total_free_memory = 0;
        int used_memory = 0;
        int usable_free_memory = 0;

        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == -1) {
                total_free_memory += partitions[i].size;
                usable_free_memory += partitions[i].size;  // Assuming all free memory is usable
            } else {
                used_memory += partitions[i].size;
            }
        }

        // For the correct output formatting
        fprintf(memory_status, "+-----------------------------------------------------------------------------------------------+\n");
        fprintf(memory_status, "| Time of Event | Memory Used | Partitions State       | Total Free Memory | Usable Free Memory |\n");
        fprintf(memory_status, "+-----------------------------------------------------------------------------------------------+\n");

        // Following the format as the headers for the output
        fprintf(memory_status, "| %14d | %10d | ", current_time, used_memory);
        for (int i = 0; i < 6; i++) {
            fprintf(memory_status, "%d", partitions[i].occupied_by);
            if (i < 5) {
                fprintf(memory_status, ", ");
            }
        }
        fprintf(memory_status, " | %17d | %18d |\n", total_free_memory, usable_free_memory);
        fprintf(memory_status, "+-----------------------------------------------------------------------------------------------+\n");
    }


    //The following 3 functions are a simpler implementation of the three algorithms than what the document asks, as the team ran out of time, so it doesn't take the processes out while they are waiting for I/O

    //FCFS implementation
    void handle_FCFS(FILE *output_file, struct PCB pcb_table[], int num_events) {
        qsort(pcb_table, num_events, sizeof(struct PCB), compare_pcb);
        int completion_time[num_events];
    
        fprintf(output_file, "+--------------------------------------------------+\n");
        fprintf(output_file, "| Time of Transition | PID | Old State  | New State  |\n");
        fprintf(output_file, "+--------------------------------------------------+\n");

        for (int i = 0; i < num_events; i++) {
            struct PCB *process = &pcb_table[i];

            // Waits for the processes to actually arrive
            if (current_time < process->arrival_time) {
                current_time = process->arrival_time;
            }

            // Process state transition: New → Running
            fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                    current_time, process->pid, "New", "Running");

            // Allocate memory for the process
            int partition = allocate_memory(process->pid, process->size);
            if (partition == -1) {
                fprintf(output_file, "Process %d cannot be allocated memory.\n", process->pid);
                printf("Process %d cannot be allocated memory.\n", process->pid);
                continue;
            }
            //outputs the change to the memory status file
            handle_system_output(memory_status);

            // Simulate process execution
            current_time += process->total_cpu_time;
            completion_time[i] = current_time;

            // Process state transition: Running → Terminated
            fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                    current_time, process->pid, "Running", "Terminated");

            // Release memory after process completes
            release_memory(process->pid);
        }

        fprintf(output_file, "+--------------------------------------------------+\n");

    }


    //External Priorities Implementation (shortest job first)
    void handle_EP(FILE *output_file, struct PCB pcb_table[], int num_events) {
        qsort(pcb_table, num_events, sizeof(struct PCB), compare_pcb_EP);
        int completion_time[num_events];
       

        fprintf(output_file, "+--------------------------------------------------+\n");
        fprintf(output_file, "| Time of Transition | PID | Old State  | New State  |\n");
        fprintf(output_file, "+--------------------------------------------------+\n");

        for (int i = 0; i < num_events; i++) {
            struct PCB *process = &pcb_table[i];

            // Wait until the process arrives
            if (current_time < process->arrival_time) {
                current_time = process->arrival_time;
            }

            // Process state transition: New → Running
            fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                    current_time, process->pid, "New", "Running");

            // Allocate memory for the process
            int partition = allocate_memory(process->pid, process->size);
            if (partition == -1) {
                fprintf(output_file, "Process %d cannot be allocated memory.\n", process->pid);
                printf("Process %d cannot be allocated memory.\n", process->pid);
                continue;
            }
            handle_system_output(memory_status);

            // Simulate process execution
            current_time += process->total_cpu_time;
            completion_time[i] = current_time;

            // Process state transition: Running → Terminated at the end of execution
            fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                    current_time, process->pid, "Running", "Terminated");

            // Release memory after process completes
            release_memory(process->pid);
        }

        fprintf(output_file, "+--------------------------------------------------+\n");
    }


    //RR implementation
    void handle_RR(FILE *output_file, struct PCB pcb_table[], int num_events) {
        int completion_time[num_events];
        int slice = 15; // Time slice for Round Robin
        int remaining_time[num_events]; // Track remaining CPU time for each process

        // Initialize remaining time for each process
        for (int i = 0; i < num_events; i++) {
            remaining_time[i] = pcb_table[i].total_cpu_time;
        }

        fprintf(output_file, "+--------------------------------------------------+\n");
        fprintf(output_file, "| Time of Transition | PID | Old State  | New State  |\n");
        fprintf(output_file, "+--------------------------------------------------+\n");

        int processes_remaining = num_events;
        while (processes_remaining > 0) {
            for (int i = 0; i < num_events; i++) {
                struct PCB *process = &pcb_table[i];

                // Skip completed processes
                if (remaining_time[i] <= 0) {
                    continue;
                }

                // Wait until the process arrives
                if (current_time < process->arrival_time) {
                    current_time = process->arrival_time;
                }

                // Allocate memory for the process if not already done
                int partition = allocate_memory(process->pid, process->size);
                if (partition == -1) {
                    fprintf(output_file, "Process %d cannot be allocated memory.\n", process->pid);
                    continue;
                }
                handle_system_output(memory_status);

                // Transition to Running and uses a ternary operator
                fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                        current_time, process->pid, 
                        (remaining_time[i] == process->total_cpu_time) ? "New" : "Waiting", "Running");

                // Execute process for the remaining time
                int time_to_execute = (remaining_time[i] > slice) ? slice : remaining_time[i];
                current_time += time_to_execute;
                remaining_time[i] -= time_to_execute;

                // Check completion
                if (remaining_time[i] <= 0) {
                    completion_time[i] = current_time;

                    // Transition to Terminated
                    fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                            current_time, process->pid, "Running", "Terminated");

                    release_memory(process->pid);
                    processes_remaining--;
                } else {
                    // Transition to Waiting
                    fprintf(output_file, "| %18d | %3d | %9s | %10s |\n", 
                            current_time, process->pid, "Running", "Waiting");
                }
            }
        }

        fprintf(output_file, "+--------------------------------------------------+\n");
    }

    
    //main function
    int main(int argc, char *argv[]) {
        //ensures that the user runs the program with atleast 3 files
        if (argc < 2) {
            printf("Usage: %s <input_file> <scheduler> \n", argv[0]);
            return 1;
        }

        //this is to seed the random number generator for different values everytime its run
        srand(time(NULL));


        //gives values to the 6 different paritions
        partitions[0].partition_number = 1;
        partitions[0].size = 40;
        partitions[0].occupied_by = -1;

        partitions[1].partition_number = 2;
        partitions[1].size = 25;
        partitions[1].occupied_by = -1;

        partitions[2].partition_number = 3;
        partitions[2].size = 15;
        partitions[2].occupied_by = -1;

        partitions[3].partition_number = 4;
        partitions[3].size = 10;
        partitions[3].occupied_by = -1;

        partitions[4].partition_number = 5;
        partitions[4].size = 8;
        partitions[4].occupied_by = -1;

        partitions[5].partition_number = 6;
        partitions[5].size = 2;
        partitions[5].occupied_by = -1;


        //initializes an array of 250 events 
        struct PCB trace[250];

        //argv[1] is the name of the input file the user gave
        int num_events = readInputFile(argv[1], trace);
        if (num_events < 0) {
            return 1;
        }

        FILE *output_file = fopen("execution.txt", "w");
        if (!output_file) {
            printf("Cannot open the output file.\n");
            return 1;
        }

        // Open memory status file
        memory_status = fopen("memory_status.txt", "w");
        if (!memory_status) {
            printf("Cannot open the memory status file.\n");
            fclose(output_file);
            return 1;
        }


        // Determine scheduler type and call the respective function
        if (strcmp(argv[2], "FCFS") == 0) {
            handle_FCFS(output_file, trace, num_events);
        } else if (strcmp(argv[2], "EP") == 0) {
            handle_EP(output_file, trace, num_events);
        } else if (strcmp(argv[2], "RR") == 0) {
            handle_RR(output_file, trace, num_events);
        }

        handle_system_output(memory_status);

        //closes the output file
        fclose(output_file);
        fclose(memory_status);

        return 0;
    }
