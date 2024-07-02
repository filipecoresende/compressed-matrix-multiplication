#include <stdio.h>
#include <stdlib.h>
#include "pq_heap.h"

#define F_ESQ(i) (2*i+1) /*Filho esquerdo de i*/ 
#define F_DIR(i) (2*i+2) /*Filho direito de i*/ 
#define PAI(i) ((i-1)/2)

PQ* pq_criar(int tam){
    PQ *p = (PQ*) malloc(sizeof(PQ));
    p->dados = (pq_item*) malloc(tam*sizeof(pq_item));
    p->n = 0;
    p->tam = tam;
    return p;
}

int pq_vazia(PQ *p){ 
    return p->n == 0;
}

void swap(pq_item *ptr1, pq_item *ptr2){
    pq_item aux = *ptr1;
    *ptr1=*ptr2;
    *ptr2=aux;
}

void sobe_no_heap(PQ *p, int pos) {
    if (pos > 0 && p->dados[PAI(pos)].dado < p->dados[pos].dado){
        swap(&p->dados[pos], &p->dados[PAI(pos)]);
        sobe_no_heap(p, PAI(pos));
    } 
}

void pq_adicionar(PQ *p, pq_item item) { 
    if(p->n == p->tam) resizePriorityQueue(p);
    p->dados[p->n] = item;
    p->n++;
    sobe_no_heap(p, p->n - 1);
}

void desce_no_heap(PQ *p, int pos){
    if (F_ESQ(pos)<p->n){
        int maior_filho = F_ESQ(pos);
        if (F_DIR(pos) < p->n && 
            p->dados[F_ESQ(pos)].dado < p->dados[F_DIR(pos)].dado)
            maior_filho = F_DIR(pos);
        if (p->dados[pos].dado < p->dados[maior_filho].dado) {
            swap(&p->dados[pos], &p->dados[maior_filho]);
            desce_no_heap(p, maior_filho);
        }
    }
}

pq_item pq_extrai_maximo(PQ *p){
    if (p->n == 0){
        perror("Unable to extract the maximum");
        exit(EXIT_FAILURE);
    } 
    pq_item item = p->dados[0];
    swap(&p->dados[p->n-1],&p->dados[0]);
    p->n--;
    desce_no_heap(p,0);
    return item; 
}

//extra
void imprimir_pq(PQ *p){
    pq_item *aux = p->dados;
    
    for (int i = 0; i < p->n; i++){
        printf("%d:  (%d, %d):%d\n", i, aux[i].chave[0], aux[i].chave[1], aux[i].dado);
    }
}

void destruir_pq(PQ **p){
    free((*p)->dados);
    free(*p);
    *p = NULL;
}


void resizePriorityQueue(PQ *p){
    int newSize = 2*p->tam;
    p->tam = newSize;
    p->dados = (pq_item*) realloc(p->dados,newSize*sizeof(pq_item));
}