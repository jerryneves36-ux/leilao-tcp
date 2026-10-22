#ifndef LEILAO_H
#define LEILAO_H

#include <pthread.h>

#define MAX_CLIENTES 20
#define TAM_NOME 64
#define TAM_MSG 512
#define TEMPO_OVERTIME 15

typedef struct {
    char item[TAM_NOME];
    double lance_atual;
    char maior_licitante[TAM_NOME];
    int tempo_restante;
    int ativo;
    int total_lances;
    pthread_mutex_t trava;
} EstadoLeilao;

void inicializar_leilao(EstadoLeilao *leilao, const char *item, double valor_inicial, int duracao);
void registar_log(const char *evento);
int processar_lance(EstadoLeilao *leilao, const char *cliente, double valor, char *resposta);

#endif
