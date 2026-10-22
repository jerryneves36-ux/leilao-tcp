#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "leilao.h"

void registar_log(const char *evento) {
    FILE *f = fopen("historico_leilao.log", "a");
    if (!f) return;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    fprintf(f, "[%02d/%02d/%04d %02d:%02d:%02d] %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec, evento);
    fclose(f);
}

void inicializar_leilao(EstadoLeilao *leilao, const char *item, double valor_inicial, int duracao) {
    strncpy(leilao->item, item, TAM_NOME - 1);
    leilao->item[TAM_NOME - 1] = '\0';
    leilao->lance_atual = valor_inicial;
    strcpy(leilao->maior_licitante, "Nenhum (Valor Inicial)");
    leilao->tempo_restante = duracao;
    leilao->ativo = 1;
    leilao->total_lances = 0;
    pthread_mutex_init(&leilao->trava, NULL);

    char msg_log[TAM_MSG];
    snprintf(msg_log, sizeof(msg_log), "INICIO DO LEILAO | Item: %s | Valor Inicial: R$ %.2f | Duracao: %ds",
             leilao->item, leilao->lance_atual, leilao->tempo_restante);
    registar_log(msg_log);
}

int processar_lance(EstadoLeilao *leilao, const char *cliente, double valor, char *resposta) {
    pthread_mutex_lock(&leilao->trava);

    if (!leilao->ativo || leilao->tempo_restante <= 0) {
        snprintf(resposta, TAM_MSG, "[ERRO] O leilao ja foi encerrado!\n");
        pthread_mutex_unlock(&leilao->trava);
        return 0;
    }

    if (valor <= leilao->lance_atual) {
        snprintf(resposta, TAM_MSG, "[RECUSADO] O seu lance (R$ %.2f) deve ser maior que o atual (R$ %.2f).\n",
                 valor, leilao->lance_atual);
        char msg_log[TAM_MSG];
        snprintf(msg_log, sizeof(msg_log), "LANCE RECUSADO | Cliente: %s | Tentou: R$ %.2f (Atual: R$ %.2f)",
                 cliente, valor, leilao->lance_atual);
        registar_log(msg_log);
        pthread_mutex_unlock(&leilao->trava);
        return 0;
    }

    leilao->lance_atual = valor;
    strncpy(leilao->maior_licitante, cliente, TAM_NOME - 1);
    leilao->maior_licitante[TAM_NOME - 1] = '\0';
    leilao->total_lances++;

    int teve_overtime = 0;
    if (leilao->tempo_restante < TEMPO_OVERTIME) {
        leilao->tempo_restante = TEMPO_OVERTIME;
        teve_overtime = 1;
    }

    if (teve_overtime) {
        snprintf(resposta, TAM_MSG,
                 "\n>>> [NOVO LANCE + OVERTIME!] %s ofereceu R$ %.2f pelo item '%s'! Tempo renovado para %ds! <<<\n",
                 cliente, valor, leilao->item, leilao->tempo_restante);
    } else {
        snprintf(resposta, TAM_MSG,
                 "\n>>> [NOVO LANCE] %s ofereceu R$ %.2f pelo item '%s'! (Tempo restante: %ds) <<<\n",
                 cliente, valor, leilao->item, leilao->tempo_restante);
    }

    char msg_log[TAM_MSG];
    snprintf(msg_log, sizeof(msg_log), "LANCE ACEITO | Cliente: %s | Valor: R$ %.2f | Restante: %ds%s",
             cliente, valor, leilao->tempo_restante, teve_overtime ? " [OVERTIME]" : "");
    registar_log(msg_log);

    pthread_mutex_unlock(&leilao->trava);
    return 1;
}
