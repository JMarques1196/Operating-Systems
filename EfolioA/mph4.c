/*
** file: mph4.c  (template) (** UTF-8 **)
**
** UC: 21111 - SO @ UAb
** e-fólio A 2025-26
** mph4 - multi process hash 4 bytes
**  
** Botão VPL "Run"  <=> Utilização com dados do utilizador em terminal:
**    ./mph4 np     # lê da entrada padrão stdin o nome do ficheiro a processar
** Botão VPL "Evaluate" <=> Utilização com redireção de ficheiros automática:
**    ./mph4 np <input.txt >output.txt
**
** Compilação VPL (auto):   gcc -Wall -std=c99 -o mph4 mph4.c
**
** Aluno: NNNNN - Nome Apelido
*/

/* SUBMISSAO:
    - O e-fólio só é considerado entregue com: 
    (i) O código do programa do aluno constante no recurso VPL (espaço central); 
    (ii) A submissão de um ficheiro pdf de uma página com a identificacão do aluno e uma imagem com o resultado dos testes 
    (obtidos com o botão "evaluate" no VPL), no recurso e-fólio no espaço turma.
*/

// Instruções
// Acrescente o código necessário para o programa funcionar corretamente.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>   
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>



// macros
// Num. máximo processos np (trabalhadores)
#define MAXPROC    8
#define MAXFNAME 256

// argc: número de argumentos da linha de comandos
// argv: vetor de strings com os argumentos da linha de comandos
int main(int argc, char *argv[])
{
	int np; // processos a criar (trabalhadores)
    char filename[MAXFNAME]; // nome do ficheiro a processar
    int fd; // descritor do ficheiro a processar
    off_t filesize; // tamanho do ficheiro a processar
    off_t total_blocks; // número total de blocos a processar (cada bloco tem 4 bytes, ou seja, o tamanho de um inteiro)
    off_t blocks_per_worker; // número de blocos a processar por cada trabalhador (processo filho)
    // Verificar se o número de processos a criar foi fornecido como argumento e convertê-lo para inteiro
    if(argc >=2){
        np = atoi(argv[1]);
    }else{
        np = 1; // Valor padrão para np se não for fornecido como argumento
    }

    /* Forçar o número de processos a criar a ser entre 1 e MAXPROC 
        mantendo o programa funcional mesmo que o utilizador forneça um valor inválido para np (ex: 0, negativo, ou maior que MAXPROC)
    */
    if (np < 1) {
        np = 1;
    }
    if (np > MAXPROC) {
        np = MAXPROC;
    }

    // Ler o nome do ficheiro a processar da entrada padrão
    if (scanf("%255s", filename) != 1) {
        return EXIT_FAILURE;
    }
    // Abrir o ficheiro em modo de leitura
    fd = open(filename, O_RDONLY);
    // Verificar se o ficheiro foi aberto com sucesso
    if (fd < 0) {
        return EXIT_FAILURE;
    }

    // Obter o tamanho do ficheiro em bytes
    filesize = lseek(fd, 0, SEEK_END); 
    // Verificar se o tamanho do ficheiro foi obtido com sucesso
    if (filesize < 0) {
        close(fd);
        return EXIT_FAILURE;
    }
    // Voltar ao início do ficheiro para futuras leituras
    lseek(fd, 0, SEEK_SET);

    // cada bloco a processar tem 4 bytes (tamanho de um inteiro)
    // o número total de blocos a processar é o tamanho do ficheiro total em bytes dividido por 4
    if(filesize % 4 != 0){
        // Se o tamanho do ficheiro não for múltiplo de 4, adicionar um bloco extra para processar esses bytes restantes
        total_blocks = (filesize / 4) + 1;
    }else{
        total_blocks = filesize / 4;
    }
    // O número de blocos a processar por cada trabalhador (processo filho) é o número total de blocos dividido pelo número de processos a criar (np)
    blocks_per_worker = total_blocks / np;

    // Criar um ficheiro temporário para armazenar os hashes locais calculados por cada trabalhador
    int hash_fd = open("hash.tmp", O_CREAT | O_TRUNC | O_RDWR, 0600);
    if (hash_fd < 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    // Distribuir o trabalho entre os processos filhos (trabalhadores)
    // Logica: O processo pai cria np processos filhos, e cada processo filho é responsável por processar um segmento do ficheiro correspondente a blocks_per_worker blocos (4 bytes cada). 
    // O processo pai deve esperar que todos os processos filhos terminem antes de encerrar o programa.
    for (int i = 0; i < np; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            // Erro ao criar processo filho
            close(fd);
            return EXIT_FAILURE;
        } else if (pid == 0) {
            // Código do processo filho

            off_t start_block = i * blocks_per_worker; // O bloco inicial a processar para este trabalhador é o índice do processo multiplicado pelo número de blocos por trabalhador
            off_t start_offset = start_block * 4; // O deslocamento inicial em bytes para este trabalhador é o bloco inicial multiplicado por 4 (tamanho de cada bloco)
            
            off_t my_blocks; // O número de blocos a processar por este trabalhador
            //uint32 tem exactamente 4 bytes, o que corresponde ao tamanho de cada bloco a processar.
            uint32_t local_hash = 0; // Variável para armazenar o hash local calculado por este trabalhador
            // local_hash é inicializado a zero (0 XOR X = X)
            uint32_t block; // bloco de 4 bytes a processar
            ssize_t bytes_read; // guarda o número de bytes lidos em cada operação de leitura
            // 4 é um bloco normal, 1,2 ou 3 é o ultimo bloco a processar
            int local_fd = open(filename, O_RDONLY);

            if (local_fd < 0) {
                exit(EXIT_FAILURE);
            }



            if (start_block >= total_blocks) {
                // Este trabalhador não tem blocos a processar
                my_blocks = 0;
            } else if (i == np - 1) {
                // O último trabalhador processa todos os blocos restantes
                my_blocks = total_blocks - start_block;
            } else {
                my_blocks = blocks_per_worker;
                // Garantir que não ultrapassa o total de blocos
                if (start_block + my_blocks > total_blocks) {
                    my_blocks = total_blocks - start_block;
                }
            }

            // Posicionar o descritor do ficheiro no início do segmento a processar por este trabalhador
            if (lseek(local_fd, start_offset, SEEK_SET) < 0) {
                exit(EXIT_FAILURE);
            }
            // Processar os blocos atribuídos a este trabalhador
            for (off_t j = 0; j < my_blocks; j++) {
                unsigned char buf[4];
                block = 0;  // Inicializar o bloco a zero para garantir que os bytes não lidos sejam considerados como zero no cálculo do hash

                bytes_read = read(local_fd, buf, 4);
                if (bytes_read < 0) {
                    exit(EXIT_FAILURE);
                }


                for (int k = 0; k < bytes_read; k++) {
                    block |= ((uint32_t)buf[k]) << (8 * (3 - k));
                }

                local_hash ^= block;
            }



            off_t hash_offset = i * sizeof(uint32_t);

            if (lseek(hash_fd, hash_offset, SEEK_SET) < 0) {
                exit(EXIT_FAILURE);
            }

            if (write(hash_fd, &local_hash, sizeof(uint32_t)) != sizeof(uint32_t)) {
                exit(EXIT_FAILURE);
            }
            close(local_fd);
            exit(0); // O processo filho deve encerrar após completar seu trabalho
        }
       
    }

    for (int i = 0; i < np; i++) {
        wait(NULL);
    }


    uint32_t final_hash = 0;
    uint32_t tmp;

    lseek(hash_fd, 0, SEEK_SET);

    for (int i = 0; i < np; i++) {
        if (read(hash_fd, &tmp, sizeof(uint32_t)) != sizeof(uint32_t)) {
            close(hash_fd);
            unlink("hash.tmp");
            return EXIT_FAILURE;
        }
        final_hash ^= tmp;
    }


    printf("hash4= %04X %04X\n",
        (final_hash >> 16) & 0xFFFF,
        final_hash & 0xFFFF);

    
    close(fd);
    close(hash_fd);
    unlink("hash.tmp");

	return 0;
}

// EOF