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

    //global variable pcb_table_elements 
    int pcb_table_elements = 0; 


    //global varibale partitions is an array of structures of Memory_Partition
    struct Memory_Partition partitions[6];

    //file for the status table drawing
    FILE *memory_status;

    int compare_pcb(const void *a, const void *b) {
        struct PCB *pcbA = (struct PCB *)a;
        struct PCB *pcbB = (struct PCB *)b;

        // Compare by arrival_time first
        if (pcbA->arrival_time != pcbB->arrival_time) {
            return pcbA->arrival_time - pcbB->arrival_time;
        }

        // If arrival_time is the same, compare by pid
        return pcbA->pid - pcbB->pid;
    }


    int readInputFile(const char *filename, struct PCB *pcb_table) {
        FILE *file = fopen(filename, "r");
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

        // Sort the PCB array based on arrival_time and pid
        qsort(pcb_table, i, sizeof(struct PCB), compare_pcb);

        return i; // Return the number of PCBs populated
    }


    int allocate_memory(int pid, int size) {
        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == -1 && partitions[i].size >= size) {
                partitions[i].occupied_by = pid;
                return partitions[i].partition_number;
            }
        }
        return -1; // No available partition
    }

    // Function to release memory occupied by a process
    void release_memory(int pid) {
        for (int i = 0; i < 6; i++) {
            if (partitions[i].occupied_by == pid) {
                partitions[i].occupied_by = -1;
            }
        }
    }

    void handle_FCFS(FILE *output_file) {
        fprintf(output_file, "FCFS");
        int completed_processes = 0;
        while (completed_processes < pcb_table_elements) {
            for (int i = 0; i < pcb_table_elements; i++) {
                struct PCB *pcb = &pcb_table[i];
                if (pcb->arrival_time == current_time) {
                    continue;
                }
                strcpy(pcb->state, "READY");
                int partition_index = allocate_memory(pcb->pid, pcb->size);
                if (partition_index == -1) {
                        // No memory available; keep in READY state
                        continue;
                } else{
                    pcb->partition_number == partition_index;
                    partitions[partition_index].partition_number = pcb ->pid;
                }
            }
            for (int i = 0; i < pcb_table_elements; i++) {
                struct PCB *pcb = &pcb_table[i];
                if (strcmp(pcb->state, "READY")) {
                    strcmp(pcb->state, "RUNNING");
                    current_time+=pcb->io_frequency;
                    strcpy(pcb->state, "WAITING");
                    pcb->total_cpu_time-=pcb->io_frequency;
                }
            }
            current_time++;
        }
    }

    void handle_EP(FILE *output_file){
        fprintf(output_file, "EP");

    }

    void handle_system_output(FILE *memory_status) {
        fprintf(memory_status, "+------------------------------------------------------------------------------------------+\n");
        fprintf(memory_status, "| Time of Event | Memory Used | Partitions State | Total Free Memory | Usable Free Memory |");
        fprintf(memory_status, "+------------------------------------------------------------------------------------------+\n");
            for (int i = 0; i < 6; i++) {
                fprintf(memory_status, "%d", partitions[i].occupied_by);
                if (i < 5) {
                    fprintf(memory_status,", ");
            }
    }


        
    }
    
    //main function
    int main(int argc, char *argv[]) {
        //ensures that the user runs the program with atleast 3 files
        if (argc < 3) {
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

        handle_system_output(memory_status);

        // Determine scheduler type and call the respective function
        if (strcmp(argv[2], "FCFS") == 0) {
            handle_FCFS(output_file);
        } if (strcmp(argv[2], "EP") == 0) {
            handle_EP(output_file);
        }

        //closes the output file
        fclose(output_file);
        return 0;
    }
