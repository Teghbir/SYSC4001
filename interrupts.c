//necessary includes
#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>
#include "interrupts.h"
#include <time.h>

//initiazling the global variable
int current_time = 0; 

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
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SYSCALL", 7) == 0 || strncmp(line, "END_IO", 6) == 0) {
            sscanf(line, "%s %d, %d", trace[i].type, &trace[i].event_number, &trace[i].duration);
        } else if (strncmp(line, "CPU", 3) == 0) {
            sscanf(line, "%[^,], %d", trace[i].type, &trace[i].duration);
            trace[i].event_number = -1;  // CPU doesn't have an event number
        }
        i++;
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

//to handle the command END_IO
void handle_end_io(FILE *output_file, int event_number, int duration) {
    fprintf(output_file, "%d, %d, check priority of interrupt\n", current_time, 1);
    current_time += 1;
    fprintf(output_file, "%d, %d, check if masked\n", current_time, 1);
    current_time += 1;
    fprintf(output_file, "%d, %d, switch to kernel mode\n", current_time, 1);
    current_time += 1;
    fprintf(output_file, "%d, %d, context saved\n", current_time, 3);
    current_time += 3;
    fprintf(output_file, "%d, %d, find vector %d in memory position 0x%04hhx\n", current_time, 1, event_number, event_number * 2);
    current_time += 1;
    fprintf(output_file, "%d, %d, load address 0x0%X into the PC\n", current_time, 1, vector_table[event_number]);
    current_time += 1;
    fprintf(output_file, "%d, %d, END_IO\n", current_time, duration);
    current_time += duration;
    fprintf(output_file, "%d, %d, IRET\n", current_time, 1);
    current_time += 1;
}

//main function
int main(int argc, char *argv[]) {
    //ensures that the user runs the program with atleast 2 files
    if (argc < 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    //this is to seed the random number generator for different values everytime its run
    srand(time(NULL));

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

    //loops over each line of the array of events and then calls the command functions for each line depeding on the command 
    for (int i = 0; i < num_events; i++) {
        if (strcmp(trace[i].type, "CPU") == 0) {
            handle_cpu(output_file, trace[i].duration);
        } else if (strcmp(trace[i].type, "SYSCALL") == 0) {
            handle_syscall(output_file, trace[i].event_number, trace[i].duration);
        } else if (strcmp(trace[i].type, "END_IO") == 0) {
            handle_end_io(output_file, trace[i].event_number, trace[i].duration);
        }
    }

    //closes the output file
    fclose(output_file);
    return 0;
}
