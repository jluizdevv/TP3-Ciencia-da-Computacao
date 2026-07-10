#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct No {
    char palavra[100];
    char significado[255];
    struct No *esquerda;
    struct No *direita;
    int altura;
} No;

int tamanho_dicionario = 0;

int altura(No *N) {
    if (N == NULL)
        return 0;
    return N->altura;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

No* novo_no(char* palavra, char* significado) {
    No* no = (No*)malloc(sizeof(No));
    strcpy(no->palavra, palavra);
    strcpy(no->significado, significado);
    no->esquerda = NULL;
    no->direita = NULL;
    no->altura = 1;
    tamanho_dicionario++;
    return no;
}

No *rotacao_direita(No *y) {
    No *x = y->esquerda;
    No *T2 = x->direita;

    x->direita = y;
    y->esquerda = T2;

    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;
    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;

    return x;
}

No *rotacao_esquerda(No *x) {
    No *y = x->direita;
    No *T2 = y->esquerda;

    y->esquerda = x;
    x->direita = T2;

    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;
    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;

    return y;
}

int obter_balanceamento(No *N) {
    if (N == NULL)
        return 0;
    return altura(N->esquerda) - altura(N->direita);
}

No* inserir(No* no, char* palavra, char* significado) {
    if (no == NULL)
        return novo_no(palavra, significado);

    if (strcmp(palavra, no->palavra) < 0)
        no->esquerda = inserir(no->esquerda, palavra, significado);
    else if (strcmp(palavra, no->palavra) > 0)
        no->direita = inserir(no->direita, palavra, significado);
    else {
        strcpy(no->significado, significado);
        return no;
    }

    no->altura = 1 + max(altura(no->esquerda), altura(no->direita));
    int balanceamento = obter_balanceamento(no);

    if (balanceamento > 1 && strcmp(palavra, no->esquerda->palavra) < 0)
        return rotacao_direita(no);

    if (balanceamento < -1 && strcmp(palavra, no->direita->palavra) > 0)
        return rotacao_esquerda(no);

    if (balanceamento > 1 && strcmp(palavra, no->esquerda->palavra) > 0) {
        no->esquerda = rotacao_esquerda(no->esquerda);
        return rotacao_direita(no);
    }

    if (balanceamento < -1 && strcmp(palavra, no->direita->palavra) < 0) {
        no->direita = rotacao_direita(no->direita);
        return rotacao_esquerda(no);
    }

    return no;
}

No *no_valor_minimo(No *no) {
    No *atual = no;
    while (atual->esquerda != NULL)
        atual = atual->esquerda;
    return atual;
}

No* remover(No* raiz, char* palavra) {
    if (raiz == NULL)
        return raiz;

    if (strcmp(palavra, raiz->palavra) < 0)
        raiz->esquerda = remover(raiz->esquerda, palavra);
    else if (strcmp(palavra, raiz->palavra) > 0)
        raiz->direita = remover(raiz->direita, palavra);
    else {
        if ((raiz->esquerda == NULL) || (raiz->direita == NULL)) {
            No *temp = raiz->esquerda ? raiz->esquerda : raiz->direita;

            if (temp == NULL) {
                temp = raiz;
                raiz = NULL;
            } else
                *raiz = *temp;
            
            free(temp);
            tamanho_dicionario--;
        } else {
            No* temp = no_valor_minimo(raiz->direita);
            strcpy(raiz->palavra, temp->palavra);
            strcpy(raiz->significado, temp->significado);
            raiz->direita = remover(raiz->direita, temp->palavra);
        }
    }

    if (raiz == NULL)
        return raiz;

    raiz->altura = 1 + max(altura(raiz->esquerda), altura(raiz->direita));
    int balanceamento = obter_balanceamento(raiz);

    if (balanceamento > 1 && obter_balanceamento(raiz->esquerda) >= 0)
        return rotacao_direita(raiz);

    if (balanceamento > 1 && obter_balanceamento(raiz->esquerda) < 0) {
        raiz->esquerda = rotacao_esquerda(raiz->esquerda);
        return rotacao_direita(raiz);
    }

    if (balanceamento < -1 && obter_balanceamento(raiz->direita) <= 0)
        return rotacao_esquerda(raiz);

    if (balanceamento < -1 && obter_balanceamento(raiz->direita) > 0) {
        raiz->direita = rotacao_direita(raiz->direita);
        return rotacao_esquerda(raiz);
    }

    return raiz;
}

void buscar(No* raiz, char* palavra) {
    if (raiz == NULL) {
        printf("Palavra '%s' nao encontrada.\n", palavra);
        return;
    }
    if (strcmp(palavra, raiz->palavra) == 0) {
        printf("Significado de '%s': %s\n", raiz->palavra, raiz->significado);
        return;
    }
    if (strcmp(palavra, raiz->palavra) < 0)
        buscar(raiz->esquerda, palavra);
    else
        buscar(raiz->direita, palavra);
}

void listar_palavras(No* raiz) {
    if (raiz != NULL) {
        listar_palavras(raiz->esquerda);
        printf("%s: %s\n", raiz->palavra, raiz->significado);
        listar_palavras(raiz->direita);
    }
}

int informar_altura(No* raiz) {
    return altura(raiz);
}

int informar_tamanho() {
    return tamanho_dicionario;
}

int main() {
    No* raiz = NULL;

    raiz = inserir(raiz, "Binaria", "Tipo de arvore onde cada no tem ate dois filhos.");
    raiz = inserir(raiz, "AVL", "Arvore binaria de busca auto-balanceada.");
    raiz = inserir(raiz, "Node", "Elemento fundamental de uma estrutura de dados.");
    raiz = inserir(raiz, "C", "Linguagem de programacao de baixo e medio nivel.");

    printf("--- Dicionario ---\n");
    listar_palavras(raiz);

    printf("\n--- Buscando ---\n");
    buscar(raiz, "AVL");
    buscar(raiz, "Java");

    printf("\n--- Informacoes ---\n");
    printf("Altura da arvore: %d\n", informar_altura(raiz));
    printf("Total de verbetes: %d\n", informar_tamanho());

    printf("\n--- Removendo 'Node' ---\n");
    raiz = remover(raiz, "Node");

    printf("Altura apos remocao: %d\n", informar_altura(raiz));
    printf("Total apos remocao: %d\n", informar_tamanho());

    return 0;
}