#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

int numeroCadeiras = 5; //4 na sala de espera + 1 para corte
int clientesAtuais = 0;
int clientesTotais = 25;
int tempoDeCorte = 0;

void *cliente();
void corteDeCabelo();
void *balk(int);
void *barbeiro();
void cortandoCabelo();

//semaforos
sem_t sem_cliente;     //Cliente querendo cortar cabelo
sem_t sem_mutex;       //Controla o fluxo de clientes na barbearia
sem_t sem_barbeiro;    //Define se o barbeiro está ocupado ou n
sem_t sem_fimCliente;  //Cliente terminou de cortar o cabelo
sem_t sem_fimBarbeiro; //Barbeiro terminou de cortar cabelo
sem_t sem_barbeiroDormindo; // Barbeiro dormindo quando não há clientes

int corteConcluido = 0; // quantos cortes o barbeiro fez no dia

int main()
{
    pthread_t barbeiro_id;
    pthread_t cliente_id[clientesTotais];
    int cliente_num[clientesTotais];

    sem_init(&sem_cliente, 0, 0);     // Semaforo do cliente iniciando em 0
    sem_init(&sem_mutex, 0, 1);       // Semaforo mutex iniciando em 1
    sem_init(&sem_barbeiro, 0, 0);    // Semaforo do barbeiro iniciando em 0
    sem_init(&sem_fimCliente, 0, 0);  // Semaforo que indica fim de corte para o cliente inciando em 0
    sem_init(&sem_fimBarbeiro, 0, 0); // Semaforo que indica fim de corte para o barbeiro iniciando em 0
    sem_init(&sem_barbeiroDormindo, 0, 0); //

    //Iniciando a thread barbeiro
    pthread_create(&barbeiro_id, NULL, *barbeiro, NULL);

    //Iniciando todas as threads de clientes
    for (int i = 0; i < clientesTotais; i++)
    {
        pthread_create(&cliente_id[i], NULL, cliente, (void *)&i);
        int rNum = rand() % 3 + 1; //número aleatório para que as threads sejam geradas de tempo em tempo simulando os clientes chegando aleatoriamente
        sleep(rNum);
    }

    printf("\nFim dos clientes.\n");
    while (clientesAtuais > 0); // faz a main rodar até o fim dos clientes para que o barbeiro acabe o corte de todos
        
    printf("\t\t\t\tDia concluido, saldo do dia: R$ %d\n", corteConcluido * 10);
    return 0;
}

void *barbeiro()
{
    while (1)
    {   
        sem_wait(&sem_mutex);
        if(clientesAtuais == 0)
            printf("\t\t\t\t\t\tBarbeiro foi dormir\n");
        sem_post(&sem_mutex);

        // primeiro rendezvous com a função cliente
        sem_wait(&sem_cliente);
        sem_post(&sem_barbeiro);
        
        printf("\t\t\t\t\t\tBarbeiro chama o cliente para o corte\n");
        cortandoCabelo();
        corteConcluido++;

        // segundo rendezvou com a função cliente
        sem_wait(&sem_fimCliente);
        sem_post(&sem_fimBarbeiro);
    }
}

void *cliente(void *n)
{
    int num = *(int *)n;
    sem_wait(&sem_mutex);
    printf("\t\tCliente %d chegou!\n", num + 1);

    if (clientesAtuais == 0) {
        printf("\t\tCliente %d acorda o barbeiro\n", num + 1);
    }

    if (clientesAtuais == numeroCadeiras)
    {
        sem_post(&sem_mutex);
        //Se nao ha cadeiras disponiveis, sai do salao
        return balk(num);
    }
    printf("\t\tCliente %d acha uma cadeira livre\n", num + 1);
    clientesAtuais++;
    printf("\t\tCliente %d está esperando\n", num + 1);
    sem_post(&sem_mutex);
    // primeiro rendezvous com a função barbeiro
    sem_post(&sem_cliente);
    sem_wait(&sem_barbeiro);
    printf("\t\t\t\t\t\tCliente %d está cortando o cabelo\n", num + 1);
    tempoDeCorte = rand() % 5 + 1;
    corteDeCabelo();
    printf("\t\t\t\t\t\tCliente %d terminou de cortar o cabelo\n", num + 1);

    sem_wait(&sem_mutex);
    clientesAtuais--;
    sem_post(&sem_mutex);

    // segundo rendezvous com a função barbeiro
    sem_post(&sem_fimCliente);
    sem_wait(&sem_fimBarbeiro);

    return NULL;
}

// função para o cliente que não achou cadeira vazia sair do salão
void *balk(int num)
{
    printf("Cliente %d não achou cadeira vazia e saiu sem cortar o cabelo\n", num + 1);
    return NULL;
}

// funções para esperar um tempo simulando o corte de cabelo
void cortandoCabelo()
{
    sleep(tempoDeCorte);
}

void corteDeCabelo()
{
    sleep(tempoDeCorte);
}
