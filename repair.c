#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/hash.h"

typedef struct { 
    int A, B;   
} Pair; 

typedef struct { 
    Pair pair;
    int count;   
} PairCount;

typedef struct {
    int size;
    int *ptr;
} expansion;

int* readFileAndConvertToIntArray(const char *filename, int *arraySize);
void writeArrayToFile(expansion compressedString, const char *filename);

void swapPointers(int **ptr1, int **ptr2); 
void resizeArray(int **ptr, int currentSize);

PairCount recordPairs(int *array, int arraySize, Hash *h); //retorna par mais frequente
int updateString(int *array, int arraySize, int *aux, Pair rule, int nonTerminal);
expansion* expander(Pair *grammar, int grammarSize);
expansion expandRule(expansion *expansions, Pair rule);
expansion decompressor(int *compressedString, int arraySize, Pair *grammar, int grammarSize);

void printCompressedString(expansion compressedString);
void printGrammar(Pair *grammar, int grammarSize);

int main(){
    int size_V;
    int *V = readFileAndConvertToIntArray("input.txt", &size_V);
    if (V == NULL) {
        return 1;  // Error occurred
    }

    int *aux = (int*) malloc(size_V*sizeof(int)); //vai ser usado para auxiliar a atualizar V a cada iteração
    Pair *grammar = (Pair*) malloc(size_V*sizeof(Pair));
    
    int nonTerminal = -1; //guarda o "nome" do símbolo não-terminal. 
    int numRules = 0; //numero de regras da gramatica
    Hash *h1 = criar_hash(100);
    PairCount pair_max = recordPairs(V, size_V, h1);
    destruir_hash(&h1);

    while(pair_max.count > 1){
        grammar[numRules] = pair_max.pair;
        numRules++;
        size_V = updateString(V, size_V, aux, pair_max.pair, nonTerminal);
        swapPointers(&V, &aux);
        nonTerminal--;
        h1 = criar_hash(100);
        pair_max = recordPairs(V, size_V, h1);
        destruir_hash(&h1);
    }
    free(aux);

/*
    //Quando a string de entrada não possui 'N', numerais ou caracteres especiais (como \n), fica legal imprimir:
    expansion compressedString;
    compressedString.size = size_V;
    compressedString.ptr = V;

    printCompressedString(compressedString);
    printGrammar(grammar, numRules);
*/

    expansion decompressedString = decompressor(V,size_V,grammar,numRules);
    free(V);
    free(grammar);

    writeArrayToFile(decompressedString, "decompressed_string.txt");
    
    free(decompressedString.ptr);



    return 0;
}

// Function to read file and convert its contents to an integer array
int* readFileAndConvertToIntArray(const char *filename, int *arraySize) {
    FILE *file;
    char *buffer;
    int *intArray;
    int fileLength;
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

void writeArrayToFile(expansion compressedString, const char *filename) {
    // Abre o arquivo para escrita
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    // Escreve os elementos do vetor no arquivo
    for (int i = 0; i < compressedString.size; i++) {
        fprintf(file, "%c", (char) compressedString.ptr[i]);
    }

    // Fecha o arquivo
    fclose(file);
}

void swapPointers(int **ptr1, int **ptr2){
    int *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

void resizeArray(int **ptr, int currentSize){
    *ptr = realloc(*ptr, 2*currentSize*sizeof(int));
}


PairCount recordPairs(int *array, int arraySize, Hash *h){
    int pair[2];
    int pairRepetitions;

    PairCount pair_max = {0,0,0};
    for (int i = 0; i < arraySize-1; i++){
        pair[0] = array[i]; pair[1] = array[i+1];
        pairRepetitions = incrementa_valor_hash(h, pair);
        if (pairRepetitions > pair_max.count){
            pair_max.pair.A = pair[0]; pair_max.pair.B = pair[1];
            pair_max.count = pairRepetitions;
        }
    }
    return pair_max;
}

int updateString(int *array, int arraySize, int *aux, Pair rule, int nonTerminal){
    int indexV = 0; 
    int indexAux = 0; 
    while ( indexV < arraySize-1){
        if ((array[indexV] == rule.A) && (array[indexV+1] == rule.B)){
            aux[indexAux++] = nonTerminal;
            indexV = indexV + 2;
        }
        else aux[indexAux++] = array[indexV++];
    }
    
    if (indexV==arraySize-1){ //o último inteiro pode ter ficado de fora.
        aux[indexAux] = array[indexV];
        indexAux++;
    } 

    return indexAux;//novo tamanho
}



expansion* expander(Pair *grammar, int grammarSize){
    expansion *expansions = (expansion*) malloc(grammarSize*sizeof(expansion));
    for (int i = 0; i < grammarSize; i++){
        expansions[i] = expandRule(expansions,grammar[i]);
    }
    return expansions;
}

expansion expandRule(expansion *expansions, Pair rule){
    int index;
    int expansionSizeA = 1;
    if (rule.A < 0){
        index = -rule.A-1;
        expansionSizeA=expansions[index].size;
    }

    int expansionSizeB = 1;
    if (rule.B < 0){
        index = -rule.B-1;
        expansionSizeB=expansions[index].size;
    }

    int size_aux = expansionSizeA + expansionSizeB;

    int *aux = (int*) malloc(size_aux*sizeof(int));

    if (rule.A<0){
        index = -rule.A-1;
        for (int i = 0; i < expansionSizeA; i++){
            aux[i] = expansions[index].ptr[i];
        }  
    }
    else{
        aux[0] = rule.A;
    } 
    

    if (rule.B<0){
        index = -rule.B-1;
        for (int i = 0; i < expansionSizeB; i++){
            aux[expansionSizeA+i] = expansions[index].ptr[i];
        } 
    }
    else{
        aux[expansionSizeA] = rule.B;
    } 
    expansion exp;
    exp.size = size_aux;
    exp.ptr = aux;
    return exp;
}


expansion decompressor(int *compressedString, int arraySize, Pair *grammar, int grammarSize){
    expansion *expansions = expander(grammar, grammarSize);
    int size_aux = arraySize; 
    int *aux = (int*) malloc(size_aux*sizeof(int)); //o tamanho pode ser futuramente aumentado com realloc
    int i,j=0;
    int grammarIndex, expansionSize;
    for (i = 0; i < arraySize; i++){
        if (compressedString[i]<0){
            grammarIndex = -compressedString[i]-1;
            expansionSize = expansions[grammarIndex].size;
            for (int k = 0; k < expansionSize; k++){
                if (j==size_aux){
                    size_aux*=2;
                    resizeArray(&aux,size_aux);
                }
                aux[j++] = expansions[grammarIndex].ptr[k];
            }
            
        }
        else {
            if (j==size_aux){
                size_aux*=2;
                resizeArray(&aux,size_aux);
            }
            aux[j++] = compressedString[i];
        }
    }
    free(expansions);
    expansion exp;
    //Talvez eu possa usar um resizeArray(&aux, j) aqui...
    exp.size = j; exp.ptr=aux;
    return exp;
}


void printCompressedString(expansion compressedString){
    int size = compressedString.size;
    int *V = compressedString.ptr;
    printf("String comprimida: ");
    for (int i = 0; i < size; i++)
    if (V[i] < 0){
        printf("N%d",-V[i]);
    }
    else printf("%c",(char) V[i]);
    printf("\n");
}

void printGrammar(Pair *grammar, int grammarSize){
    for (int i = 0; i < grammarSize; i++){
        if (grammar[i].A < 0){
            printf("N%d -> N%d", i+1, -grammar[i].A);
        }
        else printf("N%d -> %c", i+1, (char) grammar[i].A);
        if (grammar[i].B < 0){
            printf("N%d\n", -grammar[i].B);
        }
        else printf("%c\n", (char) grammar[i].B);
    }
}