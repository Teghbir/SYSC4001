
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// Global variable to track the current simulation time
extern int current_time;

//Struct for the events from the input file
struct Event {
    char type[10];       
    int event_number;     
    int duration;  
    char file_name[10];  

};

struct Memory_Partition{
    int partition_number;
    int size;
    char code[10];  
};

struct PCB{
    int pid;
    char program_name[32];  
    int memory_partition; // Index of the memory partition allocated.
    int size;
} ;

struct External_Files{
    char program_name[20];
    int program_size;
};

//hardcoded vector table
unsigned int vector_table[] = {
    0X01E3, 0X029C, 0X0695, 0X042B, 0X0292, 0X048B, 0X0639, 0X00BD,
    0X06EF, 0X036C, 0X07B0, 0X01F8, 0X03B9, 0X06C7, 0X0165, 0X0584,
    0X02DF, 0X05B3, 0X060A, 0X0765, 0X07B7, 0X0523, 0X03B7, 0X028C,
    0X05E8, 0X05D3
};

//function prototypes
void handle_cpu(FILE *output_file, int duration);
void handle_syscall(FILE *output_file, int event_number, int duration);
void handle_system_output(FILE *system_status);
#endif 

