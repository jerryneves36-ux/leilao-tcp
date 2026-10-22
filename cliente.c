#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TAM_MSG 512

int sock_global;

void *receber_mensagens(void *arg) {
    (void)arg;
    char buffer[TAM_MSG];
    int n;
    while ((n = recv(sock_global, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    printf("\n[SISTEMA] Conexao encerrada pelo servidor.\n");
    exit(0);
    return NULL;
}

int main() {
    char ip_servidor[64], nome[64], linha[128];
    int porta;

    printf("=== CLIENTE DO SISTEMA DE LEILAO ===\n");
    printf("IP do Servidor (ex: 192.168.0.11): ");
    scanf("%63s", ip_servidor);
    printf("Porta do Servidor (ex: 8080): ");
    scanf("%d", &porta);
    getchar();
    printf("A sua identificacao (Nome): ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\r\n")] = 0;

    sock_global = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(porta);
    inet_pton(AF_INET, ip_servidor, &serv_addr.sin_addr);

    if (connect(sock_global, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        return 1;
    }

    send(sock_global, nome, strlen(nome), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, receber_mensagens, NULL);

    while (fgets(linha, sizeof(linha), stdin) != NULL) {
        send(sock_global, linha, strlen(linha), 0);
    }

    close(sock_global);
    return 0;
}
