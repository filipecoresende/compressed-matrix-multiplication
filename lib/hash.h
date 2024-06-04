#ifndef HASH_H
#define HASH_H

#include "lista_ligada.h"

//Dados
typedef struct{
    No **vetor;
    int M;
} Hash;

Hash* criar_hash(int M);
void destruir_hash(Hash **p);

void inserir_hash(Hash *p, int chave[2], int dado);
//void remover_hash(Hash *p, int chave[2]);

int buscar_hash(Hash *p, int chave[2]);//retorna -1 se nao encontrar
void imprimir_hash(Hash *p);

int incrementa_valor_hash(Hash *p, int chave[2]);
#endif