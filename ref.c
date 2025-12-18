void PriorityBased(process *globaProcList, int nProc, int tTroca)
{
    process* localProcList = cloneProcList(globaProcList, nProc);
    FILE* out_prio = fopen("output_prio.txt", "w"); // Nome sugerido para saída

    if (out_prio == NULL) {
        fprintf(stderr, "Erro ao criar arquivo de saida de prioridade.\n");
        free(localProcList);
        return;
    }

    int time = 0;
    int finishedCount = 0;
    int current = -1;       // Quem está na CPU agora (-1 = Ninguém)
    int switchTimer = 0;    // Contador para a troca de contexto
    int nextProcessId = -1; // Quem vai assumir após a troca

    // Para estatísticas
    int switchCount = 0;
    int overheadTime = 0;
    int finishTime[nProc];  // Para guardar quando cada um terminou

    fprintf(out_prio, "\n=== PRIORIDADE (PREEMPTIVO) ===\n");
    fprintf(out_prio, "Linha do tempo:\n");

    while (finishedCount < nProc)
    {
        // 1. DECISÃO: Quem deveria estar rodando agora?
        int bestCandidate = -1;

        for (int i = 0; i < nProc; i++)
        {
            // Verifica se o processo já nasceu e ainda não terminou
            if (localProcList[i].bornTime <= time && localProcList[i].remCpuTime > 0)
            {
                // Se ainda não escolhi ninguém, pego este
                if (bestCandidate == -1) {
                    bestCandidate = i;
                }
                else {
                    // Critério 1: Menor valor de prioridade ganha 
                    if (localProcList[i].priority < localProcList[bestCandidate].priority) {
                        bestCandidate = i;
                    }
                    // Critério 2: Empate de prioridade -> Menor ID ganha 
                    else if (localProcList[i].priority == localProcList[bestCandidate].priority) {
                        if (localProcList[i].ID < localProcList[bestCandidate].ID) {
                            bestCandidate = i;
                        }
                    }
                }
            }
        }

        // 2. MÁQUINA DE ESTADOS
        
        // Cenario A: Estamos no meio de uma troca de contexto
        if (switchTimer > 0)
        {
            fprintf(out_prio, "t=%d -> Escalonador\n", time); // [cite: 38]
            switchTimer--;
            overheadTime++;

            // Se a troca terminou, o candidato assume
            if (switchTimer == 0) {
                current = nextProcessId;
                nextProcessId = -1;
            }
        }
        // Cenario B: Precisamos trocar de processo?
        // Se o candidato mudou em relação a quem está na CPU (current), inicia troca.
        else if (bestCandidate != current)
        {
            // Se já tinha alguém rodando ou se vai entrar alguém novo (saindo de IDLE)
            if (tTroca > 0 && bestCandidate != -1) 
            {
                nextProcessId = bestCandidate;
                switchTimer = tTroca;
                switchCount++; // [cite: 23]
                
                // Processa o primeiro milissegundo da troca agora
                fprintf(out_prio, "t=%d -> Escalonador\n", time);
                switchTimer--;
                overheadTime++;
                
                // Caso tTroca seja 1, já finaliza a troca aqui
                if (switchTimer == 0) {
                    current = nextProcessId;
                    nextProcessId = -1;
                } else {
                    current = -1; // CPU fica "em transição"
                }
            }
            else {
                // Se tTroca for 0, troca instantânea
                current = bestCandidate;
            }
        }
        
        // Cenario C: Execução normal
        if (switchTimer == 0 && current != -1)
        {
            fprintf(out_prio, "t=%d -> P%d\n", time, localProcList[current].ID); // [cite: 22]
            localProcList[current].remCpuTime--;
            
            // Verifica se acabou
            if (localProcList[current].remCpuTime == 0) {
                finishTime[current] = time + 1;
                finishedCount++;
                // Importante: current não vira -1 imediatamente aqui para permitir
                // que a lógica de "Change" detecte a mudança na próxima iteração se houver outro processo.
                // Mas, logicamente, ele solta a CPU.
                current = -1; 
            }
        }
        else if (switchTimer == 0 && current == -1)
        {
            fprintf(out_prio, "t=%d -> IDLE\n", time);
        }

        time++;
    }

    // --- CÁLCULOS FINAIS ---
    float totalTurnaround = 0.0;
    for (int i = 0; i < nProc; i++)
        totalTurnaround += (finishTime[i] - localProcList[i].bornTime);
    
    float overheadFraction = (time > 0) ? (float)overheadTime / time : 0.0;

    fprintf(out_prio, "\n=== RESULTADOS ===\n");
    fprintf(out_prio, "Tempo medio de retorno: %.2f\n", totalTurnaround / nProc); // [cite: 21]
    fprintf(out_prio, "Numero de chaveamento de processos: %d\n", switchCount);
    fprintf(out_prio, "Overhead de chaveamento: %.4f (%.2f%%)\n", overheadFraction, overheadFraction * 100); // [cite: 24]
    fprintf(out_prio, "Tempo total de simulacao: %d ms\n", time);

    fclose(out_prio);
    free(localProcList);
}