#ifndef PQ_HEAP_H
#define PQ_HEAP_H

typedef struct{
    int chave[2];
    int dado;
} pq_item;

typedef struct{
    pq_item* dados;
    int tam, n;
} PQ;

//Funcoes
PQ* pq_criar(int tam); 
void pq_adicionar(PQ *p, pq_item x);
pq_item pq_extrai_maximo(PQ *p);
int pq_vazia(PQ* p); 




//extra
void imprimir_pq(PQ *p);
void destruir_pq(PQ **p);
#endif