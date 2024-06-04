#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resize_vector(int **ptr, int tamanho_atual){
    *ptr = realloc(*ptr, 2*tamanho_atual*sizeof(int));
}

int main(){

    


    return 0;
}