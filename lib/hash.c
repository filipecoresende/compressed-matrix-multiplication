#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "hash.h"

Hash* criar_hash(int M){
    Hash *p = (Hash*) malloc(sizeof(Hash));
    p->vetor = (No**) malloc(M*sizeof(No*));
    for (int i = 0; i < M; i++)
        p->vetor[i] = criar_lista();
    p->M = M;
    return p;
}

void destruir_hash(Hash **p){
    if (p==NULL || *p==NULL) return;
    for (int i = 0; i < (*p)->M; i++)
        destruir_lista(&((*p)->vetor[i]));
    free((*p)->vetor);
    free(*p);
    *p = NULL;   
}

/*
int hashing(char *chave, int M){
    int i, len = strlen(chave), hash_code = 0;
    for (i = 0; i < len; i++){
       hash_code = (hash_code*256+chave[i])%M;
    }
    return hash_code;
}
*/

int hashing(int chave[2], int M){
    unsigned int hash_value = (chave[0] * 31 + chave[1]) % M;
    return (hash_value + M) % M;
}




void inserir_hash(Hash *p, int chave[2], int dado){
    int n = hashing(chave, p->M);
    p->vetor[n] = inserir_lista(p->vetor[n], chave, dado);
}

/*
void remover_hash(Hash *p, int chave){
    int n = hashing(chave, p->M);
    p->vetor[n] = remover_lista(p->vetor[n], chave);
}
*/

int buscar_hash(Hash *p, int chave[2]){
    int n = hashing(chave, p->M);
    return buscar_valor(p->vetor[n],chave);
}

void imprimir_hash(Hash *p){
    for (int i = 0; i < p->M; i++)
    {
        printf("%d: ",i);
        imprimir_lista(p->vetor[i]);
        printf("\n");
    }
}

int incrementa_valor_hash(Hash *p, int chave[2]){
    int n = hashing(chave, p->M);
    return incrementa_valor_lista(&p->vetor[n], chave);
}

