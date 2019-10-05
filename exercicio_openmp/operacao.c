/**
 * 
 * Sérgio Ferreira Batista Filho RA: 187064
 * 
 * Compilar: gcc -pthread operacao.c -o operacao.o
 * Executar: ./operacao.o < matrizes_entrada.txt
 *
 * O arquivo matrizes_entrada.txt deverá ter o seguinte formato:
 *
 * INTEIRO (tamanho das próximas três matrizes)
 * INTEIRO INTEIRO INTEIRO ... (valores da primeira matriz)
 * INTEIRO INTEIRO INTEIRO ... (valores da segunda matriz)
 * INTEIRO INTEIRO INTEIRO ... (valores da terceira matriz)
 *
 * INTEIRO (tamanho das próximas três matrizes, ou zero para terminar)
 * ...
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void processar(int **matriz1, int **matriz2, int **matriz3, int tamanho);

void operacaoSerial(int **matriz1, int **matriz2, int **matriz3, int **resultado, int tamanho);

void operacaoParalelo(int **matriz1, int **matriz2, int **matriz3, int **resultado, int tamanho, int num_threads);

int main(int argc, char *argv[]) {
    int tamanho;
    int **matriz1;
    int **matriz2;
    int **matriz3;
    
    register int i;
    register int j;
    
    while(1) {
        printf("Digite o tamanho N das tres matrizes N x N a serem lidas ou digite 0 para sair\n");
        
        scanf("%d", &tamanho);
        if(tamanho == 0) break;
        
        printf("Digite os valores das tres matrizes %d x %d\n", tamanho, tamanho);
        
        matriz1 = (int**) malloc(sizeof(int*) * tamanho);
        for(i = 0; i < tamanho; i++) {
            matriz1[i] = (int*) malloc(sizeof(int) * tamanho);
            for(j = 0; j < tamanho; j++) {
                scanf("%d", &matriz1[i][j]);
            }
        }
        
        matriz2 = (int**) malloc(sizeof(int*) * tamanho);
        for(i = 0; i < tamanho; i++) {
            matriz2[i] = (int*) malloc(sizeof(int) * tamanho);
            for(j = 0; j < tamanho; j++) {
                scanf("%d", &matriz2[i][j]);
            }
        }
        
        matriz3 = (int**) malloc(sizeof(int*) * tamanho);
        for(i = 0; i < tamanho; i++) {
            matriz3[i] = (int*) malloc(sizeof(int) * tamanho);
            for(j = 0; j < tamanho; j++) {
                scanf("%d", &matriz3[i][j]);
            }
        }
        
        processar(matriz1, matriz2, matriz3, tamanho);
        
        for(i = 0; i < tamanho; i++) {
            free(matriz1[i]);
            free(matriz2[i]);
            free(matriz3[i]);
        }
        free(matriz1);
        free(matriz2);
        free(matriz3);
    }
    
    return 0;
}

void processar(int **matriz1, int **matriz2, int **matriz3, int tamanho) {
    int **resultado = (int**) malloc(sizeof(int*) * tamanho);
    register int i;
    for(i = 0; i < tamanho; i++) {
        resultado[i] = (int*) malloc(sizeof(int) * tamanho);
    }
    
    struct timespec start, end;
    double time_used;
    
    clock_gettime(CLOCK_REALTIME, &start);
    operacaoSerial(matriz1, matriz2, matriz3, resultado, tamanho);
    clock_gettime(CLOCK_REALTIME, &end);
    time_used = (int) (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
    
    printf("Operacao sobre matrizes %d x %d com processamento serial levou %f segundos\n", tamanho, tamanho, time_used);
    
    int t;
    for(t = 2; t <= 32; t *= 2) {
        clock_gettime(CLOCK_REALTIME, &start);
        operacaoParalelo(matriz1, matriz2, matriz3, resultado, tamanho, t);
        clock_gettime(CLOCK_REALTIME, &end);
        time_used = (int) (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.0e9);
        
        printf("Operacao sobre matrizes %d x %d com processamento paralelo com %d threads levou %f segundos\n", tamanho, tamanho, t, time_used);
    }
    
    for(i = 0; i < tamanho; i++) {
        free(resultado[i]);
    }
    free(resultado);
}

void operacaoSerial(int **matriz1, int **matriz2, int **matriz3, int **resultado, int tamanho) { 
    register int i, j, k;
    
    // Multiplicação
    for (i = 0; i < tamanho; i++) {
        for (j = 0; j < tamanho; j++) {
            resultado[i][j] = 0;
            for (k = 0; k < tamanho; k++) {
                resultado[i][j] += matriz1[i][k] * matriz2[k][j];
            }
            
            // Soma
            resultado[i][j] += matriz3[i][j];
        }
    }
}

void operacaoParalelo(int **matriz1, int **matriz2, int **matriz3, int **resultado, int tamanho, int num_threads) {
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel
    {
        register int i, j, k;
        
        // Multiplicação
        #pragma omp for
        for (i = 0; i < tamanho; i++) {
            for (j = 0; j < tamanho; j++) {
                resultado[i][j] = 0;
                for (k = 0; k < tamanho; k++) {
                    resultado[i][j] += matriz1[i][k] * matriz2[k][j];
                }
                
                // Soma
                resultado[i][j] += matriz3[i][j];
            }
        }
    }
}










