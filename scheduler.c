#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int ID;                      // Process ID
    int bornTime;                // Moment that the process was created
    int priority;                // Priority of the process      
    int reqCpuTime;              // Amount of time on the CPU that the process needs to finnish
    int remCpuTime;              // Remaining CPU time required for the process to finnish
}process;

typedef struct
{
    int avgRetTime;             // Average return time of the processes on the simulation
    int switchsCnt;             // Amount of times that the scheduler switched the process on execution
    int totalTime;              // Total amount of time to execute all processes
}simulationData;



int main (void)
{
    FILE* input_file = fopen("input.txt", "r");
    if(input_file == NULL)
    {
        fprintf(stderr, "Failed opening/reading file, sorry bro\n");
        return 1;
    }

    int nProc, quantum, tTroca;
    if(fscanf(input_file, "%d, %d, %d", &nProc, &quantum, &tTroca) != 3)
    {   // fscanf returns amount of things it has read, we can check it to avoid errors
        fprintf(stderr, "Your input file is a mess, I failed getting the info I needed from it\n");
        return 2;      
    }


    process *procList = malloc(sizeof(process) * nProc);
    for(int i = 0; i < nProc; i++)
    {
        fscanf(input_file, "%d, %d, %d, %d",
            &procList[i].ID,
            &procList[i].bornTime,
            &procList[i].priority,
            &procList[i].reqCpuTime);
        procList[i].remCpuTime = procList[i].reqCpuTime;
    }
    fclose(input_file);

    RoundRobin(procList, nProc, quantum, tTroca);
    PriorityBased(procList, nProc, tTroca);

    free(procList);
}

void RoundRobin(process *procList, int nProc, int quantum, int tTroca)
{

}
void PriorityBased(process *procList, int nProc, int tTroca)
{

}