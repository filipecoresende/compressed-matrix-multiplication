#include <iostream>
#include <fstream>
#include <unistd.h>
#include "repair.hpp"

using namespace std;

//reads from a text file and produces the first version of the symbol vector
void readFileAndStoreCharacters(const string& filename, vector<VectorElement>& symbolVector) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    char ch;

    while (file.get(ch)) {
        int charAsInt = static_cast<int>(ch);
        symbolVector.emplace_back(VectorElement{EMPTY, charAsInt, EMPTY});
    }

    if (!file.eof()){
        cerr << "Error reading from " << filename << endl;
        file.close();
        exit(EXIT_FAILURE);
    }

    file.close();
}


//Increments the count of a pair and links it to the next occurrence of the 
// pair in the symbol vector
void incrementPairCountFirstIteration(int currentPosition, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
    
    int leftSymbol = symbolVector[currentPosition].symbol;
    int rightSymbol = symbolVector[currentPosition + 1].symbol;
    const Pair pair = {leftSymbol, rightSymbol};
    auto hashTableIterator = hashTable.find(pair);

    if (hashTableIterator == hashTable.end()){
        //inserts a new pair record in the first list of the priority queue
        priorityQueueVector[0].emplace_front(PairRecord{currentPosition, 1});
        hashTable.emplace(pair, HashTableEntry{priorityQueueVector[0].begin(), false});
        return;
    }

    PairRecord& pairRecord = *(hashTableIterator->second.pairRecordIterator);
    pairRecord.pairCount++;
    int nextOccurrenceOfThePair = pairRecord.firstAppearance; 
    symbolVector[nextOccurrenceOfThePair].prev = currentPosition; 
    symbolVector[currentPosition].next = nextOccurrenceOfThePair;
    pairRecord.firstAppearance = currentPosition;
}

//performs the first iteration (it's done backwards) through the symbol vector 
//and records the counts of the pairs.
void iterateFirstTime(vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){


    for (int i = symbolVector.size() - 2; i >= 0; i--){
        Pair pair = getPairFromPosition(i, symbolVector);
        if (pair.first == 0 || pair.second == 0) continue; //ignore o $
        PairRecord* pairRecordPointer = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecordPointer == nullptr){
            PairRecord newPairRecord;
            newPairRecord.pairCount = 1; 
            newPairRecord.firstAppearance = i;
            priorityQueueVector[0].emplace_front(newPairRecord);
            hashTable.emplace(pair, HashTableEntry{priorityQueueVector[0].begin(), false});
        }
        else{
            symbolVector[i].next = pairRecordPointer->firstAppearance;
            symbolVector[pairRecordPointer->firstAppearance].prev = i;
            pairRecordPointer->firstAppearance = i;
        }
    }
    
}

void removeUnnecessaryRecords(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector){
    auto it = priorityQueueVector[0].begin();
    while (it != priorityQueueVector[0].end()){
        if (it->pairCount == 1){
            Pair pair = getPairFromPosition(it->firstAppearance, symbolVector);
            auto itHashTable = hashTable.find(pair);
            hashTable.erase(itHashTable);
            it = priorityQueueVector[0].erase(it);
        }
        else ++it;
    }
}



int getMostFrequentPair(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector, vector<Pair>& grammar){
    int i;
    
    for (i = priorityQueueVector.size() - 1; i >= 0; i--){
        if (!priorityQueueVector[i].empty())
            break;
    }

    if (i == -1)
        return EMPTY;


    auto& listWithMaxPair = priorityQueueVector[i];

    list<PairRecord>::iterator itMaxPair;

    if (i == priorityQueueVector.size() - 1){
        // Custom comparator to compare pairCount of PairRecord
        auto comp = [](const PairRecord& a, const PairRecord& b) {
            return a.pairCount < b.pairCount;
        };
        itMaxPair = max_element(listWithMaxPair.begin(), listWithMaxPair.end(), comp);
    }
    else
        itMaxPair = listWithMaxPair.begin();
    

    int firstAppearanceMaxPair = itMaxPair->firstAppearance;
    
    listWithMaxPair.erase(itMaxPair); 

    const Pair maxPair = getPairFromPosition(firstAppearanceMaxPair, symbolVector);


    auto itHashTable = hashTable.find(maxPair);

    hashTable.erase(itHashTable);

    grammar.push_back(maxPair);

    return firstAppearanceMaxPair;

}

Pair getPairFromPosition(int position, vector<VectorElement>& symbolVector){ //'position' cannot be the last nonempty position
    int leftSymbol = symbolVector[position].symbol;
    int rightSymbolPosition = getNextNonEmptyPosition(position, symbolVector);
    int rightSymbol = symbolVector[rightSymbolPosition].symbol;
    return Pair{leftSymbol, rightSymbol};

}

int replacePairs(int maxPairFirstAppearance, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, int newSymbol){

    int currentPosition = maxPairFirstAppearance;
    int lastOccurrence; //holds the last occurrence of the pair being replaced
    while(currentPosition != EMPTY){

        int nextPosition = pairContextHandler(currentPosition, symbolVector, hashTable, priorityQueueVector, newSymbol);
        
        lastOccurrence = currentPosition;
        currentPosition = nextPosition;        
    } 

    return lastOccurrence;
}

//returns nullptr in case the current position is the last non-empty position in the symbol vector
int getNextNonEmptyPosition(int currentPosition, vector<VectorElement>& symbolVector){ 
    if (currentPosition == symbolVector.size() - 1)
        return EMPTY;
    int nextNonEmptyPosition = currentPosition + 1;
    if (symbolVector[nextNonEmptyPosition].symbol == EMPTY)
        nextNonEmptyPosition = symbolVector[nextNonEmptyPosition].next;
    

    return nextNonEmptyPosition;
}

//returns nullptr in case the current position is the first non-empty position in the symbol vector
int getPreviousNonEmptyPosition(int currentPosition, vector<VectorElement>& symbolVector){
    if (currentPosition == 0)
        return EMPTY;
    int previousNonEmptyPosition = currentPosition - 1;
    if (symbolVector[previousNonEmptyPosition].symbol == EMPTY)
        previousNonEmptyPosition = symbolVector[previousNonEmptyPosition].prev;
    return previousNonEmptyPosition;
}

int pairContextHandler(int ptr, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, int newSymbol){
    //Suppose the pair being replace is "ab", and its context in a given appearance is "xaby"
    //Records for the new pairs xA and Ay (A is the new symbol) must be created if they appear at least twice in the sequence,
    //but the number of occurrences for them will not be counted yet.

    int pairContext[4];
    pairContext[0] = getPreviousNonEmptyPosition(ptr, symbolVector);
    pairContext[1] = ptr;
    pairContext[2] = getNextNonEmptyPosition(pairContext[1], symbolVector);
    pairContext[3] = getNextNonEmptyPosition(pairContext[2], symbolVector);


    if (pairContext[0] != EMPTY && symbolVector[pairContext[0]].symbol != newSymbol){
        Pair auxPair = {symbolVector[pairContext[0]].symbol, symbolVector[pairContext[1]].symbol}; 
        Pair newPair = {symbolVector[pairContext[0]].symbol, newSymbol};
        createPairRecordIfNecessary(newPair, auxPair,  hashTable, priorityQueueVector); //check if a record must be created for xA

        Pair pairThatDisappear = {symbolVector[pairContext[0]].symbol, symbolVector[pairContext[1]].symbol};

        handleDisappearingPair(pairThatDisappear, pairContext[0], symbolVector, hashTable, priorityQueueVector);
        
    }

    if (pairContext[3] != EMPTY){

        if (symbolVector[pairContext[1]].symbol != symbolVector[pairContext[3]].symbol){
            Pair auxPair = {symbolVector[pairContext[2]].symbol, symbolVector[pairContext[3]].symbol};
            Pair newPair = {newSymbol, symbolVector[pairContext[3]].symbol};
            createPairRecordIfNecessary(newPair, auxPair, hashTable, priorityQueueVector); //check if a record must be created for Ay
        }

        Pair pairThatDisappear = {symbolVector[pairContext[2]].symbol, symbolVector[pairContext[3]].symbol};

        handleDisappearingPair(pairThatDisappear, pairContext[2], symbolVector, hashTable, priorityQueueVector);

    }

    int nextPosition = symbolVector[pairContext[1]].next;

    symbolVector[pairContext[1]].symbol = newSymbol; 

    symbolVector[pairContext[2]].symbol = EMPTY; 
    symbolVector[pairContext[1] + 1].next = pairContext[3];

    if (pairContext[3] != EMPTY)
       symbolVector[pairContext[3]-1].prev = pairContext[1];
           
    return nextPosition;

}


void decreasePairCount(const Pair& pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){

    auto itHashTable = hashTable.find(pair);
    if (itHashTable == hashTable.end())
        return;


    auto itPairRecord = itHashTable->second.pairRecordIterator;


    int& currentCount = itPairRecord->pairCount;

    currentCount--;

    if (currentCount == 1){   
        priorityQueueVector[0].erase(itPairRecord);
        hashTable.erase(itHashTable);
    }


    else if (currentCount <= priorityQueueVector.size()){
        list<PairRecord>& listWithPair = priorityQueueVector[currentCount - 1];
        list<PairRecord>& destinationList = priorityQueueVector[currentCount - 2];
        destinationList.splice(destinationList.end(), listWithPair, itPairRecord);
    }


}

void createPairRecordIfNecessary(const Pair& newPair, const Pair& auxPair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
   
    auto itHashTable = hashTable.find(newPair);
    if (itHashTable != hashTable.end())
        return;


    itHashTable = hashTable.find(auxPair);
    if (itHashTable == hashTable.end())
        return;

    bool& flag = itHashTable->second.flag;
    if (flag){
        priorityQueueVector[0].emplace_front(PairRecord{EMPTY, 0});
        hashTable.emplace(newPair, HashTableEntry{priorityQueueVector[0].begin(), false});
    }
    else flag = !flag; 

}

//Iterates through the vector backwards
void handleNewPairs(int ptr, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, Pair replacedPair){    
    int currentPosition = ptr;
    int pairContext[3];

    while (currentPosition != EMPTY){

        pairContext[0] = getPreviousNonEmptyPosition(currentPosition, symbolVector);
        pairContext[1] = currentPosition;
        pairContext[2] = getNextNonEmptyPosition(pairContext[1], symbolVector);


        clearFlags(&pairContext[0], symbolVector, hashTable, replacedPair);
        decrementOldPairs(&pairContext[0], symbolVector, hashTable, priorityQueueVector, replacedPair);

        currentPosition = newPairs(&pairContext[0], hashTable, priorityQueueVector, symbolVector);
        
    }


}


int newPairs(int* pairContext, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector){

    if (pairContext[0] != EMPTY && symbolVector[pairContext[0]].symbol != symbolVector[pairContext[1]].symbol){
        symbolVector[pairContext[0]].prev = EMPTY;
        symbolVector[pairContext[0]].next = EMPTY;    
        Pair pair = {symbolVector[pairContext[0]].symbol, symbolVector[pairContext[1]].symbol};
        PairRecord *pairRecord = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecord != nullptr){
            symbolVector[pairContext[0]].next = pairRecord->firstAppearance;
            if (pairRecord->firstAppearance != EMPTY)
                symbolVector[pairRecord->firstAppearance].prev = pairContext[0];
            pairRecord->firstAppearance = pairContext[0];
        }
    }


    int aux = symbolVector[pairContext[1]].prev;  
    if (pairContext[2] != EMPTY){
        symbolVector[pairContext[1]].prev = EMPTY; 
        symbolVector[pairContext[1]].next = EMPTY;
        Pair pair = {symbolVector[pairContext[1]].symbol, symbolVector[pairContext[2]].symbol};
        PairRecord *pairRecord = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecord != nullptr){
            symbolVector[pairContext[1]].next = pairRecord->firstAppearance;
            if (pairRecord->firstAppearance != EMPTY)
                symbolVector[pairRecord->firstAppearance].prev = pairContext[1];
            pairRecord->firstAppearance = pairContext[1];
        }
    }
    
    return aux;
}


void clearFlags(int* pairContext, vector<VectorElement>& symbolVector, HashTable& hashTable, Pair replacedPair){
    if (pairContext[0] != EMPTY && symbolVector[pairContext[0]].symbol != symbolVector[pairContext[1]].symbol){
        Pair auxPair = Pair{symbolVector[pairContext[0]].symbol, replacedPair.first}; 
        auto itAux = hashTable.find(auxPair);
        if (itAux != hashTable.end())
            itAux->second.flag = false;
    }

    if (pairContext[2] != EMPTY && symbolVector[pairContext[1]].symbol != symbolVector[pairContext[2]].symbol){
        Pair auxPair = Pair{replacedPair.second, symbolVector[pairContext[2]].symbol}; 
        auto itAux = hashTable.find(auxPair);
        if (itAux != hashTable.end())
            itAux->second.flag = false;
    }

}

void decrementOldPairs(int* pairContext, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, Pair replacedPair){
    if (pairContext[0] != EMPTY && symbolVector[pairContext[0]].symbol != symbolVector[pairContext[1]].symbol){
        Pair pairThatDisappear = {symbolVector[pairContext[0]].symbol, replacedPair.first};
        decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector); //talvez dê pra já aproveitar o itHashTable de antes 
    }

    if (pairContext[2] != EMPTY){
        if (symbolVector[pairContext[1]].symbol != symbolVector[pairContext[2]].symbol){
            Pair pairThatDisappear = {replacedPair.second, symbolVector[pairContext[2]].symbol};
            decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector);//decrease the count for "by"
        }
        else{
            Pair pairThatDisappear = {replacedPair.second, replacedPair.first};
            decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector);//decrease the count for "by"
        }
        
    }

}



vector<vector<int>> expander(vector<Pair>& grammar){

    //return an array of expansions of the non-terminal symbols of the grammar
    vector<vector<int>> expansions(grammar.size());
    for (int i = 0; i < grammar.size(); i++)
        expansions[i] = expandRule(expansions, grammar[i]);
    
    return expansions;
}



vector<int> expandRule(vector<vector<int>>& expansions, Pair rule){
    //expands the rule and returns its expansion along with its size

    int indexA, indexB;
    //first symbol of the rule
    int expansionSizeA = 1; 
    if (rule.first < 0){
        indexA = -rule.first-1;
        expansionSizeA = expansions[indexA].size();
    }

    //second symbol of the rule
    int expansionSizeB = 1;
    if (rule.second < 0){
        indexB = -rule.second-1;
        expansionSizeB = expansions[indexB].size();
    }

    int sizeAux = expansionSizeA + expansionSizeB;

    vector<int> aux;
    aux.reserve(sizeAux);

    if (rule.first < 0){
        for (int i = 0; i < expansionSizeA; i++){
            aux.push_back(expansions[indexA][i]);
        }  
    }
    else{
        aux.push_back(rule.first);
    } 

    if (rule.second < 0){
        for (int i = 0; i < expansionSizeB; i++){
            aux.push_back(expansions[indexB][i]);
        } 
    }
    else{
        aux.push_back(rule.second);
    } 
    return aux;
}

void writeBinaryFile(const string& inputFilename, vector<VectorElement>& symbolVector, vector<Pair>& grammar){
    //decides the appropriate name for the output file (binary) and stores the compressed information in it

    vector<int> outputVector;

    int numRules = grammar.size(); //number of rules

    string outputFilename = changeExtension(inputFilename, ".re32"); 

    // Opens the output file for writing in binary mode
    ofstream outputFile(outputFilename, ios::binary);
    if (!outputFile.is_open()) {
        // Print an error message and exit if the file cannot be opened
        cerr << "Error: Could not open " << outputFilename << endl;
        exit(EXIT_FAILURE);
    }

    outputFile.write(reinterpret_cast<char*>(&numRules), sizeof(numRules));
    if (!outputFile.good()) {
        cerr << "Error writing numRules to " << outputFilename << endl;
        outputFile.close();
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < numRules; i++){
        outputVector.push_back(grammar[i].first);
        outputVector.push_back(grammar[i].second);
    }
    for (int i = 0; i < symbolVector.size(); i++){
        if (symbolVector[i].symbol == EMPTY){
            i = symbolVector[i].next;
            if (i == EMPTY) break;
        } 
        outputVector.push_back(symbolVector[i].symbol);
    }                  
    outputFile.write(reinterpret_cast<const char*>(outputVector.data()), outputVector.size() * sizeof(int));
    if (!outputFile.good()) {
        cerr << "Error writing outputVector to " << outputFilename << endl;
        outputFile.close(); // Ensure the file is closed on error
        exit(EXIT_FAILURE);
    }
        
    outputFile.close();

    if (!outputFile) {
        cerr << "Error closing " << outputFilename << endl;
        exit(EXIT_FAILURE);
    }

}

string changeExtension(const string& originalFilename, const string& newExtension) {
    // Find the position of the last dot in the filename
    size_t dotPos = originalFilename.find_last_of('.');
    
    // If a dot is found, copy the part of the filename before the dot
    string baseName;
    if (dotPos != string::npos) {
        baseName = originalFilename.substr(0, dotPos);
    } else {
        baseName = originalFilename;
    }
    
    // Add the new extension to the base name
    // Ensure newExtension starts with a dot
    string result = baseName + (newExtension.empty() || newExtension[0] == '.' ? newExtension : '.' + newExtension);
    
    return result;
}


void decompressFile(const string& inputFilename, const string& outputFilename) { 

    pair<vector<int>,vector<Pair>> aux = getSequenceAndGrammar(inputFilename);

    vector<int> originalVector = decompressor(get<0>(aux), get<1>(aux));

    convertIntArrayToTextFile(originalVector, outputFilename);

}

pair<vector<int>,vector<Pair>> getSequenceAndGrammar(const string& inputFilename){
    ifstream inputFile(inputFilename, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << inputFilename << endl;
        exit(EXIT_FAILURE);
    }
    
    int numRules;
    inputFile.read(reinterpret_cast<char*>(&numRules), sizeof(numRules));
    if (!inputFile.good()) {
        cerr << "Error reading numRules from " << inputFilename << endl;
        inputFile.close();
        exit(EXIT_FAILURE);
    }

    vector<Pair> grammar;
    grammar.reserve(numRules);
    vector<int> V;

    
    int value1, value2;
    for (int i = 0; i < numRules; i++){
        if (!inputFile.read(reinterpret_cast<char*>(&value1), sizeof(value1))) {
            cerr << "Error reading value1 at rule " << i << endl;
            exit(EXIT_FAILURE);
        }
        if (!inputFile.read(reinterpret_cast<char*>(&value2), sizeof(value2))) {
            cerr << "Error reading value2 at rule " << i << endl;
            exit(EXIT_FAILURE);
        }
        grammar.emplace_back(Pair{value1, value2});
    }

    int value;
    while(inputFile.read(reinterpret_cast<char*>(&value), sizeof(value)))
        V.push_back(value);
    if (!inputFile.eof()) {
        cerr << "Error reading V from " << inputFilename << endl;
        inputFile.close();
        exit(EXIT_FAILURE);
    }
            
        

    inputFile.close();

    return make_pair(V,grammar);

};


void convertIntArrayToTextFile(vector<int>& array, const string& outputFilename) { 
    ofstream outputFile(outputFilename);
    if (!outputFile.is_open()){
        cerr << "Error: Could not open " << outputFilename << endl;
        exit(EXIT_FAILURE);
    }

    for (const auto& vectorElement: array){
        outputFile << static_cast<char>(vectorElement);
    }

    if (outputFile.fail()) {
        cerr << "Error writing to " << outputFilename << endl;
        outputFile.close();
        exit(EXIT_FAILURE);
    }

    outputFile.close();
} 


PairRecord* incrementPairCount(const Pair pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
    auto itHashTable = hashTable.find(pair);
    if (itHashTable == hashTable.end()) 
        return nullptr;
    auto itPairRecord = itHashTable->second.pairRecordIterator;
    auto& pairRecord = *itPairRecord;
    int count = ++pairRecord.pairCount;

    if (count > 2 && count < priorityQueueVector.size() + 2){
        auto& listWithPair = priorityQueueVector[count - 3];
        auto& destinationList = priorityQueueVector[count - 2];
        destinationList.splice(destinationList.end(), listWithPair, itPairRecord);
    }
    
    return &pairRecord;
}

void compressFile(const string& filename){

    vector<VectorElement> symbolVector;

    readFileAndStoreCharacters(filename, symbolVector);

    compressor(filename, symbolVector);
    
}

void compressor(const string& inputFilename, vector<VectorElement>& symbolVector){
    size_t priorityQueueVectorSize = ceil(sqrt(symbolVector.size()));

    HashTable hashTable;
    vector<list<PairRecord>> priorityQueueVector(priorityQueueVectorSize);

    vector<Pair> grammar;

    iterateFirstTime(symbolVector, hashTable, priorityQueueVector); 
    removeUnnecessaryRecords(priorityQueueVector, hashTable, symbolVector);   

    int newSymbol = 0;
    auto maxPairFirstAppearance = getMostFrequentPair(priorityQueueVector, hashTable, symbolVector, grammar);
  
    while (maxPairFirstAppearance != EMPTY){

        newSymbol--;


        auto maxPair = getPairFromPosition(maxPairFirstAppearance, symbolVector);

        priorityQueueVector[0].emplace_front(PairRecord{EMPTY, 0});
        hashTable.emplace(Pair{newSymbol, newSymbol}, HashTableEntry{priorityQueueVector[0].begin(), false});

        priorityQueueVector[0].emplace_front(PairRecord{EMPTY, 0});
        hashTable.emplace(Pair{newSymbol, maxPair.first}, HashTableEntry{priorityQueueVector[0].begin(), false});

        auto lastOccurrence = replacePairs(maxPairFirstAppearance, hashTable, priorityQueueVector, symbolVector, newSymbol);

        handleNewPairs(lastOccurrence, hashTable, priorityQueueVector, symbolVector, maxPair);

        auto it = hashTable.find(Pair{newSymbol, newSymbol});
        auto pairRecordIterator = it->second.pairRecordIterator;
        if (pairRecordIterator->pairCount < 2){
            priorityQueueVector[0].erase(pairRecordIterator);
            hashTable.erase(it);
        }

        it = hashTable.find(Pair{newSymbol, maxPair.first});
        pairRecordIterator = it->second.pairRecordIterator;
        if (pairRecordIterator->pairCount < 2){
            priorityQueueVector[0].erase(pairRecordIterator);
            hashTable.erase(it);
        }
        

        maxPairFirstAppearance = getMostFrequentPair(priorityQueueVector, hashTable, symbolVector, grammar);

    }
    writeBinaryFile(inputFilename, symbolVector, grammar);
}


vector<int> decompressor(vector<int>& vectorOfIntegers, vector<Pair>& grammar){

    vector<Pair> alreadyExpandedNonTerminals(grammar.size()); 
    //the first element of Pair holds the location of the expansion within originalVector (it's an index)
    //the second element holds the size of the expansion
    vector<int> originalVector; //we want to reconstruct the original text                                                          
    originalVector.reserve(vectorOfIntegers.size());
    for (int i = 0; i < vectorOfIntegers.size(); i++){
        if (vectorOfIntegers[i] < 0){
            vector<int> expansion = expandNonTerminal(originalVector, originalVector.size(), vectorOfIntegers[i], grammar, alreadyExpandedNonTerminals);
            for (const auto& i: expansion)
                originalVector.push_back(i);
        } 
        else 
            originalVector.push_back(vectorOfIntegers[i]);
    }

    originalVector.shrink_to_fit();
    
    return originalVector;

}

vector<int> expandNonTerminal(vector<int>& originalVector, int positionOriginalVector, int nonTerminal, vector<Pair>& grammar, vector<Pair>& alreadyExpandedNonTerminals){
    vector<int> expansion;
    int grammarIndex = -nonTerminal - 1;
    Pair auxPair = alreadyExpandedNonTerminals[grammarIndex];
    if (auxPair.second != 0 && originalVector.size() >= auxPair.first + auxPair.second){
        expansion.resize(auxPair.second);
        copy(originalVector.begin() + auxPair.first, originalVector.begin() + auxPair.first + auxPair.second, expansion.begin());
        return expansion;
    }

    int symbol1 = grammar[grammarIndex].first;
    int symbol2 = grammar[grammarIndex].second;

    int positionSymbol2; //it depends on the size of the expansion of symbol1

    vector<int> aux;
    if (symbol1 < 0){
        aux = expandNonTerminal(originalVector, positionOriginalVector, symbol1, grammar, alreadyExpandedNonTerminals);
        for (const auto& i: aux){
            expansion.push_back(i);
        }
        positionSymbol2 = positionOriginalVector + aux.size();
    }
    else{
        expansion.push_back(symbol1);
        positionSymbol2 = positionOriginalVector + 1;
    }
    if (symbol2 < 0){
        aux = expandNonTerminal(originalVector, positionSymbol2, symbol2, grammar, alreadyExpandedNonTerminals);
        for (const auto& i: aux){
            expansion.push_back(i);
        }
    }
    else
        expansion.push_back(symbol2);

    alreadyExpandedNonTerminals[grammarIndex] = Pair{positionOriginalVector, expansion.size()};

    return expansion;
}

void handleDisappearingPair(Pair disappearingPair, int currentPosition, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){

    int previousOccurrence = symbolVector[currentPosition].prev;
    int nextOccurrence = symbolVector[currentPosition].next;

    if (previousOccurrence != EMPTY){
        symbolVector[previousOccurrence].next = nextOccurrence;
        if (nextOccurrence != EMPTY) symbolVector[nextOccurrence].prev = previousOccurrence;
        return;
    } 
    if (nextOccurrence == EMPTY) return;
    symbolVector[nextOccurrence].prev = EMPTY;
    auto itHashTable = hashTable.find(disappearingPair);
    itHashTable->second.pairRecordIterator->firstAppearance = nextOccurrence;
}

// int main(int argc, char *argv[]){
//     int option;
//     while ((option = getopt(argc, argv, "c:d:")) != -1) {
//         switch (option) {
//             case 'c':{
//                 auto start = chrono::high_resolution_clock::now();
//                 compressFile(optarg);
//                 auto end = chrono::high_resolution_clock::now();
//                 chrono::duration<double, milli> duration = end - start;
//                 cout << "The compression took " << duration.count() << " milliseconds\n";
//                 break;
//             }
//             case 'd':{  
//                 auto start = chrono::high_resolution_clock::now();
//                 decompressFile(optarg, "output.txt");
//                 auto end = chrono::high_resolution_clock::now();
//                 chrono::duration<double, milli> duration = end - start;
//                 cout << "The decompression took " << duration.count() << " milliseconds\n";
//                 break;
//             }
//             default:
//                 cerr << "Unknown option\n";
//                 exit(EXIT_FAILURE);
//                 break;
//         }
//     }

//     return 0;
// }