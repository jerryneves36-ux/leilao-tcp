# Sistema de Leilão Virtual em Tempo Real (TCP Sockets em C)

Projeto prático desenvolvido para a disciplina de **Redes de Computadores I**, implementando um sistema distribuído de leilão eletrónico em tempo real utilizando **Sockets TCP** e programação concorrente com **POSIX Threads (`pthread`)** em linguagem C.

## 1. Arquitetura e Infraestrutura de Rede
O ambiente de testes foi construído sobre duas Máquinas Virtuais no **Oracle VirtualBox** executando **Debian GNU/Linux 13 (Trixie) 64-bit**, conectadas em rede local através do modo **Placa em Modo Ponte (Bridged Adapter)**:

* **VM Servidor (`servidor`):** IP `192.168.0.11` | 2 CPUs | 1024 MB RAM
* **VM Cliente (`cliente`):** IP `192.168.0.13` | 2 CPUs | 1024 MB RAM

## 2. Requisitos Técnicos Atendidos
* **Comunicação via Sockets TCP (`SOCK_STREAM`):** Conexão persistente orientada à conexão entre múltiplos licitantes e o servidor central.
* **Concorrência (`pthread`):** O servidor cria uma thread dedicada (`atender_cliente`) para cada novo participante conectado, além de uma thread independente para o cronómetro regressivo (`thread_cronometro`). No cliente, uma thread dedicada (`receber_mensagens`) escuta os avisos do servidor em tempo real sem bloquear a digitação de lances (`fgets`).
* **Exclusão Mútua e Estado Compartilhado (`pthread_mutex_t`):** Proteção contra condições de corrida (*race conditions*) na leitura e atualização do maior lance e do tempo restante, bem como no envio de mensagens em *broadcast* para os sockets ativos.
* **Regra de Overtime (Prorrogação Automática):** Caso um lance válido seja registado quando restam menos de 15 segundos no cronómetro, o tempo é automaticamente renovado para 15 segundos.
* **Persistência em Disco (`historico_leilao.log`):** Registo de auditoria com data e hora (*timestamp*) de início, conexões, lances aceites/recusados, ativações de *overtime* e encerramento com o vencedor.

## 3. Estrutura de Arquivos
* `leilao.h`: Definição da estrutura `EstadoLeilao`, constantes e protótipos das funções de negócio.
* `leilao.c`: Implementação da validação de lances, controlo de *overtime*, exclusão mútua e gravação do arquivo de log.
* `servidor.c`: Gerenciamento do socket TCP do servidor, *broadcast* para clientes conectados e threads de atendimento e temporizador.
* `cliente.c`: Interface interativa de linha de comando para conexão ao servidor e envio/recebimento simultâneo de eventos.

## 4. Como Compilar e Executar

### Compilação
Na máquina Servidor:
```bash
gcc servidor.c leilao.c -o servidor -lpthread
```

Na máquina Cliente:
```bash
gcc cliente.c -o cliente -lpthread
```

### Execução
1. Inicie o servidor (`./servidor`) informando o IP, a porta TCP, o nome do item, o lance mínimo inicial e a duração em segundos.
2. Em uma ou mais instâncias de cliente (`./cliente`), conecte-se informando o IP do servidor, a porta TCP e o nome do licitante.

## 5. Auditoria de Eventos
Para consultar o histórico persistido após ou durante os leilões no servidor (`historico_leilao.log`):
```bash
cat historico_leilao.log
```

## 6. Autores
* Jerry Adriany Farias Neves Dourado
* Jardell Rai Farias Neves Dourado
