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
    float avgRetTime;           // Average return time of the processes on the simulation
    float overhead;             // Ratio between the time of overhead and the total time
    int switchsCnt;             // Amount of times that the scheduler switched the process on execution
    int totalTime;              // Total amount of time to execute all processes
}simulationData;



// Functions declaration

process* cloneProcList(process* ogProcList, int nProc);
void PriorityBased(process *globaProcList, int nProc, int tTroca);
void RoundRobin(process *globaProcList, int nProc, int quantum, int tTroca);


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



void RoundRobin(process *globaProcList, int nProc, int quantum, int tTroca)
{
 
    process *localProcList = cloneProcList(globaProcList, nProc);

    int time = 0;              // Relógio da simulação (ms)
    int finished = 0;          // Quantidade de processos finalizados

    int current = -1;          // Índice do processo em execução (-1 = CPU livre)
    int quantumCounter = 0;    // Tempo já usado do quantum atual

    /* Fila de prontos (Round Robin usa fila) */
    int readyQueue[1000];
    int front = 0;
    int rear = 0;

    /* Controle para não inserir processo duas vezes */
    int inserted[nProc];

    /* Guarda o tempo de finalização de cada processo */
    int finishTime[nProc];

    /* Inicializa todos como não inseridos */
    for (int i = 0; i < nProc; i++)
        inserted[i] = 0;

    int switchCount = 0;       // Número de trocas de contexto
    int overheadTime = 0;      // Tempo total gasto em trocas

    printf("\n=== ROUND ROBIN ===\n");
    printf("Linha do tempo:\n");

    /* Loop principal: executa até todos os processos terminarem */
    while (finished < nProc)
    {
        /* 1) Verifica quais processos chegam neste instante */
        for (int i = 0; i < nProc; i++)
        {
            if (localProcList[i].bornTime == time && !inserted[i])
            {
                /* Processo entra na fila de prontos */
                readyQueue[rear++] = i;
                inserted[i] = 1;
            }
        }

        /* 2) Se a CPU está livre e há processos prontos, escalona */
        if (current == -1 && front < rear)
        {
            /* Remove o próximo processo da fila */
            current = readyQueue[front++];
            quantumCounter = 0;

            /* Simula a troca de contexto */
            for (int i = 0; i < tTroca; i++)
            {
                printf("t=%d -> Escalonador\n", time);
                time++;
            }

            switchCount++;
            overheadTime += tTroca;
        }

        /* 3) Executa 1 ms do processo atual */
        if (current != -1)
        {
            printf("t=%d -> P%d\n", time, localProcList[current].ID);

            localProcList[current].remCpuTime--;
            quantumCounter++;

            /* Caso o processo tenha terminado */
            if (localProcList[current].remCpuTime == 0)
            {
                finishTime[current] = time + 1;
                finished++;
                current = -1;
            }
            /* Caso o quantum tenha acabado */
            else if (quantumCounter == quantum)
            {
                /* Processo volta para o fim da fila */
                readyQueue[rear++] = current;
                current = -1;
            }
        }
        else
        {
            /* CPU ociosa */
            printf("t=%d -> IDLE\n", time);
        }

        /* Avança o tempo da simulação */
        time++;
    }

    /* 4) Cálculo do tempo médio de retorno */
    float avgReturn = 0.0;
    for (int i = 0; i < nProc; i++)
        avgReturn += (finishTime[i] - localProcList[i].bornTime);

    avgReturn /= nProc;

    /* Resultados exigidos no PDF */
    printf("\nResultados:\n");
    printf("Tempo medio de retorno: %.2f\n", avgReturn);
    printf("Numero de trocas de contexto: %d\n", switchCount);
    printf("Overhead: %.2f\n", (float)overheadTime / time);
    printf("Tempo total de execucao: %d\n", time);

    /* Libera a memória da cópia local */
    free(localProcList);
}


// Makes a copy of the processes list, so that the functions can't mess with the global processes list
process* cloneProcList(process* ogProcList, int nProc)
{
    process* cpProcList = malloc(sizeof(process) * nProc);
    for(int i = 0; i < nProc; i++)
    {
        cpProcList[i] = ogProcList[i];
    }
    return cpProcList;
}