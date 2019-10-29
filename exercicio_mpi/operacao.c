/**
 * 
 * Sérgio Ferreira Batista Filho RA: 187064
 * 
 * Compilar: mpicc operacao.c -lm -o operacao.o
 * Executar: qsub ./script_pbs.sh
 *
 * Mantenha o arquivo matrizes.txt no mesmo diretório do executável "operacao.o"
 * O arquivo matrizes_entrada.txt deverá ter o seguinte formato:
 *
 * INTEIRO (tamanho N das próximas três matrizes N x N)
 * INTEIRO INTEIRO INTEIRO ... (valores da primeira matriz)
 * INTEIRO INTEIRO INTEIRO ... (valores da segunda matriz)
 * INTEIRO INTEIRO INTEIRO ... (valores da terceira matriz)
 *
 * INTEIRO (tamanho N das próximas três matrizes N x N, ou zero para terminar)
 * ...
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ARQUIVO_ENTRADA "./matrizes_entrada.txt"
#define MASTER 0

void processar(int *m1, int *m2, int *m3, int tamanho);

void operacaoSerial(int *m1, int *m2, int *m3, int *resultado, int tamanho);

void operacaoParalelo(int *m1, int *m2, int *m3, int *resultado, int tamanho);

void scan100integers(char *string, int *m, int *length);

int main(int argc, char *argv[]) {
    int numtasks, taskid;
    MPI_Status status;
    MPI_File file;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
    int tamanho = -1;
    
    int *m1; // Matriz representada por um único vetor
    int *m2;
    int *m3;
    
    int *m1_part;
    int *m2_part;
    int *m3_part;

    register int i;
    register int j;
    
    char *conteudo = NULL;
    int charsLidos = 0;
    
    MPI_File_open(MPI_COMM_WORLD, ARQUIVO_ENTRADA, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    
    MPI_File_seek(file, 0, MPI_SEEK_END);
    MPI_Offset pos;
    MPI_File_get_position(file, &pos);
    int length = (int) pos;
    printf("Numero de caracteres: %d\n", length);
    conteudo = (char*) malloc(sizeof(char) * (length + 1));
    conteudo[length] = 0;
    MPI_File_read_at_all(file, 0, conteudo, length, MPI_CHAR, &status);
    
    while(tamanho != 0) {
        if (taskid == MASTER) {
            printf("Lendo tamanho N das tres matrizes N x N ou 0 para sair...\n");
            
            int n = 0;
            sscanf(conteudo + charsLidos, "%d%n", &tamanho, &n);
            charsLidos += n;
            printf("Tamanho obtido: %d\n", tamanho);
        }
        
        // Enviar tamanho da matriz para todos os processos
        MPI_Bcast(&tamanho, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        
        if(tamanho == 0) break;
        
        printf("Processo %d obteve valor %d\n", taskid, tamanho);
        
        if (taskid == MASTER) {
            printf("Lendo os valores das tres matrizes %d x %d...\n", tamanho, tamanho);
            m1 = (int*) malloc(sizeof(int) * tamanho * tamanho);
            for(i = 0; i < tamanho * tamanho; i+=100) {
                if(i%500000 == 0 && i > 0) printf("%d\n", i);
                int n = 0;
                scan100integers(conteudo+charsLidos, m1 + i, &n);
                charsLidos += n;
            }
            
            printf("Lendo valores da segunda matriz...\n");
            m2 = (int*) malloc(sizeof(int) * tamanho * tamanho);
            for(i = 0; i < tamanho * tamanho; i+=100) {
                if(i%500000 == 0 && i > 0) printf("%d\n", i);
                int n = 0;
                scan100integers(conteudo+charsLidos, m2 + i, &n);
                charsLidos += n;
            }
            
            printf("Lendo valores da terceira matriz...\n");
            m3 = (int*) malloc(sizeof(int) * tamanho * tamanho);
            for(i = 0; i < tamanho * tamanho; i+=100) {
                if(i%500000 == 0 && i > 0) printf("%d\n", i);
                int n = 0;
                scan100integers(conteudo+charsLidos, m3 + i, &n);
                charsLidos += n;
            }
            
            printf("Processando...\n");
            struct timespec start, end;
            double time_used;
            clock_gettime(CLOCK_REALTIME, &start);
            int *resultado = (int*) malloc(sizeof(int) * tamanho * tamanho);
            operacaoSerial(m1, m2, m3, resultado, tamanho);
            clock_gettime(CLOCK_REALTIME, &end);
            time_used = (int) (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
            printf("Operacao sobre matrizes %d x %d com processamento serial levou %f segundos\n", tamanho, tamanho, time_used);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Processamento paralelo
        int size = tamanho * tamanho / numtasks;
        m1_part = (int*) malloc(sizeof(int) * size);
        m2_part = (int*) malloc(sizeof(int) * size);
        m3_part = (int*) malloc(sizeof(int) * size);
        int *resultado_part = (int*) malloc(sizeof(int) * tamanho * tamanho);
        
        MPI_Scatter(m1, size, MPI_INT, m1_part, size, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Scatter(m2, size, MPI_INT, m2_part, size, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Scatter(m3, size, MPI_INT, m3_part, size, MPI_INT, MASTER, MPI_COMM_WORLD);
        
        int offset = taskid * size;
        
        int i_start = taskid * (tamanho / numtasks);
        int i_end = (taskid + 1) * (tamanho / numtasks);
        
        register int k;
        
        struct timespec start, end;
        if (taskid == MASTER) {
            printf("Processando...\n");
            clock_gettime(CLOCK_REALTIME, &start);
        }
        
        // Multiplicação
        for (i = i_start; i < i_end; i++) {
            for (j = 0; j < tamanho; j++) {
                resultado_part[i * tamanho + j - offset] = 0;
                for (k = 0; k < tamanho; k++) {
                    resultado_part[i * tamanho + j - offset] += 
                        m1_part[i * tamanho + k - offset] * m2_part[i * tamanho + j - offset];
                }
                
                // Soma
                resultado_part[i * tamanho + j  - offset] += m3_part[i * tamanho + j - offset];
            }
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        
        if (taskid == MASTER) {
            int *resultado = (int*) malloc(sizeof(int) * tamanho * tamanho);
            
            double time_used;
            clock_gettime(CLOCK_REALTIME, &end);
            time_used = (int) (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
            printf("Operacao sobre matrizes %d x %d com processamento paralelo com %d processos levou %f segundos\n", tamanho, tamanho, numtasks, time_used);
        }
    }
    
    MPI_File_close(&file);
    
    MPI_Finalize();
    
    return 0;
}


void operacaoSerial(int *m1, int *m2, int *m3, int *resultado, int tamanho) { 
    register int i, j, k;
    
    // Multiplicação
    for (i = 0; i < tamanho; i++) {
        for (j = 0; j < tamanho; j++) {
            resultado[i * tamanho + j] = 0;
            for (k = 0; k < tamanho; k++) {
                resultado[i * tamanho + j] += m1[i * tamanho + k] * m2[k * tamanho + j];
            }
            
            // Soma
            resultado[i * tamanho + j] += m3[i * tamanho + j];
        }
    }
}

// Função implementada para obter mais eficiência em casos em que os servidores do fedcloud estiveram sobrecarregados
void scan100integers(char *string, int *m, int *length) {
    sscanf(
        string,
        "%d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d%n",
        m, m+1, m+2, m+3, m+4, m+5, m+6, m+7, m+8, m+9,
        m+10, m+11, m+12, m+13, m+14, m+15, m+16, m+17, m+18, m+19,
        m+20, m+21, m+22, m+23, m+24, m+25, m+26, m+27, m+28, m+29,
        m+30, m+31, m+32, m+33, m+34, m+35, m+36, m+37, m+38, m+39,
        m+40, m+41, m+42, m+43, m+44, m+45, m+46, m+47, m+48, m+49,
        m+50, m+51, m+52, m+53, m+54, m+55, m+56, m+57, m+58, m+59,
        m+60, m+61, m+62, m+63, m+64, m+65, m+66, m+67, m+68, m+69,
        m+70, m+71, m+72, m+73, m+74, m+75, m+76, m+77, m+78, m+79,
        m+80, m+81, m+82, m+83, m+84, m+85, m+86, m+87, m+88, m+89,
        m+90, m+91, m+92, m+93, m+94, m+95, m+96, m+97, m+98, m+99,
        length
    );
}










