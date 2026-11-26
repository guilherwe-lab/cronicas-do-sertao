#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINHAS 6
#define COLUNAS 6
#define MAX_INVENTARIO 5
#define MAX_NOME 50

/* =========================================================================
   STRUCTS E STRUCTS ANINHADAS
   ========================================================================= */

// Struct aninhada 1: Coordenada no mapa
typedef struct {
    int linha;
    int coluna;
} Coordenada;

// Struct aninhada 2: Status vital do personagem
typedef struct {
    int vida;
    int sede;
    int fome;
    int pontos;
} Status;

// Struct para itens do inventário 
typedef struct {
    char nome[30];
    int quantidade;
    int curaVida;
    int aliviaSede;
    int aliviaFome;
} Item;

// Struct principal: Personagem 
typedef struct {
    char nome[MAX_NOME];
    Coordenada localizacao;        
    Status estado;                    
    Item mochila[MAX_INVENTARIO];    
    int totalItens;
} Personagem;

/* =========================================================================
    FUNÇÕES 
   ========================================================================= */

void exibirIntroducao(void);
void inicializarPersonagem(Personagem *jogador);
int carregarMapa(char mapa[LINHAS][COLUNAS], Personagem *jogador);
void exibirMapa(char mapa[LINHAS][COLUNAS], Personagem jogador);
void exibirStatusEInventario(Personagem jogador);
void adicionarItem(Personagem *jogador, const char *nome, int qtd, int vida, int sede, int fome);
void usarItem(Personagem *jogador);
void moverJogador(char mapa[LINHAS][COLUNAS], Personagem *jogador, char direcao);
void processarEventoCelula(char mapa[LINHAS][COLUNAS], Personagem *jogador, int novaLinha, int novaColuna, int *jogoAtivo);
void salvarProgresso(Personagem jogador, char mapa[LINHAS][COLUNAS]);
int carregarProgresso(Personagem *jogador, char mapa[LINHAS][COLUNAS]);
void registrarRanking(Personagem jogador);
void exibirRanking(void);
void loopPartida(Personagem *jogador, char mapa[LINHAS][COLUNAS]);

/* =========================================================================
   FUNÇÃO MAIN 
   ========================================================================= */

int main(void) {
    Personagem jogador;
    char mapa[LINHAS][COLUNAS];
    int opcao = 0;

    exibirIntroducao();

    do {
        printf("\n================ MENU PRINCIPAL ================\n");
        printf("1. Novo Jogo (Iniciar travessia no Sertao)\n");
        printf("2. Continuar Jogo Salvo (savegame.txt)\n");
        printf("3. Ver Ranking de Sobreviventes\n");
        printf("4. Sair\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            while (getchar() != '\n'); // Limpa buffer
            opcao = 0;
        }
        getchar(); // Consome o '\n'

        switch (opcao) {
            case 1:
                inicializarPersonagem(&jogador);
                if (carregarMapa(mapa, &jogador)) {
                    loopPartida(&jogador, mapa);
                }
                break;

            case 2:
                if (carregarProgresso(&jogador, mapa)) {
                    printf("\nSave carregado com sucesso! Bem-vindo de volta, %s.\n", jogador.nome);
                    loopPartida(&jogador, mapa);
                } else {
                    printf("\nNenhum arquivo 'savegame.txt' encontrado.\n");
                }
                break;

            case 3:
                exibirRanking();
                break;

            case 4:
                printf("\nOOOO QUINTURAAA MININO. Ate a Proxima!\n");
                break;

            default:
                printf("\nOpcao invalida! Tente novamente.\n");
        }
    } while (opcao != 4);

    return 0;
}

/* =========================================================================
   IMPLEMENTAÇÃO DAS FUNÇÕES DE LÓGICA, MAPA 
   ========================================================================= */

// Leitura do arquivo historia.txt usando fgets
void exibirIntroducao(void) {
    FILE *arquivo = fopen("historia.txt", "r");
    char linha[200];

    printf("\n");
    if (arquivo == NULL) {
        printf("=== CRONICAS DO SERTAO: A SOMBRA DO PAPA-FIGO ===\n");
        printf("Sobreviva a seca, colha milho e feijao e fuja do Papa-Figo!\n");
        return;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        printf("%s", linha);
    }
    printf("\n");
    fclose(arquivo);
}

// Configura os valores iniciais da struct Personagem e suas structs aninhadas
void inicializarPersonagem(Personagem *jogador) {
    printf("\nDigite o nome do seu personagem: ");
    fgets(jogador->nome, MAX_NOME, stdin);
    jogador->nome[strcspn(jogador->nome, "\n")] = '\0';

    if (strlen(jogador->nome) == 0) {
        strcpy(jogador->nome, "Severino");
    }

    // Acesso às structs aninhadas
    jogador->estado.vida = 100;
    jogador->estado.sede = 20;   // 0 = hidratado, 100 = desidratado
    jogador->estado.fome = 20;   // 0 = satisfeito, 100 = faminto
    jogador->estado.pontos = 0;

    jogador->localizacao.linha = 1;
    jogador->localizacao.coluna = 1;
    jogador->totalItens = 0;

    // Itens iniciais típicos do Nordeste
    adicionarItem(jogador, "Cuscuz com Ovo", 1, 20, 0, 35);
    adicionarItem(jogador, "Cuia d'Agua", 1, 5, 40, 0);
}

// Lê a matriz do mapa a partir de mapa.txt usando fgets
int carregarMapa(char mapa[LINHAS][COLUNAS], Personagem *jogador) {
    FILE *arquivo = fopen("mapa.txt", "r");
    char buffer[32];

    if (arquivo == NULL) {
        printf("\n[ERRO] Arquivo 'mapa.txt' nao encontrado na pasta do programa!\n");
        return 0;
    }

    for (int i = 0; i < LINHAS; i++) {
        if (fgets(buffer, sizeof(buffer), arquivo) != NULL) {
            for (int j = 0; j < COLUNAS; j++) {
                mapa[i][j] = buffer[j];
                if (mapa[i][j] == 'J') {
                    jogador->localizacao.linha = i;
                    jogador->localizacao.coluna = j;
                }
            }
        }
    }

    fclose(arquivo);
    return 1;
}

// Renderiza a matriz do mapa no terminal
void exibirMapa(char mapa[LINHAS][COLUNAS], Personagem jogador) {
    printf("\n--- MAPA DA CAATINGA ---\n");
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (i == jogador.localizacao.linha && j == jogador.localizacao.coluna) {
                printf("J ");
            } else {
                printf("%c ", mapa[i][j]);
            }
        }
        printf("\n");
    }
    printf("Legenda: [J] Voce | [A] Cacimba | [M] Milho | [F] Feijao | [C] Cangaceiro | [P] Papa-Figo | [S] Acude (Saida)\n");
}

// Exibe os dados das structs aninhadas e do array de structs
void exibirStatusEInventario(Personagem jogador) {
    printf("\n--------------------------------------------------\n");
    printf("Retirante: %s | Posicao: (%d, %d) | Pontos: %d\n",
           jogador.nome,
           jogador.localizacao.linha,
           jogador.localizacao.coluna,
           jogador.estado.pontos);
    printf("Vida: %d/100 | Sede: %d/100 | Fome: %d/100\n",
           jogador.estado.vida,
           jogador.estado.sede,
           jogador.estado.fome);
    printf("Mochila (%d/%d): ", jogador.totalItens, MAX_INVENTARIO);

    if (jogador.totalItens == 0) {
        printf("Vazia");
    } else {
        for (int i = 0; i < jogador.totalItens; i++) {
            printf("[%d] %s (x%d)  ", i + 1, jogador.mochila[i].nome, jogador.mochila[i].quantidade);
        }
    }
    printf("\n--------------------------------------------------\n");
}

// Adiciona ou empilha um item no array de structs mochila
void adicionarItem(Personagem *jogador, const char *nome, int qtd, int vida, int sede, int fome) {
    for (int i = 0; i < jogador->totalItens; i++) {
        if (strcmp(jogador->mochila[i].nome, nome) == 0) {
            jogador->mochila[i].quantidade += qtd;
            return;
        }
    }

    if (jogador->totalItens < MAX_INVENTARIO) {
        int idx = jogador->totalItens;
        strcpy(jogador->mochila[idx].nome, nome);
        jogador->mochila[idx].quantidade = qtd;
        jogador->mochila[idx].curaVida = vida;
        jogador->mochila[idx].aliviaSede = sede;
        jogador->mochila[idx].aliviaFome = fome;
        jogador->totalItens++;
    } else {
        printf("\nSua mochila de couro ta cheia! Nao foi possivel guardar %s.\n", nome);
    }
}

// Consome um item do inventário
void usarItem(Personagem *jogador) {
    int escolha;
    if (jogador->totalItens == 0) {
        printf("\nVoce nao tem NADA na mochila!\n");
        return;
    }

    printf("\nQual item deseja consumir (1 a %d, ou 0 para cancelar)? ", jogador->totalItens);
    if (scanf("%d", &escolha) != 1) {
        while (getchar() != '\n');
        return;
    }
    getchar();

    if (escolha < 1 || escolha > jogador->totalItens) {
        return;
    }

    int idx = escolha - 1;
    Item *item = &jogador->mochila[idx];

    jogador->estado.vida += item->curaVida;
    if (jogador->estado.vida > 100) jogador->estado.vida = 100;

    jogador->estado.sede -= item->aliviaSede;
    if (jogador->estado.sede < 0) jogador->estado.sede = 0;

    jogador->estado.fome -= item->aliviaFome;
    if (jogador->estado.fome < 0) jogador->estado.fome = 0;

    printf("\nVoce consumiu %s! Sentiu as forcas voltarem.\n", item->nome);

    item->quantidade--;
    if (item->quantidade <= 0) {
        for (int i = idx; i < jogador->totalItens - 1; i++) {
            jogador->mochila[i] = jogador->mochila[i + 1];
        }
        jogador->totalItens--;
    }
}

// Controla o laço de turnos da partida
void loopPartida(Personagem *jogador, char mapa[LINHAS][COLUNAS]) {
    char comando;
    int jogoAtivo = 1;

    while (jogoAtivo) {
        exibirMapa(mapa, *jogador);
        exibirStatusEInventario(*jogador);

        printf("Acoes: [W] Cima | [S] Baixo | [A] Esquerda | [D] Direita | [I] Usar Item | [G] Salvar | [Q] Sair\n");
        printf("Comando: ");
        scanf(" %c", &comando);
        getchar();

        if (comando >= 'a' && comando <= 'z') {
            comando = comando - 32; // Converte para maiúscula
        }

        if (comando == 'W' || comando == 'S' || comando == 'A' || comando == 'D') {
            moverJogador(mapa, jogador, comando);

            // Verifica desgaste natural do sol do sertão
            jogador->estado.sede += 6;
            jogador->estado.fome += 5;

            if (jogador->estado.sede >= 100 || jogador->estado.fome >= 100) {
                printf("\n[ALERTA] O sol castiga! Voce esta sofrendo dano por sede ou fome extrema!\n");
                jogador->estado.vida -= 15;
            }

            // Verifica eventos na nova posição
            processarEventoCelula(mapa, jogador, jogador->localizacao.linha, jogador->localizacao.coluna, &jogoAtivo);

            if (jogador->estado.vida <= 0 && jogoAtivo) {
                printf("\n=== FIM DE JOGO ===\n");
                printf("%s nao resistiu aos perigos da caatinga...\n", jogador->nome);
                registrarRanking(*jogador);
                jogoAtivo = 0;
            }

        } else if (comando == 'I') {
            usarItem(jogador);
        } else if (comando == 'G') {
            salvarProgresso(*jogador, mapa);
        } else if (comando == 'Q') {
            printf("\nSaindo da partida...\n");
            jogoAtivo = 0;
        } else {
            printf("\nComando desconhecido!\n");
        }
    }
}

// Atualiza coordenadas na matriz validando colisões
void moverJogador(char mapa[LINHAS][COLUNAS], Personagem *jogador, char direcao) {
    int novaLinha = jogador->localizacao.linha;
    int novaColuna = jogador->localizacao.coluna;

    if (direcao == 'W') novaLinha--;
    else if (direcao == 'S') novaLinha++;
    else if (direcao == 'A') novaColuna--;
    else if (direcao == 'D') novaColuna++;

    if (novaLinha < 0 || novaLinha >= LINHAS || novaColuna < 0 || novaColuna >= COLUNAS) {
        printf("\nVoce chegou ao limite do terreno!\n");
        return;
    }

    if (mapa[novaLinha][novaColuna] == '#') {
        printf("\nTem uma ceica de pau e pedras bloqueando a passagem!\n");
        return;
    }

    // Limpa a posição antiga caso fosse 'J'
    if (mapa[jogador->localizacao.linha][jogador->localizacao.coluna] == 'J') {
        mapa[jogador->localizacao.linha][jogador->localizacao.coluna] = '.';
    }

    jogador->localizacao.linha = novaLinha;
    jogador->localizacao.coluna = novaColuna;
}

// Regras de interação com elementos regionais no mapa
void processarEventoCelula(char mapa[LINHAS][COLUNAS], Personagem *jogador, int linha, int coluna, int *jogoAtivo) {
    char celula = mapa[linha][coluna];

    switch (celula) {
        case 'A':
            printf("\n[CACIMBA] Voce encontrou agua fresca na pedra! Sede reduzida.\n");
            jogador->estado.sede -= 45;
            if (jogador->estado.sede < 0) jogador->estado.sede = 0;
            jogador->estado.pontos += 20;
            break;

        case 'M':
            printf("\n[ROCA DE MILHO] Voce colheu espigas de milho! (+50 pts)\n");
            adicionarItem(jogador, "Pamonha / Milho Assado", 1, 15, 0, 30);
            jogador->estado.pontos += 50;
            mapa[linha][coluna] = '.';
            break;

        case 'F':
            printf("\n[PLANTACAO DE FEIJAO] Voce colheu feijao macassar! (+50 pts)\n");
            adicionarItem(jogador, "Baiao de Dois", 1, 20, 0, 40);
            jogador->estado.pontos += 50;
            mapa[linha][coluna] = '.';
            break;

        case 'C':
            printf("\n[CANGACEIRO] Um cangaceiro do bando de Lampiao lhe entrega rapadura e uma peixeira!\n");
            adicionarItem(jogador, "Rapadura do Sertao", 2, 15, 0, 25);
            jogador->estado.pontos += 80;
            mapa[linha][coluna] = '.';
            break;

        case 'P':
            printf("\n[PERIGO - PAPA-FIGO!] A figura sombria do Papa-Figo surge entre os mandacarus!\n");
            printf("Voce luta para escapar e perde 40 de vida!\n");
            jogador->estado.vida -= 40;
            mapa[linha][coluna] = '.';
            break;

        case 'S':
            printf("\n======================================================\n");
            printf("VITORIA! %s alcancou o Acude Grande em seguranca!\n", jogador->nome);
            jogador->estado.pontos += 200 + jogador->estado.vida;
            printf("Pontuacao Final: %d pontos!\n", jogador->estado.pontos);
            printf("======================================================\n");
            registrarRanking(*jogador);
            *jogoAtivo = 0;
            break;

        default:
            break;
    }
}

// Persistência 2A: Salva o estado completo em savegame.txt usando fprintf
void salvarProgresso(Personagem jogador, char mapa[LINHAS][COLUNAS]) {
    FILE *arquivo = fopen("savegame.txt", "w");
    if (arquivo == NULL) {
        printf("\n[ERRO] Nao foi possivel criar 'savegame.txt'.\n");
        return;
    }

    fprintf(arquivo, "%s\n", jogador.nome);
    fprintf(arquivo, "%d %d\n", jogador.localizacao.linha, jogador.localizacao.coluna);
    fprintf(arquivo, "%d %d %d %d\n",
            jogador.estado.vida,
            jogador.estado.sede,
            jogador.estado.fome,
            jogador.estado.pontos);
    fprintf(arquivo, "%d\n", jogador.totalItens);

    for (int i = 0; i < jogador.totalItens; i++) {
        fprintf(arquivo, "%s;%d;%d;%d;%d\n",
                jogador.mochila[i].nome,
                jogador.mochila[i].quantidade,
                jogador.mochila[i].curaVida,
                jogador.mochila[i].aliviaSede,
                jogador.mochila[i].aliviaFome);
    }

    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            fprintf(arquivo, "%c", mapa[i][j]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    printf("\n[SAVE] Jogo salvo com sucesso em 'savegame.txt'!\n");
}

// Carrega o progresso salvo a partir de savegame.txt usando fscanf e fgets
int carregarProgresso(Personagem *jogador, char mapa[LINHAS][COLUNAS]) {
    FILE *arquivo = fopen("savegame.txt", "r");
    char linhaBuffer[128];

    if (arquivo == NULL) {
        return 0;
    }

    fgets(jogador->nome, MAX_NOME, arquivo);
    jogador->nome[strcspn(jogador->nome, "\n")] = '\0';

    fscanf(arquivo, "%d %d\n", &jogador->localizacao.linha, &jogador->localizacao.coluna);
    fscanf(arquivo, "%d %d %d %d\n",
           &jogador->estado.vida,
           &jogador->estado.sede,
           &jogador->estado.fome,
           &jogador->estado.pontos);
    fscanf(arquivo, "%d\n", &jogador->totalItens);

    for (int i = 0; i < jogador->totalItens; i++) {
        fgets(linhaBuffer, sizeof(linhaBuffer), arquivo);
        linhaBuffer[strcspn(linhaBuffer, "\n")] = '\0';

        char *token = strtok(linhaBuffer, ";");
        if (token) strcpy(jogador->mochila[i].nome, token);

        token = strtok(NULL, ";");
        if (token) jogador->mochila[i].quantidade = atoi(token);

        token = strtok(NULL, ";");
        if (token) jogador->mochila[i].curaVida = atoi(token);

        token = strtok(NULL, ";");
        if (token) jogador->mochila[i].aliviaSede = atoi(token);

        token = strtok(NULL, ";");
        if (token) jogador->mochila[i].aliviaFome = atoi(token);
    }

    for (int i = 0; i < LINHAS; i++) {
        if (fgets(linhaBuffer, sizeof(linhaBuffer), arquivo) != NULL) {
            for (int j = 0; j < COLUNAS; j++) {
                mapa[i][j] = linhaBuffer[j];
            }
        }
    }

    fclose(arquivo);
    return 1;
}

// Persistência 2B: Grava pontuação final em ranking.txt usando fprintf
void registrarRanking(Personagem jogador) {
    FILE *arquivo = fopen("ranking.txt", "a");
    if (arquivo == NULL) {
        return;
    }

    fprintf(arquivo, "Sobrevivente: %-20s | Pontuacao: %d pts\n",
            jogador.nome, jogador.estado.pontos);
    fclose(arquivo);
    printf("[RANKING] Sua pontuacao foi registrada em 'ranking.txt'.\n");
}

// Lê e exibe o ranking salvo
void exibirRanking(void) {
    FILE *arquivo = fopen("ranking.txt", "r");
    char linha[150];

    printf("\n=========== RANKING DO SERTAO ===========\n");
    if (arquivo == NULL) {
        printf("Nenhum registro encontrado ainda. Jogue uma partida!\n");
        return;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        printf("%s", linha);
    }
    printf("=========================================\n");
    fclose(arquivo);
}