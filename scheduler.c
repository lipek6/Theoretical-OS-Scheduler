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


// Functions declaration

process* cloneProcList(process* ogProcList, int nProc);
void PriorityBased(process *globaProcList, int nProc, int tTroca, int test_count);
void RoundRobin(process *globaProcList, int nProc, int quantum, int tTroca, int test_count);


int main (void)
{   
    int testcase_count = 0;

    FILE* input_file;

    while (testcase_count < 4)
    {
        if (testcase_count == 0)
        {
            input_file = fopen("input.txt", "r");
        } else if (testcase_count == 1)
        {
            input_file = fopen("input1.txt", "r");
        } else if (testcase_count == 2)
        {
            input_file = fopen("input2.txt", "r");
        } else if (testcase_count == 3)
        {
            input_file = fopen("input3.txt", "r");
        }
        
        
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

        RoundRobin(procList, nProc, quantum, tTroca, testcase_count);
        PriorityBased(procList, nProc, tTroca, testcase_count);

        free(procList);

        testcase_count++;
    }
    
}



void RoundRobin(process *globaProcList, int nProc, int quantum, int tTroca, int test_count)
{
    process *localProcList = cloneProcList(globaProcList, nProc);
    FILE* out_robin;
    if (test_count == 0)
    {
        out_robin = fopen("out_robin.txt", "w");
    } else if (test_count == 1)
    {
        out_robin = fopen("out_robin1.txt", "w");
    }else if (test_count == 2)
    {
        out_robin = fopen("out_robin2.txt", "w");
    }else if (test_count == 3)
    {
        out_robin = fopen("out_robin3.txt", "w");
    }

    if (out_robin == NULL)
    {
        fprintf(stderr, "Failed to create/write on the out_robin.txt file.\n");
        free(localProcList);
        return;
    }
    
    int time = 0;              // Relógio da simulação (ms)
    int finishedCount = 0;     // Quantidade de processos finalizados
    
    int current = -1;          // Índice do processo em execução (-1 = CPU livre)
    int quantumCounter = 0;    // Tempo já usado do quantum atual
    
    int switchTimer = 0;       // Contador regressivo para o tempo de troca
    int nextProcessId = -1;    // Quem será o próximo a assumir após a troca
    
    // Circular queue 
    int *readyQueue = malloc(nProc * sizeof(int));
    int head = 0;
    int tail = 0;
    int qCount = 0;

    /* Controle para não inserir processo duas vezes */
    int inserted[nProc];

    /* Guarda o tempo de finalização de cada processo */
    int finishTime[nProc];

    /* Inicializa todos como não inseridos */
    for (int i = 0; i < nProc; i++) inserted[i] = 0;

    int switchCount = 0;       // Número de trocas de contexto
    int overheadTime = 0;      // Tempo total gasto em trocas

    fprintf(out_robin, "=== ROUND ROBIN ===\n");
    fprintf(out_robin, "Linha do tempo:\n");

    /* Loop principal: executa até todos os processos terminarem */
    while (finishedCount < nProc)
    {
        // VERIFICA CHEGADAS
        for (int i = 0; i < nProc; i++)
        {
            if (localProcList[i].bornTime <= time && !inserted[i])
            {
                /* Processo entra na fila de prontos */
                readyQueue[tail] = i;
                tail = (tail + 1) % nProc;
                qCount++;
                
                inserted[i] = 1;
            }
        }
        
        // MÁQUINA DE ESTADOS

        if (switchTimer > 0) 
        {
            // ESTADO: TROCA DE CONTEXTO
            fprintf(out_robin, "t=%d -> Escalonador\n", time);
            switchTimer--;
            overheadTime++;

            // Se o tempo de troca acabou, o processo assume a CPU
            if (switchTimer == 0)
            {
                current = nextProcessId;
                quantumCounter = 0;
                nextProcessId = -1; 
            }
        }
        
        else if (current != -1) 
        {
            // ESTADO: EXECUTANDO PROCESSO
            fprintf(out_robin, "t=%d -> P%d\n", time, localProcList[current].ID); // [cite: 22, 72]
            
            localProcList[current].remCpuTime--;
            quantumCounter++;

            if (localProcList[current].remCpuTime == 0) 
            {
                finishTime[current] = time + 1; 
                finishedCount++;
                current = -1; 
            }
            else if (quantumCounter == quantum) 
            {
                readyQueue[tail] = current;
                tail = (tail + 1) % nProc;
                qCount++;

                current = -1;
            }
        }

        else 
        {            
            // ESTADO: CPU LIVRE (scheduler working)
            if (qCount > 0) 
            {
                nextProcessId = readyQueue[head];
                head = (head + 1) % nProc;
                qCount--;

                if (tTroca > 0)
                {
                    switchCount++;
                    switchTimer = tTroca;
                    fprintf(out_robin, "t=%d -> Escalonador\n", time);
                    switchTimer--;
                    overheadTime++;
                    
                    if (switchTimer == 0)
                    {
                        current = nextProcessId;
                        quantumCounter = 0;
                        nextProcessId = -1;
                    }
                }
                else
                {
                    current = nextProcessId;
                    quantumCounter = 0;
                    fprintf(out_robin, "t=%d -> P%d\n", time, localProcList[current].ID);
                    localProcList[current].remCpuTime--;
                    quantumCounter++;
                }
            }
            else 
            {
                // ESTADO: IDLE
                fprintf(out_robin, "t=%d -> IDLE\n", time);
            }
        }    
        time++; 
    }

    /* 4) Cálculo do tempo médio de retorno */
    float totalTurnaround = 0.0;
    for (int i = 0; i < nProc; i++)
        totalTurnaround += (finishTime[i] - localProcList[i].bornTime);
    float overheadFraction = (time > 0) ? (float)overheadTime / time : 0.0;


    /* Resultados exigidos no PDF */
    fprintf(out_robin, "\n=== RESULTADOS ===\n");
    fprintf(out_robin, "Tempo medio de retorno: %.2f\n", totalTurnaround / nProc); // [cite: 21, 68]
    fprintf(out_robin, "Numero de chaveamento de processos: %d\n", switchCount); // [cite: 23, 69]
    fprintf(out_robin, "Overhead de chaveamento: %.4f (%.2f%%)\n", overheadFraction, overheadFraction * 100); // 
    fprintf(out_robin, "Tempo total de simulacao: %d ms\n", time); // [cite: 25, 71]    /* Libera a memória da cópia local */

    fclose(out_robin);
    free(readyQueue);
    free(localProcList);
}


// Simulates a priority based algorithm to select the executing process
void PriorityBased(process *globaProcList, int nProc, int tTroca, int test_count)
{
    process* localProcList = cloneProcList(globaProcList, nProc);
    FILE* out_priority;
    
    if (test_count == 0)
    {
        out_priority = fopen("out_priority.txt", "w");
    } else if (test_count == 1)
    {
        out_priority = fopen("out_priority1.txt", "w");
    } else if (test_count == 2)
    {
        out_priority = fopen("out_priority2.txt", "w");
    } else if (test_count == 3)
    {
        out_priority = fopen("out_priority3.txt", "w");
    }

    if (out_priority == NULL)
    {
        fprintf(stderr, "Failed to create/write on the out_priority.txt file.\n");
        free(localProcList);
        return;
    }

   int time = 0;                    // Simulation timer 
   int finishedCount = 0;           // Amount of finnished processes to control when we should stop the simulation
   int current = -1;                // Current process on the execution state (-1 by default means no process executing)
   int switchTimer = 0;             // Timer to simulate the context switch realized by the scheduler 
   int nextProcessId = -1;          // Id of the next process on the ready queue
   
   int switchCount = 0;             // Counter to save how many times the scheduler needed to change the executing process
   int overheadTime = 0;            // Time lost on the scheduler menagement of the processes
   int finishTime[nProc];           // Array to save when each process ended

    fprintf(out_priority, "=== PRIORIDADE (PREEMPTIVO) ===\n");
    fprintf(out_priority, "Linha do tempo:\n");


    while(finishedCount < nProc)
    {
        // Choosing the next process
        int bestProcessId = -1;
        for(int i = 0; i < nProc; i++)
        {
            if (localProcList[i].bornTime <= time && localProcList[i].remCpuTime > 0)
            {
                if (bestProcessId == -1) bestProcessId = i;
                else
                {
                    if (localProcList[i].priority < localProcList[bestProcessId].priority)
                        bestProcessId = i;
                        
                    else if (localProcList[i].priority == localProcList[bestProcessId].priority)
                    {
                        if (localProcList[i].ID < localProcList[bestProcessId].ID) bestProcessId = i;
                    }
                }
            }
        }

        // STATE MACHINE
        
        if(switchTimer > 0)
        { // Context swap
            fprintf(out_priority, "t=%d -> Escalonador\n", time);
            switchTimer--;
            overheadTime++;

            if (switchTimer == 0)
            {
                current = nextProcessId;
                nextProcessId = -1;
            }
        }

        else if (bestProcessId != current)
        { // The guy executing isn't the guy with highest priority, we shall take him out
            if (tTroca > 0 && bestProcessId != -1)
            {
                nextProcessId = bestProcessId;
                switchTimer = tTroca;
                switchCount++;

                fprintf(out_priority, "t=%d -> Escalonador\n", time);
                switchTimer--;
                overheadTime++;

                if(switchTimer == 0)
                {
                    current = nextProcessId;
                    nextProcessId = -1;
                }
                else current = -1;
            }
            else
            {
                current = bestProcessId;
                if (current != -1)
                {
                    fprintf(out_priority, "t=%d -> P%d\n", time, localProcList[current].ID);
                    localProcList[current].remCpuTime--;
                    if(localProcList[current].remCpuTime == 0)
                    {
                       finishTime[current] = time + 1;
                       finishedCount++;
                       current = -1;
                    }
                }
                else
                {
                    fprintf(out_priority, "t=%d -> IDLE\n", time);
                }
            } 
        }
        
        else if(switchTimer == 0 && current != -1)
        { // Common execution
            fprintf(out_priority, "t=%d -> P%d\n", time, localProcList[current].ID);
            localProcList[current].remCpuTime--;

            if(localProcList[current].remCpuTime == 0)
            {
                finishTime[current] = time+1;
                finishedCount++;
                
                current = -1;
            }
        }
        else if (switchTimer == 0 && current == -1)
            fprintf(out_priority, "t=%d -> IDLE\n", time);
        time++;
    }

    float totalTurnaround = 0.0;
    for (int i = 0; i < nProc; i++)
        totalTurnaround += (finishTime[i] - localProcList[i].bornTime);
    
    float overheadFraction = (time > 0) ? (float)overheadTime / time : 0.0;

    fprintf(out_priority, "\n=== RESULTADOS ===\n");
    fprintf(out_priority, "Tempo medio de retorno: %.2f\n", totalTurnaround / nProc); 
    fprintf(out_priority, "Numero de chaveamento de processos: %d\n", switchCount);
    fprintf(out_priority, "Overhead de chaveamento: %.4f (%.2f%%)\n", overheadFraction, overheadFraction * 100); 
    fprintf(out_priority, "Tempo total de simulacao: %d ms\n", time);


    fclose(out_priority);
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