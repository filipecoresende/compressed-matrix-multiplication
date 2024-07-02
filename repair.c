#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "lib/hash.h"
#include "lib/pq_heap.h"

typedef struct { 
    int A, B;   
} Pair; 

typedef struct {
    int size;
    int *ptr;
} expansion; //expansion of nonterminals

int* readFileAndConvertToIntArray(const char *filename, int *arraySize);
void convertIntArrayToTextFile(int *array, int arraySize, const char *filename); 

void swapPointers(int **ptr1, int **ptr2); 
void resizeArray(int **ptr, int newSize);
void resizeGrammar(Pair **grammar, int newSize);

void recordPairsFirstIteration(int *array, int arraySize, Hash *h); //record the pairs in the first iteration
void recordPairs(int *array, int arraySize, Hash *h1, Hash *h2, int nonTerminal, Pair rule); //records the pairs from the second iteration onwards
int updateString(int **array, int arraySize, int **aux, Pair rule, int nonTerminal); //produz a "string" com a nova regra e retorna seu novo tamanho
void insertPairsPQ(Hash *h, PQ *pq); //percorre h, adicionando cada par-frequência à fila pq
expansion* expander(Pair *grammar, int grammarSize); //expande cada símbolo não-terminal da gramática separadamente
expansion expandRule(expansion *expansions, Pair rule);

void writeBinaryFile(const char *inputFileName, int *compressedString, int compressedStringSize, Pair *grammar, int grammarSize);
expansion decompressor(int *compressedString, int compressedStringSize, Pair *grammar, int grammarSize); 

void printCompressedString(expansion compressedString);
void printGrammar(Pair *grammar, int grammarSize);

void compressFile(const char *inputFileName);
void decompressFile(const char *inputFilename, const char *outputFilename);


char* changeExtension(const char* originalFilename, const char* newExtension);

void printUsage(const char *progName);

int main(int argc, char **argv) {
    int opt;

    if (argc < 2) {
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "c:d:")) != -1) {
        switch (opt) {
            case 'c':
                compressFile(optarg);
                break;
            case 'd':
                decompressFile(optarg, "decompressedString.txt");
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

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

void convertIntArrayToTextFile(int *array, int arraySize, const char *filename) { 
    // Opens the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening the file");
        return;
    }

    // Writes the elements of the array to the file
    for (int i = 0; i < arraySize; i++) {
        fprintf(file, "%c", (char) array[i]);
    }

    fclose(file);
}

void swapPointers(int **ptr1, int **ptr2){
    int *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

void resizeArray(int **ptr, int newSize){
    *ptr = realloc(*ptr, newSize*sizeof(int));
}

void resizeGrammar(Pair **grammar, int newSize){
    *grammar = realloc(*grammar, newSize*sizeof(Pair));
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
    int counter = 0; //Count of consecutive non-terminal elements

    // Handle a possible sequence of non-terminal elements at the beginning
    while (counter<arraySize && array[counter]==nonTerminal) 
        counter++;

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

    for (int i = counter+1; i < arraySize; i++){ //handle consecutive non-terminal elements
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
    //Update the array of integers with a newly created non-terminal
    int indexV = 0; 
    int indexAux = 0; 
    while ( indexV < arraySize-1){
        if (((*array)[indexV] == rule.A) && ((*array)[indexV+1] == rule.B)){
            (*aux)[indexAux++] = nonTerminal;
            indexV = indexV + 2;
        }
        else (*aux)[indexAux++] = (*array)[indexV++];
    }
    
    if (indexV == arraySize-1){ //the last integer may have been left out
        (*aux)[indexAux] = (*array)[indexV];
        indexAux++;
    } 

    swapPointers(array, aux);
    

    return indexAux;//new size
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
    //return an array of expansions of the non-terminal symbols of the grammar
    expansion *expansions = (expansion*) malloc(grammarSize*sizeof(expansion));
    for (int i = 0; i < grammarSize; i++){
        expansions[i] = expandRule(expansions, grammar[i]);
    }
    return expansions;
}

expansion expandRule(expansion *expansions, Pair rule){ 
    //expands the rule and returns its expansion along with its size

    int index;

    //first symbol of the rule
    int expansionSizeA = 1; 
    if (rule.A < 0){
        index = -rule.A-1;
        expansionSizeA = expansions[index].size;
    }

    //second symbol of the rule
    int expansionSizeB = 1;
    if (rule.B < 0){
        index = -rule.B-1;
        expansionSizeB = expansions[index].size;
    }

    int sizeAux = expansionSizeA + expansionSizeB;

    int *aux = (int*) malloc(sizeAux*sizeof(int));

    if (rule.A < 0){
        index = -rule.A-1;
        for (int i = 0; i < expansionSizeA; i++){
            aux[i] = expansions[index].ptr[i];
        }  
    }
    else{
        aux[0] = rule.A;
    } 
    

    if (rule.B < 0){
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

void writeBinaryFile(const char *inputFileName, int *compressedString, int compressedStringSize, Pair *grammar, int grammarSize){
    //decides the appropriate name for the output file (binary) and stores the compressed information in it

    
    int sizeOutputArray = compressedStringSize + 2*grammarSize;
    int8_t *outputArray_int8;
    int16_t *outputArray_int16;
    int32_t *outputArray_int32;
    const char *newExtension; //extension of the binary file to be generated

    int intSize; //stores how many bits are required for the "biggest" (or smallest) integer
    if ((-grammarSize) >= INT8_MIN){
        intSize = 8;
        newExtension = ".re8";
        outputArray_int8 = (int8_t *) malloc(sizeof(int8_t)*sizeOutputArray);
    }
    else if ((-grammarSize) >= INT16_MIN){
        intSize = 16;
        newExtension = ".re16";
        outputArray_int16 = (int16_t *) malloc(sizeof(int16_t)*sizeOutputArray);
    } 
    else if ((-grammarSize) >= INT32_MIN){
        intSize = 32;
        newExtension = ".re32";
        outputArray_int32 = (int32_t *) malloc(sizeof(int32_t)*sizeOutputArray);
    } 

    char* outputFileName = changeExtension(inputFileName, newExtension); 

    // Opens the output file for writing in binary mode
    FILE *outputFile = fopen(outputFileName, "wb");
    if (!outputFile) {
        // Print an error message and exit if the file cannot be opened
        perror("Unable to open file for writing");
        exit(EXIT_FAILURE);
    }
    
    free(outputFileName);

    //these 3 sizes are written as regular integers
    if (fwrite(&intSize, sizeof(intSize), 1, outputFile) != 1){ 
        perror("Error writing intSize to file");
        fclose(outputFile);
        exit(EXIT_FAILURE);
    }
    if (fwrite(&compressedStringSize, sizeof(compressedStringSize), 1, outputFile) != 1){ 
        perror("Error writing compressedStringSize to file");
        fclose(outputFile);
        exit(EXIT_FAILURE);
    }
    if (fwrite(&grammarSize, sizeof(grammarSize), 1, outputFile) != 1){ 
        perror("Error writing grammarSize to file");
        fclose(outputFile);
        exit(EXIT_FAILURE);
    }

    int index = 0;
    switch (intSize){ 
        case 8: 
            for (int i = 0; i < compressedStringSize; i++){
                outputArray_int8[index++] = (int8_t) compressedString[i];
            }             
            for (int i = 0; i < grammarSize; i++){
                outputArray_int8[index++] = (int8_t) grammar[i].A;
                outputArray_int8[index++] = (int8_t) grammar[i].B;
            }
            if (fwrite(outputArray_int8, sizeof(int8_t), index, outputFile) != index){
                perror("Error writing outputArray_int8 to file");
                fclose(outputFile);
                exit(EXIT_FAILURE);
            }
            free(outputArray_int8);
            break;
        case 16:
            for (int i = 0; i < compressedStringSize; i++){
                outputArray_int16[index++] = (int16_t) compressedString[i];
            }             
            for (int i = 0; i < grammarSize; i++){
                outputArray_int16[index++] = (int16_t) grammar[i].A;
                outputArray_int16[index++] = (int16_t) grammar[i].B;
            }
            if (fwrite(outputArray_int16, sizeof(int16_t), index, outputFile) != index){
                perror("Error writing outputArray_int16 to file");
                fclose(outputFile);
                exit(EXIT_FAILURE);
            }
            free(outputArray_int16);
            break;
        case 32:
            for (int i = 0; i < compressedStringSize; i++){
                outputArray_int32[index++] = (int32_t) compressedString[i];
            }             
            for (int i = 0; i < grammarSize; i++){
                outputArray_int32[index++] = (int32_t) grammar[i].A;
                outputArray_int32[index++] = (int32_t) grammar[i].B;
            }
            if (fwrite(outputArray_int32, sizeof(int32_t), index, outputFile) != index){
                perror("Error writing outputArray_int32 to file");
                fclose(outputFile);
                exit(EXIT_FAILURE);
            }
            free(outputArray_int32);
            break;
        default:
            exit(EXIT_FAILURE);
    }

    
    if (fclose(outputFile) != 0) {
        perror("Error closing file");
        exit(EXIT_FAILURE);
    }

}

expansion decompressor(int *array, int arraySize, Pair *grammar, int grammarSize){
//reconstructs the original text as an array of integers
    expansion *expansions = expander(grammar, grammarSize); //expand the non-terminal symbols
    int sizeAux = arraySize; 
    int *aux = (int*) malloc(sizeAux*sizeof(int)); //the size may be increased in the future with realloc
    int auxCount = 0; //number of elements in aux

    int grammarIndex, expansionSize;
    for (int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++){
        if (array[arrayIndex] < 0){
            grammarIndex = -array[arrayIndex] - 1;
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
            aux[auxCount++] = array[arrayIndex];
        }
    }

    for (int i = 0; i < grammarSize; i++) 
        free(expansions[i].ptr);
    free(expansions);

    aux = (int*) realloc(aux, auxCount*sizeof(int));
    expansion exp;
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

char* changeExtension(const char* originalFilename, const char* newExtension) {
// returns a filename with the new extension

    // Find the last dot in the filename
    const char* dot = strrchr(originalFilename, '.');
    
    // Calculate the new length: (original length - length of old extension) + length of new extension + 1 for null terminator
    size_t new_length;
    if (dot) {
        new_length = (dot - originalFilename) + strlen(newExtension) + 1;
    } else {
        new_length = strlen(originalFilename) + strlen(newExtension) + 1;
    }

    // Allocate memory for the new filename
    char* newFilename = (char*)malloc(new_length);

    if (!newFilename) {
        perror("malloc");
        return NULL;
    }

    // Copy the base filename (excluding the old extension, if any)
    if (dot) {
        strncpy(newFilename, originalFilename, dot - originalFilename);
        newFilename[dot - originalFilename] = '\0';
    } else {
        strcpy(newFilename, originalFilename);
    }

    // Append the new extension
    strcat(newFilename, newExtension);

    return newFilename;
}

void compressFile(const char *inputFileName){
//produces a binary file with the compressed text

    //stores the original text as an array of integers
    int sizeV;
    int *V = readFileAndConvertToIntArray(inputFileName, &sizeV);

    int *aux = (int*) malloc(sizeV*sizeof(int)); //helps to update V at each iteration
    int grammarSize = 1000; //this size may be increased if necessary
    Pair *grammar = (Pair*) malloc(grammarSize*sizeof(Pair)); //array of rules (that may be resized in the future)
    PQ *pq = pq_criar(1000);
    int numRules = 0; 
    Hash *h1 = criar_hash(1000);
    Hash *h2 = criar_hash(1000);
    recordPairsFirstIteration(V, sizeV, h1);//conta os pares na primeira iteração do RePair
    insertPairsPQ(h1, pq); //insere todos os pares numa fila de prioridade
    destruir_hash(&h1); 
    pq_item auxPair = pq_extrai_maximo(pq); 
    Pair maxPair;
    maxPair.A = auxPair.chave[0]; maxPair.B = auxPair.chave[1];
    int maxPairCount = auxPair.dado;
   

    while(maxPairCount > 1){
        printf("(%d, %d): %d repetições\n", maxPair.A, maxPair.B, maxPairCount);
        grammar[numRules++] = maxPair;

        if (numRules==grammarSize){
            grammarSize*=2;
            resizeGrammar(&grammar, grammarSize);
        }

        sizeV = updateString(&V, sizeV, &aux, maxPair, -numRules);
        h1 = criar_hash(1000);
        recordPairs(V, sizeV, h1, h2, -numRules, maxPair);
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

    writeBinaryFile(inputFileName, V, sizeV, grammar, numRules);

    free(V);
    free(grammar);
   
}

void decompressFile(const char *inputFilename, const char *outputFilename) { 

    FILE *inputFile = fopen(inputFilename, "rb");
    if (!inputFile) {
        perror("Unable to open file for reading");
        exit(EXIT_FAILURE);
    }

    int intSize, sizeV, numRules;
    if (fread(&intSize, sizeof(intSize), 1, inputFile) != 1) {
        perror("Error reading intSize");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }
    if (fread(&sizeV, sizeof(sizeV), 1, inputFile) != 1) {
        perror("Error reading sizeV");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }
    if (fread(&numRules, sizeof(numRules), 1, inputFile) != 1) {
        perror("Error reading numRules");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }


    int *V = (int *) malloc(sizeV * sizeof(int));
    if (!V) {
        perror("Memory allocation failed for V");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    Pair *grammar = (Pair *) malloc(numRules*sizeof(Pair));
    if (!grammar) {
        perror("Memory allocation failed for grammar");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    int8_t *inputArray_int8;
    int16_t *inputArray_int16;
    int32_t *inputArray_int32;
    int sizeInputArray = sizeV + 2*numRules;

    int index = 0;
    switch (intSize){
        case 8:
            inputArray_int8 = (int8_t*) malloc(sizeInputArray * sizeof(int8_t));
            if (!inputArray_int8) {
                perror("Memory allocation failed for inputArray_int8");
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            if (fread(inputArray_int8, sizeof(int8_t), sizeInputArray, inputFile) != sizeInputArray) {
                perror("Error reading inputArray_int8");
                free(inputArray_int8);
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < sizeV; i++){
                V[i] = (int) inputArray_int8[index++];
            }

            for (int i = 0; i < numRules; i++){
                grammar[i].A = (int) inputArray_int8[index++];
                grammar[i].B = (int) inputArray_int8[index++];
            }

            free(inputArray_int8);
            break;
        
        case 16:
            inputArray_int16 = (int16_t *)malloc(sizeInputArray * sizeof(int16_t));
            if (!inputArray_int16) {
                perror("Memory allocation failed for inputArray_int16");
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            if (fread(inputArray_int16, sizeof(int16_t), sizeInputArray, inputFile) != sizeInputArray) {
                perror("Error reading inputArray_int16");
                free(inputArray_int16);
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < sizeV; i++){
                V[i] = (int) inputArray_int16[index++];
            }

            for (int i = 0; i < numRules; i++){
                grammar[i].A = (int) inputArray_int16[index++];
                grammar[i].B = (int) inputArray_int16[index++];
            }
           
            free(inputArray_int16);
            break;

        case 32:
            inputArray_int32 = (int32_t *)malloc(sizeInputArray * sizeof(int32_t));
            if (!inputArray_int32) {
                perror("Memory allocation failed for inputArray_int32");
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            if (fread(inputArray_int32, sizeof(int32_t), sizeInputArray, inputFile) != sizeInputArray) {
                perror("Error reading inputArray_int32");
                free(inputArray_int32);
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < sizeV; i++){
                V[i] = (int) inputArray_int32[index++];
            }

            for (int i = 0; i < numRules; i++){
                grammar[i].A = (int) inputArray_int32[index++];
                grammar[i].B = (int) inputArray_int32[index++];
            }
            
            free(inputArray_int32);
            break;

        default:
            break;
    }

    fclose(inputFile);

    expansion decompressedString = decompressor(V, sizeV, grammar, numRules);

    free(V);
    free(grammar);

    convertIntArrayToTextFile(decompressedString.ptr, decompressedString.size, outputFilename);


    
    free(decompressedString.ptr);
}

void printUsage(const char *progName) {
    fprintf(stderr, "Usage: %s -c <file_to_compress> | -d <file_to_decompress>\n", progName);
}