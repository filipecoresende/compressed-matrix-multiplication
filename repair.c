#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/hash.h"

typedef struct { 
    int A, B;   
} Par; 

typedef struct { 
    Par par;
    int qtde;   
} ParQuantidade;

typedef struct {
    int tam;
    int *ptr;
} expansao;

int* readFileAndConvertToIntArray(const char *filename, long *arraySize);
void escrever_vetor_no_arquivo(expansao var, const char *nome_arquivo);
ParQuantidade registra_pares(int *V, int tam_V, Hash *h); //retorna par mais frequente
int atualiza_cadeia(int *V, int tam_V, int *aux, Par regra, int nonterminal);
void swap_pointers(int **ptr1, int **ptr2); 
void resize_vector(int **ptr, int tamanho_atual);

expansao* expansor(Par *gramatica, int tam_gramatica);
expansao expandir_regra(expansao *expansoes, Par regra);
expansao descompressor(int *V, int tam_V, Par *gramatica, int tam_gramatica);

void imprimir_gramatica(Par gramatica[], int tam_gramatica);

int main(){
    long arraySize;
    // Call the function to read the file and convert to integer array
    int *V = readFileAndConvertToIntArray("entrada.txt", &arraySize);
    if (V == NULL) {
        return 1;  // Error occurred
    }

    int tam_V = (int) arraySize;
    int *aux = (int*) malloc(tam_V*sizeof(int)); //vai ser usado para auxiliar a atualizar V a cada iteração
    Par *gramatica = (Par*) malloc(tam_V*sizeof(Par));
    //Entrada: 1 17 28 15 0 11 13 24 10 0 1 17  13 24 0 16 28 15 0 11 13 24 0 1 17 13 24 20 0
    //Saída: -2  -6  10  -8  0  16  -6  -8  20  0
    
    int nonterminal = -1; //guarda o "nome" do símbolo não-terminal. 
    int i = 0; //numero de regras
    Hash *h1 = criar_hash(30);
    ParQuantidade par_max = registra_pares(V, tam_V, h1);
    destruir_hash(&h1);

    while(par_max.qtde > 1){
        gramatica[i] = par_max.par;
        i++;
        tam_V = atualiza_cadeia(V, tam_V, aux, par_max.par, nonterminal);
        swap_pointers(&V, &aux);
        nonterminal--;
        h1 = criar_hash(30);
        par_max = registra_pares(V, tam_V, h1);
        destruir_hash(&h1);
    }
    free(aux);

/*
   //imprimir resposta:
    printf("\n");
    printf("String comprimida: ");
    for (int i = 0; i < tam_V; i++)
    if (V[i] < 0){
        printf("N%d",-V[i]);
    }
    else printf("%c",(char) V[i]);
    printf("\n");
    
    imprimir_gramatica(gramatica, i);
*/

    expansao vetor_descomprimido = descompressor(V,tam_V,gramatica,i);
    free(V);
    free(gramatica);

    escrever_vetor_no_arquivo(vetor_descomprimido, "descompressao.txt");
    
    free(vetor_descomprimido.ptr);



    return 0;
}

ParQuantidade registra_pares(int *V, int tam_V, Hash *h){
    int par[2];
    int quantidade_par;

    ParQuantidade par_max = {0,0,0};
    for (int i = 0; i < tam_V-1; i++){
        par[0] = V[i]; par[1] = V[i+1];
        quantidade_par = incrementa_valor_hash(h, par);
        if (quantidade_par > par_max.qtde){
            par_max.par.A = par[0]; par_max.par.B = par[1];
            par_max.qtde = quantidade_par;
        }
    }
    return par_max;
}

int atualiza_cadeia(int *V, int tam_V, int *aux, Par regra, int nonterminal){
    int i = 0; //índice no vetor V
    int j = 0; //índice no vetor aux
    int novo_tamanho;
    while ( i < tam_V-1){
        if ((V[i] == regra.A) && (V[i+1] == regra.B)){
            aux[j] = nonterminal;
            j++;
            i = i + 2;
        }
        else {
            aux[j] = V[i];
            j++; i++;
        }
    }
    

    if (i==tam_V-1){ //o último inteiro pode ter ficado de fora.
        aux[j] = V[i];
        j++;
    } 
        
    novo_tamanho = j;
    return novo_tamanho;
    

}

void swap_pointers(int **ptr1, int **ptr2){
    int *aux = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = aux;
}

void imprimir_gramatica(Par *gramatica, int tam_gramatica){
    for (int i = 0; i < tam_gramatica; i++){
        if (gramatica[i].A < 0){
            printf("N%d -> N%d", i+1, -gramatica[i].A);
        }
        else printf("N%d -> %c", i+1, (char) gramatica[i].A);
        if (gramatica[i].B < 0){
            printf("N%d\n", -gramatica[i].B);
        }
        else printf("%c\n", (char) gramatica[i].B);
    }
}



expansao* expansor(Par *gramatica, int tam_gramatica){
    expansao *expansoes = (expansao*) malloc(tam_gramatica*sizeof(expansao));
    for (int i = 0; i < tam_gramatica; i++){
        expansoes[i] = expandir_regra(expansoes,gramatica[i]);
    }
    return expansoes;
}

expansao expandir_regra(expansao *expansoes, Par regra){
    int index;

    int tam_expansao_A = 1;
    if (regra.A < 0){
        index = -regra.A-1;
        tam_expansao_A=expansoes[index].tam;
    }

    int tam_expansao_B = 1;
    if (regra.B < 0){
        index = -regra.B-1;
        tam_expansao_B=expansoes[index].tam;
    }

    int tam_aux = tam_expansao_A + tam_expansao_B;

    int *aux = (int*) malloc(tam_aux*sizeof(int));

    if (regra.A<0){
        index = -regra.A-1;
        for (int i = 0; i < tam_expansao_A; i++){
            aux[i] = expansoes[index].ptr[i];
        }  
    }
    else{
        aux[0] = regra.A;
    } 
    

    if (regra.B<0){
        index = -regra.B-1;
        for (int i = 0; i < tam_expansao_B; i++){
            aux[tam_expansao_A+i] = expansoes[index].ptr[i];
        } 
    }
    else{
        aux[tam_expansao_A] = regra.B;
    } 
    expansao exp;
    exp.tam = tam_expansao_A + tam_expansao_B;
    exp.ptr = aux;
    return exp;
}

expansao descompressor(int *V, int tam_V, Par *gramatica, int tam_gramatica){
    expansao *expansoes = expansor(gramatica, tam_gramatica);
    int tam_aux = tam_V; //inicialmente alocarei tam_V pra aux, aumentarei conforme necessario
    int *aux = (int*) malloc(tam_aux*sizeof(int));
    int i,j=0;
    for (i = 0; i < tam_V; i++){
        if (V[i]<0){
            int index = -V[i]-1;
            int tam_expansao = expansoes[index].tam;
            for (int k = 0; k < tam_expansao; k++){
                if (j==tam_aux){
                    tam_aux*=2;
                    resize_vector(&aux,tam_aux);
                }
                aux[j] = expansoes[index].ptr[k];
                j++;
            }
            
        }
        else {
            if (j==tam_aux){
                tam_aux*=2;
                resize_vector(&aux,tam_aux);
            }
            aux[j] = V[i];
            j++;
        }
    }
    free(expansoes);
    expansao resposta;
    //Talvez eu possa usar um resize_vector(&aux, j) aqui...
    resposta.tam = j; resposta.ptr=aux;
    return resposta;
}

void resize_vector(int **ptr, int tamanho_atual){
    *ptr = realloc(*ptr, 2*tamanho_atual*sizeof(int));
}


// Function to read file and convert its contents to an integer array
int* readFileAndConvertToIntArray(const char *filename, long *arraySize) {
    FILE *file;
    char *buffer;
    int *intArray;
    long fileLength;
    size_t bytesRead, i;

    // Open the file for reading
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open %s for reading\n", filename);
        return NULL;
    }

    // Determine the file length
    fseek(file, 0, SEEK_END);
    fileLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer to store the string
    buffer = (char *)malloc((fileLength + 1) * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    bytesRead = fread(buffer, sizeof(char), fileLength, file);
    buffer[fileLength] = '\0';  // Null-terminate the string

    // Close the file
    fclose(file);

    if (bytesRead != fileLength) {
        fprintf(stderr, "Reading error\n");
        free(buffer);
        return NULL;
    }

    // Allocate memory for the integer array
    intArray = (int *)malloc(fileLength * sizeof(int));
    if (intArray == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer);
        return NULL;
    }

    // Cast characters to integers and store in the integer array
    for (i = 0; i < fileLength; ++i) {
        intArray[i] = (int)buffer[i];
    }

    // Free the buffer
    free(buffer);

    // Set the array size
    *arraySize = fileLength;

    // Return the integer array
    return intArray;
}

void escrever_vetor_no_arquivo(expansao var, const char *nome_arquivo) {
    // Abre o arquivo para escrita
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    // Escreve os elementos do vetor no arquivo
    for (int i = 0; i < var.tam; i++) {
        fprintf(arquivo, "%c", (char) var.ptr[i]);
    }

    // Fecha o arquivo
    fclose(arquivo);
}