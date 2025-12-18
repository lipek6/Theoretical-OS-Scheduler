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
    FILE* out_robin = fopen("out_robin.txt", "w");

    
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
void PriorityBased(process *globaProcList, int nProc, int tTroca)
{
    process* localProcList = cloneProcList(globaProcList, nProc);
    FILE* out_priority = fopen("out_priority.txt", "w");

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