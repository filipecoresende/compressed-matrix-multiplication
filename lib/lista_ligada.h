#ifndef LISTA_LIGADA_H
#define LISTA_LIGADA_H

typedef struct no{ 
    int chave[2];
    int dado;
    struct no* prox;
} No; 

/* Funçoes Exportadas */
 
No* criar_lista();
void destruir_lista(No **L);


No* inserir_lista(No *L, int chave[2], int dado);


void imprimir_lista(No *L);

int buscar_valor(No *L, int chave[2]);

void incrementa_valor_lista(No **L, int chave[2], int incremento);

void remover_elemento(No **L, int chave[2]);

#endif