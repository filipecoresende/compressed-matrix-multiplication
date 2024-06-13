#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/hash.h"
#include "lib/pq_heap.h"

typedef struct { 
    int A, B;   
} Pair; 

typedef struct {
    int size;
    int *ptr;
} expansion;

int* readFileAndConvertToIntArray(const char *filename, int *arraySize);
void writeArrayToFile(expansion compressedString, const char *filename);

void swapPointers(int **ptr1, int **ptr2); 
void resizeArray(int **ptr, int currentSize);

void recordPairsFirstIteration(int *array, int arraySize, Hash *h); //registra os pares na primeira vez
void recordPairs(int *array, int arraySize, Hash *h1, Hash *h2, int nonTerminal, Pair rule); //registra os pares nas demais
int updateString(int **array, int arraySize, int **aux, Pair rule, int nonTerminal); //produz a "string" com a nova regra e retorna seu novo tamanho
void insertPairsPQ(Hash *h, PQ *pq); //percorre h, adicionando cada par-frequência à fila pq
expansion* expander(Pair *grammar, int grammarSize); //expande cada símbolo não-terminal da gramática separadamente
expansion expandRule(expansion *expansions, Pair rule);
expansion decompressor(int *compressedString, int arraySize, Pair *grammar, int grammarSize); 

void printCompressedString(expansion compressedString);
void printGrammar(Pair *grammar, int grammarSize);

//TODO: argc, argv
int main(){
    int sizeV;
    int *V = readFileAndConvertToIntArray("input.txt", &sizeV);
    if (V == NULL) {
        return 1;  // Erro
    }
  
    //TODO: optarg
    //./repair -c (compress)
    //./repair -d (decompress)
    //
    //saida => input.re32 (compressed)

    //TASK 1: 
    //TODO: taxa de compressao = |arquivo comprimido|/|arquivo original|
    //
    //t1 => input.re32 guarda inteiros de 32 bits
    //
    //t2 => input.re16 guarda inteiros de 16 bits (se o maior couber em 16 bits)
    //
    //t3 => input.re8 ..
    //
    //t? => input.reiv guarda inteiros de tamanho V, em V representa o maior int necessário

    //TASK 2:
    //TODO: medir o tempo de execução compressão e descompressão

    //TASK 3:
    //TODO: medir o pico de memória utilizada durante a compressão e descompressão

    //V2: discutir a melhoria do Valina

    int *aux = (int*) malloc(sizeV*sizeof(int)); //vai ser usado para auxiliar a atualizar V a cada iteração
    Pair *grammar = (Pair*) malloc(sizeV*sizeof(Pair));
    
    PQ *pq = pq_criar(10000);//TODO: corrigir
    int nonTerminal = -1; //guarda o "nome" do símbolo não-terminal. 
    int numRules = 0; //numero de regras da gramatica
    Hash *h1 = criar_hash(10000);
    Hash *h2 = criar_hash(10000);
    recordPairsFirstIteration(V, sizeV, h1);//conta os pares na primeira iteração do RePair
    insertPairsPQ(h1, pq); //insere todos os pares numa fila de prioridade
    destruir_hash(&h1); 
    pq_item auxPair = pq_extrai_maximo(pq); 
    Pair maxPair;
    maxPair.A = auxPair.chave[0]; maxPair.B = auxPair.chave[1];
    int maxPairCount = auxPair.dado;
   
    while(maxPairCount > 1){
        grammar[numRules++] = maxPair;
        sizeV = updateString(&V, sizeV, &aux, maxPair, nonTerminal);
        h1 = criar_hash(100);
        recordPairs(V, sizeV, h1, h2, nonTerminal, maxPair);
        nonTerminal--;
        insertPairsPQ(h1, pq);
        destruir_hash(&h1);
        auxPair = pq_extrai_maximo(pq); //guarda o "candidato" a par mais frequente
        int busca = buscar_hash(h2, auxPair.chave);

        while (busca!=-1){ //se busca for -1, isto é, o maxPair não estiver em h2, então podemos prosseguir
            remover_elemento_hash(h2, auxPair.chave);
            auxPair.dado = auxPair.dado - busca;
            if (auxPair.dado > 0) pq_adicionar(pq, auxPair);
            auxPair = pq_extrai_maximo(pq);//tentando de novo
            busca = buscar_hash(h2, auxPair.chave);
        }

        maxPair.A = auxPair.chave[0];
        maxPair.B = auxPair.chave[1];
        maxPairCount = auxPair.dado;
    }
    free(aux);
    destruir_pq(&pq);
    destruir_hash(&h2);

/*
    //Quando a string de entrada não possui o caractere 'N', numerais ou caracteres especiais (como \n), fica legal imprimir:
    expansion compressedString;
    compressedString.size = sizeV;
    compressedString.ptr = V;

    printCompressedString(compressedString);
    printGrammar(grammar, numRules);
*/


    expansion decompressedString = decompressor(V,sizeV,grammar,numRules);
    free(V);
    free(grammar);

    writeArrayToFile(decompressedString, "decompressedString.txt");
    
    free(decompressedString.ptr);

    return 0;
}

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

void recordPairsFirstIteration(int *array, int arraySize, Hash *h){
    int pair[2];

    for (int i = 0; i < arraySize-1; i++){
        pair[0] = array[i]; pair[1] = array[i+1];
        incrementa_valor_hash(h, pair, 1);
    }
}

void recordPairs(int *array, int arraySize, Hash *h1, Hash *h2, int nonTerminal, Pair rule){
    int pair[2];
    int counter = 0;//quantidade de não-terminais seguidos
    while (counter<arraySize && array[counter]==nonTerminal) counter++;
    if (counter > 0){
        if (counter > 1){
            pair[0] = nonTerminal ; pair[1] = nonTerminal;
            incrementa_valor_hash(h1, pair, counter-1);
            pair[0] = rule.B; pair[1] = rule.A;
            incrementa_valor_hash(h2, pair, counter-1);
        }
        if (counter < arraySize){
            pair[0] = array[counter-1]; pair[1] = array[counter];
            incrementa_valor_hash(h1, pair, 1);
            pair[0] = rule.B;
            incrementa_valor_hash(h2, pair, 1);
        }
    }

    for (int i = counter+1; i < arraySize; i++){
        if (array[i]!=nonTerminal)
            continue;
        pair[0] = array[i-1]; pair[1] = array[i];
        incrementa_valor_hash(h1, pair, 1);
        pair[1] = rule.A;
        incrementa_valor_hash(h2, pair, 1);
        counter = 0;
        while(array[i]==nonTerminal){
            counter++; 
            i++;
        }
        if (counter > 1){
            pair[0] = nonTerminal ; pair[1] = nonTerminal;
            incrementa_valor_hash(h1, pair, counter-1);
            pair[0] = rule.B; pair[1] = rule.A;
            incrementa_valor_hash(h2, pair, counter-1);
        }
        pair[0] = array[i-1]; pair[1] = array[i];
        incrementa_valor_hash(h1, pair, 1);
        pair[0] = rule.B;
        incrementa_valor_hash(h2, pair, 1);
    }

}

int updateString(int **array, int arraySize, int **aux, Pair rule, int nonTerminal){
    int indexV = 0; 
    int indexAux = 0; 
    while ( indexV < arraySize-1){
        if (((*array)[indexV] == rule.A) && ((*array)[indexV+1] == rule.B)){
            (*aux)[indexAux++] = nonTerminal;
            indexV = indexV + 2;
        }
        else (*aux)[indexAux++] = (*array)[indexV++];
    }
    
    if (indexV==arraySize-1){ //o último inteiro pode ter ficado de fora.
        (*aux)[indexAux] = (*array)[indexV];
        indexAux++;
    } 

    swapPointers(array, aux);
    

    return indexAux;//novo tamanho
}

void insertPairsPQ(Hash *h, PQ *pq){ 
    //sei que eu não deveria usar detalhes das implementações dos TAD, mas depois arrumo isso
    for (int i = 0; i < h->M; i++){
       No* aux = h->vetor[i];
       while (aux!=NULL){
        pq_item x;
        x.chave[0] = aux->chave[0];
        x.chave[1] = aux->chave[1];
        x.dado = aux->dado;
        pq_adicionar(pq, x);
        aux=aux->prox;
       }
    }
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

    int sizeAux = expansionSizeA + expansionSizeB;

    int *aux = (int*) malloc(sizeAux*sizeof(int));

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
    exp.size = sizeAux;
    exp.ptr = aux;
    return exp;
}

expansion decompressor(int *compressedString, int arraySize, Pair *grammar, int grammarSize){
    expansion *expansions = expander(grammar, grammarSize);
    int sizeAux = arraySize; 
    int *aux = (int*) malloc(sizeAux*sizeof(int)); //o tamanho pode ser aumentado futuramente com realloc
    int auxCount=0; //quantidade de elementos em aux
    int grammarIndex, expansionSize;
    for (int compressedStringIndex = 0; compressedStringIndex < arraySize; compressedStringIndex++){
        if (compressedString[compressedStringIndex]<0){
            grammarIndex = -compressedString[compressedStringIndex]-1;
            expansionSize = expansions[grammarIndex].size;
            for (int expansionIndex = 0; expansionIndex < expansionSize; expansionIndex++){
                if (auxCount==sizeAux){
                    sizeAux*=2;
                    resizeArray(&aux,sizeAux);
                }
                aux[auxCount++] = expansions[grammarIndex].ptr[expansionIndex];
            }
            
        }
        else {
            if (auxCount==sizeAux){
                sizeAux*=2;
                resizeArray(&aux,sizeAux);
            }
            aux[auxCount++] = compressedString[compressedStringIndex];
        }
    }
    for (int i = 0; i < grammarSize; i++) free(expansions[i].ptr);
    free(expansions);
    expansion exp;
    //Talvez eu possa redimensionar aux para um tamanho menor aqui...
    exp.size = auxCount; exp.ptr=aux;
    return exp;
}

void printCompressedString(expansion compressedString){
    int size = compressedString.size;
    int *V = compressedString.ptr;
    printf("String comprimida: ");
    for (int i = 0; i < size; i++){
        if (V[i] < 0){
            printf("N%d",-V[i]);
        }
        else printf("%c",(char) V[i]);
    }
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



