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

//initiliazes global variables to track exec calls and fork calls
int exec_call = 1;
int fork_call = 0;

//file for the status table drawing
FILE *system_status;

////global varibale tracker is an array of structures of External_Files
struct External_Files tracker[10];

//reads the trace file, extracts the events from it and stores in the array of Event structures
int readInputFile(const char *filename, struct Event *trace) {
    FILE *file = fopen(filename, "r");
    //error handling
    if (!file) {
        printf("Cannot open the needed file: %s\n", filename);
        return -1;
    }
    //go over each line until no more lines
    int i = 0;
    char line[256];
    //goes over each line until the end and tackles all the possible commands for this assingment
    //added an additional variable to the Event struct for the file name with exec
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SYSCALL", 7) == 0) {
            sscanf(line, "%s %d, %d", trace[i].type, &trace[i].event_number, &trace[i].duration);
            strcpy(trace[i].file_name, "");
        } else if (strncmp(line, "CPU", 3) == 0) {
            sscanf(line, "%[^,], %d", trace[i].type, &trace[i].duration);
            trace[i].event_number = -1; 
            strcpy(trace[i].file_name, "");
        } else if (strncmp(line, "FORK", 4) == 0) {
            sscanf(line, "%[^,], %d", trace[i].type, &trace[i].duration);
            trace[i].event_number = -1;  // FORK doesn't have an event number
            strcpy(trace[i].file_name, "");
        } 
        else if (strncmp(line, "EXEC", 4) == 0) {
            sscanf(line, "%s %[^,], %d", trace[i].type, trace[i].file_name, &trace[i].duration);
            trace[i].event_number = -1; 
        }
        i++;
    }
    //close the file and return the number of events
    fclose(file);
    return i;
}

//reads the external file, extracts the events from it and stores in the array of external files structure
int externalFilesInfo(const char *filename, struct External_Files *tracker) {
    FILE *file = fopen(filename, "r");
    //error handling
    if (!file) {
        printf("Cannot open the needed file: %s\n", filename);
        return -1;
    }
    //go over each line until no more lines
    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n') {
            continue; // Skip empty lines
        }
        // Use sscanf to extract program name and size
        sscanf(line, "%[^,], %d", tracker[i].program_name, &tracker[i].program_size);
        i++; // Increment the count of programs
    }
    //close the file and return the number of events
    fclose(file);
    return i;
}

//to handle when the command is CPU
void handle_cpu(FILE *output_file, int duration) {
    fprintf(output_file, "%d, %d, CPU execution\n", current_time, duration);
    current_time += duration;
}

//to handle the command SYSCALL
void handle_syscall(FILE *output_file, int event_number, int duration) {
    fprintf(output_file, "%d, %d, switch to kernel mode\n", current_time, 1);
    current_time += 1;

    //save/restore context gets a random value between 1-3ms
    int num = rand() % 3 + 1;
    fprintf(output_file, "%d, %d, context saved\n", current_time, num);
    current_time += num;

    fprintf(output_file, "%d, %d, find vector %d in memory position 0x%04hhx\n", current_time, 1, event_number, event_number * 2);
    current_time += 1;
    fprintf(output_file, "%d, %d, load address 0x0%X into the PC\n", current_time, 1, vector_table[event_number]);
    current_time += 1;

    //The ISR steps get random values that add upto the total duration specified
    int num1 = rand() % (duration -1);
    int num2 = rand() % (duration -num1);
    int num3 = duration - num1 - num2;

    //ISR body and it uses the randomly generated times
    fprintf(output_file, "%d, %d, SYSCALL: run the ISR\n", current_time, num1);
    current_time += num1;
    fprintf(output_file, "%d, %d, transfer data\n", current_time, num2);
    current_time += num2;
    fprintf(output_file, "%d, %d, check for errors\n", current_time, num3);
    current_time += num3;

    fprintf(output_file, "%d, %d, IRET\n", current_time, 1);
    current_time += 1;

}

//to handle when the command is FORK
void handle_fork(FILE *output_file, int event_number, int duration) {
    fork_call++;
    fprintf(output_file, "%d, %d, switch to kernel mode\n", current_time, 1);
    current_time += 1;

    //save/restore context gets a random value between 1-3ms
    int num = rand() % 3 + 1;
    fprintf(output_file, "%d, %d, context saved\n", current_time, num);
    current_time += num;

    fprintf(output_file, "%d, %d, find vector 2 in memory position 0x0004\n", current_time, 1);
    current_time += 1;

    fprintf(output_file, "%d, %d, load address 0X0695 into the PC\n", current_time, 1);
    current_time += 1;

    int index = pcb_table_elements - 1; // Start from the last element

    //this block of code handles the init programs
    
    // Keep going back in the table until you find an entry with the program name "init"
    while (index >= 0 && strcmp(pcb_table[index].program_name, "init") != 0) {
        index--; // Move backward
    }

    if (index >= 0) { 
        pcb_table[fork_call].pid = pcb_table[pcb_table_elements - 1].pid + 1;

        pcb_table[fork_call].memory_partition = pcb_table[index].memory_partition;

        strcpy(pcb_table[fork_call].program_name, pcb_table[index].program_name);

        pcb_table[fork_call].size = pcb_table[index].size;
    }

    // Random time for the first step
    int num1 = rand() % (duration - 1); 
    // Remaining time for the second step
    int num2 = duration - num1;         

    // FORK steps with the two randomly generated times
    fprintf(output_file, "%d, %d, FORK: copy parent PCB to child PCB\n", current_time, num1);
    current_time += num1;

    fprintf(output_file, "%d, %d, scheduler called\n", current_time, num2);
    current_time += num2;

    //Output the system status table
    handle_system_output(system_status);

    fprintf(output_file, "%d, %d, IRET\n", current_time, 1);
    current_time += 1;   

    //increments the number of elements in the pcb table
    pcb_table_elements++;
}


//to handle when the command is EXEC
void handle_exec(FILE *output_file, char *file_name, int duration) {

    fprintf(output_file, "%d, %d, switch to kernel mode\n", current_time, 1);
    current_time += 1;

    //save/restore context gets a random value between 1-3ms
    int num = rand() % 3 + 1;
    fprintf(output_file, "%d, %d, context saved\n", current_time, num);
    current_time += num;

    fprintf(output_file, "%d, %d, find vector 3 in memory position 0x0006\n", current_time, 1);
    current_time += 1;

    fprintf(output_file, "%d, %d, load address 0X042B into the PC\n", current_time, 1);
    current_time += 1; 

    // Generates 5 random values that add up to the duration
    int num1 = rand() % (duration - 4);         
    int num2 = rand() % (duration - num1 - 3);   
    int num3 = rand() % (duration - num1 - num2 - 2); 
    int num4 = rand() % (duration - num1 - num2 - num3 - 1); 
    int num5 = duration - num1 - num2 - num3 - num4; 

    fprintf(output_file, "%d, %d, EXEC: load %s of size %dmb\n", current_time, num1, file_name, tracker[exec_call-1].program_size); 
    current_time += num1;

    int partition;

    for (int i = 0; i < 6; i++) {

        //loops over the available parititons and finds an appropriate slot and marks it as occupied
        if (tracker[exec_call-1].program_size==partitions[i].size){
            fprintf(output_file, "%d, %d, found partition %d with %d of space\n", current_time, num2, i+1, partitions[i].size);
            partition = i+1;
            strcpy(partitions[i].code, file_name);
            current_time += num2;

            fprintf(output_file, "%d, %d, partition %d marked as occupied\n", current_time, num3, i+1);
            current_time += num3;
        }
    }

    fprintf(output_file, "%d, %d, Updating PCB with new information", current_time, num4, file_name); 
    current_time += num4;

    //this block updates the pcb table with the exec instructions

    //if the last entry in the table was init, then it gets replaced
    if (strcpy(pcb_table[pcb_table_elements-1].program_name, "init")){
        strcpy(pcb_table[pcb_table_elements-1].program_name, file_name); 
        pcb_table[pcb_table_elements-1].size = tracker[exec_call-1].program_size;
        pcb_table[pcb_table_elements-1].memory_partition = partition;

    } else{
        //if the last entry wasn't init then a new entry gets created
        strcpy(pcb_table[pcb_table_elements].program_name, file_name);
        pcb_table[pcb_table_elements].size = tracker[exec_call - 1].program_size;
        pcb_table[pcb_table_elements-1].memory_partition = partition;
    }
    
    fprintf(output_file, "\n%d, %d, scheduler called\n", current_time, num5);
    current_time += num5;

    //outputs the system status
    handle_system_output(system_status);

    fprintf(output_file, "%d, %d, IRET\n", current_time, 1);
    current_time += 1;

    //this block of code handles the instructions inside the program file mentioned in the trace file
    //it dynamically gets the program number, it isn't hardcoded
    struct Event trace[250];
    char program[15];
    sprintf(program, "program%d.txt", exec_call);
    readInputFile(program, trace);
    for (int i = 0; i < 10; i++) {
        if (strcmp(trace[i].type, "CPU") == 0) {
            handle_cpu(output_file, trace[i].duration);
        } else if (strcmp(trace[i].type, "SYSCALL") == 0) {
            handle_syscall(output_file, trace[i].event_number, trace[i].duration);
        }
    }

    // increments the number of exec calls
    exec_call++;
}

//Called before each IRET instruction
void handle_system_output(FILE *system_status) {
    fprintf(system_status, "!-----------------------------------------------------------!\n");
    fprintf(system_status, "Save Time: %d ms\n", current_time);
    fprintf(system_status, "+--------------------------------------------+\n");
    fprintf(system_status, "| PID | Program Name | Partition Number | Size |\n");
    fprintf(system_status, "+--------------------------------------------+\n");

    // Loop through each PCB entry and print the formatted row
    for (int i = 0; i < 6; i++) {
        if (pcb_table[i].pid != 0) {  // Only print non-empty entries
            //this is to handle the rows and alignment
            fprintf(system_status, "| %2d  | %12s | %15d | %4d |\n", 
                    pcb_table[i].pid,
                    pcb_table[i].program_name,
                    pcb_table[i].memory_partition,
                    pcb_table[i].size);
        }
    }

    fprintf(system_status, "+--------------------------------------------+\n");
    fprintf(system_status, "!-----------------------------------------------------------!\n");
}

//main function
int main(int argc, char *argv[]) {
    //ensures that the user runs the program with atleast 3 files
    if (argc < 4) {
        printf("Usage: %s <input_file> <output_file> <external_files> \n", argv[0]);
        return 1;
    }

    //this is to seed the random number generator for different values everytime its run
    srand(time(NULL));


    //gives values to the 6 different paritions
    partitions[0].partition_number = 1;
    partitions[0].size = 40;
    strcpy(partitions[0].code, "free");

    partitions[1].partition_number = 2;
    partitions[1].size = 25;
    strcpy(partitions[1].code, "free");

    partitions[2].partition_number = 3;
    partitions[2].size = 15;
    strcpy(partitions[2].code, "free");

    partitions[3].partition_number = 4;
    partitions[3].size = 10;
    strcpy(partitions[3].code, "free");

    partitions[4].partition_number = 5;
    partitions[4].size = 8;
    strcpy(partitions[4].code, "free");

    partitions[5].partition_number = 6;
    partitions[5].size = 2;
    strcpy(partitions[5].code, "free");

    //init gets created at the very start, before system calls are executed
    pcb_table[0].pid = 1;
    pcb_table[0].memory_partition = 6;
    strcpy(pcb_table[0].program_name, "init");
    pcb_table[0].size = 1;

    pcb_table_elements++;

    //adds the initial init to the paritions table and is now occupied
    strcpy(partitions[5].code, "init");

    //initializes an array of 250 events 
    struct Event trace[250];

    //argv[1] is the name of the input file the user gave
    int num_events = readInputFile(argv[1], trace);
    if (num_events < 0) {
        return 1;
    }

    //opens the output file
    //argv[2] is the second file name the user gave
    FILE *output_file = fopen(argv[2], "w");
    if (!output_file) {
        printf("Cannot open the output file: %s\n", argv[2]);
        return 1;
    }

    //opens the other output file
    system_status = fopen("system_status.txt", "w");
    if (!system_status) {
        printf("Cannot open the output file: %s\n", argv[2]);
        return 1;
    }

    //argv[3] is the third file name the user gave
    //calulcates the number of external program files
    int num_external_files = externalFilesInfo(argv[3], tracker); // Use argv[3] for the external file
    if (num_external_files < 0) {
        return 1;
    }
    
    handle_system_output(system_status);


    //loops over each line of the array of events and then calls the command functions for each line depeding on the command 
    for (int i = 0; i < num_events; i++) {
        if (strcmp(trace[i].type, "CPU") == 0) {
            handle_cpu(output_file, trace[i].duration);
        } else if (strcmp(trace[i].type, "SYSCALL") == 0) {
            handle_syscall(output_file, trace[i].event_number, trace[i].duration);
        } else if (strcmp(trace[i].type, "FORK") == 0) {
            handle_fork(output_file, trace[i].event_number, trace[i].duration);
        } else if (strcmp(trace[i].type, "EXEC") == 0) {
            handle_exec(output_file, trace[i].file_name, trace[i].duration);
        } 
    } // End of for loop

    //closes the output file
    fclose(output_file);
    return 0;
}
