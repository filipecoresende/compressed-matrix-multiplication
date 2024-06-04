#ifndef PQ_HEAP_H
#define PQ_HEAP_H

typedef struct{
    int chave[2];
    int dado;
} t_item;

typedef struct{
    t_item* dados;
    int tam, n;
} PQ;

//Funcoes
PQ* pq_criar(int tam); 
void pq_destruir(PQ **p);
void pq_adicionar(PQ *p, t_item x);
t_item pq_extrai_maximo(PQ *p);
int pq_vazia(PQ* p); 




//extra
void imprimir_pq(PQ *p);
void destruir_pq(PQ **p);
#endif