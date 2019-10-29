#!/bin/bash
# Script exemplo para submissao de jobs ao sistema de filas Torque/PBS
# Essa e uma versao para ambientes paralelos.
#
# Criado por Andre Leon S. Gradvohl, Dr.
# Ultima atualizacao: Sex Mar 23 19:52:56 BRST 2019
#
# Definindo o nome do Job
#PBS -N nome_do_job
#
# Definindo arquivo de saida e arquivo de erros
#PBS -o arqSaida.out
#PBS -e arqErro.err
#
# Definindo o nome da fila (paralela)
#PBS -q batch
#
# Quantidade de nos e tarefas por no
#PBS -l nodes=4:ppn=2
#
# Configuracoes de variaveis locais
## Daemon para rodar o MPI
MPIRUN=/usr/bin/mpirun
#
## Arquivo com o endereco dos hosts
MPI_HOSTS=${PBS_O_WORKDIR}/host_file
#
## Arquivo com o programa executavel
PROGRAM=${PBS_O_WORKDIR}/operacao.o
#
## Numero de processos a serem criados
N_PROCS=4
#
# Informa ao PBS para mudar para o diretorio de trabalho
cd $PBS_O_WORKDIR
echo "Diretorio trabalho: $PBS_O_WORKDIR"
#
# Inicia a contagem de tempo
START=$(($(date +%s%N)/1000000))
#
# Dispara N_PROCS instancias do programa
#
${MPIRUN} -n ${N_PROCS} -hostfile ${MPI_HOSTS} ${PROGRAM}
#
# Fim da contagem de tempo
END=$(($(date +%s%N)/1000000))
#
# Calculo da duracao de tempo e impressao na tela
DURACAO=$((END-START))
echo "Tempo de processamento ${DURACAO} millisegundos"
