cat << 'EOF' > .gitignore
servidor
cliente
EOF

cat << 'EOF' > README.md
# Sistema de Leilão Virtual em Tempo Real (TCP Sockets em C)

Projeto prático desenvolvido para a disciplina de **Redes de Computadores I**, implementando um sistema distribuído de leilão eletrônico em tempo real utilizando **Sockets TCP** e programação concorrente com **POSIX Threads (`pthread`)** em linguagem C.

## 1. Arquitetura e Infraestrutura de Rede
O ambiente de testes foi construído sobre duas Máquinas Virtuais no **Oracle VirtualBox** executando **Debian GNU/Linux 13 (Trixie) 64-bit**, conectadas em rede local através do modo **Placa em Modo Ponte (Bridged Adapter)**:

* **VM Servidor (`servidor`):** IP `192.168.0.11` | 2 CPUs | 1024 MB RAM
* **VM Cliente (`cliente`):** IP `192.168.0.13` | 2 CPUs | 1024 MB RAM

## 2. Requisitos Técnicos Atendidos
* **Comunicação via Sockets TCP (`SOCK_STREAM`):** Conexão persistente orientada a conexão entre múltiplos licitantes e o servidor central.
* **Concorrência (`pthread`):** O servidor cria uma thread dedicada (`atender_cliente`) para cada novo participante conectado, além de uma thread independente para o cronômetro regressivo (`thread_cronometro`). No cliente, uma thread dedicada (`receber_mensagens`) escuta os avisos do servidor em tempo real sem bloquear a digitação de lances (`fgets`).
* **Exclusão Mútua e Estado Compartilhado (`pthread_mutex_t`):** Proteção contra condições de corrida (*race conditions*) na leitura e atualização do maior lance e do tempo restante, bem como no envio de mensagens em *broadcast* para os sockets ativos.
* **Regra de Overtime (Prorrogação Automática):** Caso um lance válido seja registrado quando restam menos de 15 segundos no cronômetro, o tempo é automaticamente renovado para 15 segundos.
* **Persistência em Disco (`historico_leilao.log`):** Registro de auditoria com data e hora (*timestamp*) de início, conexões, lances aceitos/recusados, ativações de *overtime* e encerramento com o vencedor.

## 3. Estrutura de Arquivos
* `leilao.h`: Definição da estrutura `EstadoLeilao`, constantes e protótipos das funções de negócio.
* `leilao.c`: Implementação da validação de lances, controle de *overtime*, exclusão mútua e gravação do arquivo de log.
* `servidor.c`: Gerenciamento do socket TCP do servidor, *broadcast* para clientes conectados e threads de atendimento e temporizador.
* `cliente.c`: Interface interativa de linha de comando para conexão ao servidor e envio/recebimento simultâneo de eventos.

## 4. Como Compilar e Executar

### Compilação
Na máquina Servidor:
```bash
gcc servidor.c leilao.c -o servidor -lpthread# Sistema de Leilão Virtual em Tempo Real (TCP Sockets em C)

Projeto prático desenvolvido para a disciplina de **Redes de Computadores I**, implementando um sistema distribuído de leilão eletrônico em tempo real utilizando **Sockets TCP** e programação concorrente com **POSIX Threads (`pthread`)** em linguagem C.

## 1. Arquitetura e Infraestrutura de Rede
O ambiente de testes foi construído sobre duas Máquinas Virtuais no **Oracle VirtualBox** executando **Debian GNU/Linux 13 (Trixie) 64-bit**, conectadas em rede local através do modo **Placa em Modo Ponte (Bridged Adapter)**:

* **VM Servidor (`servidor`):** IP `192.168.0.11` | 2 CPUs | 1024 MB RAM
* **VM Cliente (`cliente`):** IP `192.168.0.13` | 2 CPUs | 1024 MB RAM

## 2. Requisitos Técnicos Atendidos
* **Comunicação via Sockets TCP (`SOCK_STREAM`):** Conexão persistente orientada a conexão entre múltiplos licitantes e o servidor central.
* **Concorrência (`pthread`):** O servidor cria uma thread dedicada (`atender_cliente`) para cada novo participante conectado, além de uma thread independente para o cronômetro regressivo (`thread_cronometro`). No cliente, uma thread dedicada (`receber_mensagens`) escuta os avisos do servidor em tempo real sem bloquear a digitação de lances (`fgets`).
* **Exclusão Mútua e Estado Compartilhado (`pthread_mutex_t`):** Proteção contra condições de corrida (*race conditions*) na leitura e atualização do maior lance e do tempo restante, bem como no envio de mensagens em *broadcast* para os sockets ativos.
* **Regra de Overtime (Prorrogação Automática):** Caso um lance válido seja registrado quando restam menos de 15 segundos no cronômetro, o tempo é automaticamente renovado para 15 segundos.
* **Persistência em Disco (`historico_leilao.log`):** Registro de auditoria com data e hora (*timestamp*) de início, conexões, lances aceitos/recusados, ativações de *overtime* e encerramento com o vencedor.

## 3. Estrutura de Arquivos
* `leilao.h`: Definição da estrutura `EstadoLeilao`, constantes e protótipos das funções de negócio.
* `leilao.c`: Implementação da validação de lances, controle de *overtime*, exclusão mútua e gravação do arquivo de log.
* `servidor.c`: Gerenciamento do socket TCP do servidor, *broadcast* para clientes conectados e threads de atendimento e temporizador.
* `cliente.c`: Interface interativa de linha de comando para conexão ao servidor e envio/recebimento simultâneo de eventos.

## 4. Como Compilar e Executar

### Compilação
Na máquina Servidor:
```bash
gcc servidor.c leilao.c -o servidor -lpthread

