
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// Global variable to track the current simulation time
extern int current_time;


struct Memory_Partition{
    int partition_number;
    int size;
    int occupied_by;  
};

struct PCB{
    int pid;
    int size;
    int arrival_time;
    int total_cpu_time;
    int io_frequency;
    int io_duration;
    int partition_number;
    char state[10]; // NEW, READY, RUNNING, WAITING, TERMINATED
    //I realized how much i been avoiding padhai i think iss event ke baad underground hona padega😭
} ;


#endif 

