#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lista_ligada.h"



No* criar_lista(){
    return NULL;    
} 



void destruir_lista(No **L){
    No* q;
    while (*L != NULL){
        q = *L;
        *L = (*L)->prox;
        free(q);
    }
}

No* inserir_lista(No *L, int chave[2], int dado){
    No *q;
    q = (No*) malloc(sizeof(No));
    if (q==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    q->chave[0] = chave[0];
    q->chave[1] = chave[1];
    q->dado=dado;
    q->prox = L;
    L=q;
    return L;
}


int buscar_valor(No *L, int chave[2]){
    while (L!=NULL){
        if ((L->chave[0] == chave[0]) && (L->chave[1] == chave[1])) 
            return L->dado;
        L=L->prox;
    }
    return -1;
}

void imprimir_lista(No *L){
    while (L!=NULL){
        printf("(%d , %d): %d -> ", L->chave[0], L->chave[1], L->dado);
        L=L->prox;
    }
    printf("NULL\t");
}

void incrementa_valor_lista(No **L, int chave[2], int incremento){
    No *aux = *L;
    while (aux!=NULL){
        if ((aux->chave[0] == chave[0]) && (aux->chave[1] == chave[1])){
            aux->dado = aux->dado+incremento;
            return;
        } 
        aux = aux->prox;
    }
    No *q = (No*) malloc(sizeof(No));
    q->chave[0] = chave[0];
    q->chave[1] = chave[1];
    q->dado = incremento;
    q->prox = *L;
    *L = q;
}

void remover_elemento(No **L, int chave[2]){
    No* q = *L;
    if (q==NULL) return;
    if (q->chave[0]==chave[0] && q->chave[1]==chave[1]) {
        *L=q->prox;
        free(q);
        return;
    }
    while (q->prox!=NULL){
        if (q->prox->chave[0]==chave[0] && q->prox->chave[1]==chave[1])
            break;
        q=q->prox; 
    }
    if (q->prox==NULL) return;
    No* tmp = q->prox;
    q->prox = tmp->prox;
    free(tmp);
}