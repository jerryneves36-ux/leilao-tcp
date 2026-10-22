#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "leilao.h"

EstadoLeilao leilao;
int clientes_sockets[MAX_CLIENTES];
pthread_mutex_t mutex_clientes = PTHREAD_MUTEX_INITIALIZER;

void enviar_broadcast(const char *mensagem) {
    pthread_mutex_lock(&mutex_clientes);
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (clientes_sockets[i] != 0) {
            send(clientes_sockets[i], mensagem, strlen(mensagem), 0);
        }
    }
    pthread_mutex_unlock(&mutex_clientes);
}

void *thread_cronometro(void *arg) {
    (void)arg;
    while (1) {
        sleep(1);
        pthread_mutex_lock(&leilao.trava);
        if (!leilao.ativo) {
            pthread_mutex_unlock(&leilao.trava);
            break;
        }

        leilao.tempo_restante--;
        int rest = leilao.tempo_restante;

        if (rest <= 0) {
            leilao.ativo = 0;
            char msg_fim[TAM_MSG];
            snprintf(msg_fim, sizeof(msg_fim),
                     "\n======================================================\n"
                     "                LEILAO ENCERRADO!                     \n"
                     " Item: %s\n"
                     " Vencedor: %s\n"
                     " Valor Final: R$ %.2f (%d lances realizados)\n"
                     "======================================================\n",
                     leilao.item, leilao.maior_licitante, leilao.lance_atual, leilao.total_lances);
            pthread_mutex_unlock(&leilao.trava);

            printf("%s", msg_fim);
            enviar_broadcast(msg_fim);

            char log_fim[TAM_MSG];
            snprintf(log_fim, sizeof(log_fim), "FIM DO LEILAO | Vencedor: %s | Valor Final: R$ %.2f",
                     leilao.maior_licitante, leilao.lance_atual);
            registar_log(log_fim);
            break;
        }

        if (rest % 10 == 0 || rest <= 5) {
            char aviso[TAM_MSG];
            snprintf(aviso, sizeof(aviso), "[CRONOMETRO] Restam %d segundos! Maior lance: R$ %.2f (%s)\n",
                     rest, leilao.lance_atual, leilao.maior_licitante);
            pthread_mutex_unlock(&leilao.trava);
            printf("%s", aviso);
            enviar_broadcast(aviso);
        } else {
            pthread_mutex_unlock(&leilao.trava);
        }
    }
    return NULL;
}

void *atender_cliente(void *arg) {
    int sock = *(int *)arg;
    free(arg);
    char nome[TAM_NOME] = {0};
    char buffer[TAM_MSG];

    int n = recv(sock, nome, sizeof(nome) - 1, 0);
    if (n <= 0) {
        close(sock);
        return NULL;
    }
    nome[strcspn(nome, "\r\n")] = 0;

    printf("[CONEXAO] Participante '%s' entrou no leilao.\n", nome);
    char log_entrou[TAM_MSG];
    snprintf(log_entrou, sizeof(log_entrou), "CONEXAO | Cliente '%s' conectou-se.", nome);
    registar_log(log_entrou);

    pthread_mutex_lock(&leilao.trava);
    snprintf(buffer, sizeof(buffer),
             "\n--- BEM-VINDO AO LEILAO EM TEMPO REAL ---\n"
             "Item: %s | Lance Atual: R$ %.2f (%s) | Tempo: %ds\n"
             "Digite o valor do seu lance (ex: 150.00) e prima Enter:\n",
             leilao.item, leilao.lance_atual, leilao.maior_licitante, leilao.tempo_restante);
    pthread_mutex_unlock(&leilao.trava);
    send(sock, buffer, strlen(buffer), 0);

    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        double valor = atof(buffer);
        if (valor <= 0) {
            char erro[] = "[AVISO] Digite um valor numerico valido maior que zero.\n";
            send(sock, erro, strlen(erro), 0);
            continue;
        }

        char resposta[TAM_MSG];
        if (processar_lance(&leilao, nome, valor, resposta)) {
            printf("%s", resposta);
            enviar_broadcast(resposta);
        } else {
            send(sock, resposta, strlen(resposta), 0);
        }
    }

    printf("[DESCONEXAO] Participante '%s' saiu.\n", nome);
    pthread_mutex_lock(&mutex_clientes);
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (clientes_sockets[i] == sock) {
            clientes_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_clientes);
    close(sock);
    return NULL;
}

int main() {
    char ip_bind[64], item[TAM_NOME];
    int porta, duracao;
    double valor_inicial;

    printf("=== CONFIGURACAO DO SERVIDOR DE LEILAO ===\n");
    printf("IP do Servidor (ex: 192.168.0.11): ");
    scanf("%63s", ip_bind);
    printf("Porta TCP (ex: 8080): ");
    scanf("%d", &porta);
    getchar();
    printf("Nome do Item em Leilao: ");
    fgets(item, sizeof(item), stdin);
    item[strcspn(item, "\r\n")] = 0;
    printf("Valor Inicial (R$): ");
    scanf("%lf", &valor_inicial);
    printf("Tempo de duracao (segundos): ");
    scanf("%d", &duracao);

    inicializar_leilao(&leilao, item, valor_inicial, duracao);

    int servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(porta);
    endereco.sin_addr.s_addr = INADDR_ANY;

    if (bind(servidor_fd, (struct sockaddr *)&endereco, sizeof(endereco)) < 0) {
        perror("Erro no bind");
        return 1;
    }

    listen(servidor_fd, 10);
    printf("\n[SERVIDOR ATIVO] A aguardar licitantes em %s:%d...\n", ip_bind, porta);

    pthread_t tid_timer;
    pthread_create(&tid_timer, NULL, thread_cronometro, NULL);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int novo_sock = accept(servidor_fd, (struct sockaddr *)&cli_addr, &cli_len);
        if (novo_sock < 0) continue;

        pthread_mutex_lock(&mutex_clientes);
        for (int i = 0; i < MAX_CLIENTES; i++) {
            if (clientes_sockets[i] == 0) {
                clientes_sockets[i] = novo_sock;
                break;
            }
        }
        pthread_mutex_unlock(&mutex_clientes);

        int *pclient = malloc(sizeof(int));
        *pclient = novo_sock;
        pthread_t tid;
        pthread_create(&tid, NULL, atender_cliente, pclient);
        pthread_detach(tid);
    }
    return 0;
}
