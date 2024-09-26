#include <iostream>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <unistd.h>

#define EMPTY 0

using namespace std;

using Pair = pair<int, int>;

typedef struct VectorElement{
    VectorElement *prev;
    int symbol;
    VectorElement *next;
} VectorElement;

typedef struct PairRecord{
    VectorElement *firstAppearance;
    int pairCount;
} PairRecord;


// Custom hash function for Pair
struct PairHash {
    size_t operator()(const Pair& p) const {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};

// Custom equality function for Pair
struct PairEqual {
    bool operator()(const Pair& p1, const Pair& p2) const {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

typedef struct{
    list<PairRecord>::iterator pairRecordIterator;
    bool flag;
} HashTableEntry;

using HashTable = unordered_map<Pair, HashTableEntry, PairHash, PairEqual>;


void readFileAndStoreCharacters(const string& filename, vector<VectorElement>& symbolVector);
void incrementPairCountFirstIteration(VectorElement& currentPosition, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void iterateFirstTime(vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void removeUnnecessaryRecords(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector);
VectorElement* getMostFrequentPair(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector, vector<Pair>& grammar);
Pair getPairFromPosition(VectorElement* position, vector<VectorElement>& symbolVector);
VectorElement* replacePairs(VectorElement* maxPairFirstAppearance, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, int newSymbol);
VectorElement* getNextNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector);
VectorElement* getPreviousNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector);
VectorElement* pairContextHandler(VectorElement* ptr, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, int newSymbol);
void decreasePairCount(const Pair& pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void createPairRecordIfNecessary(const Pair& newPair, const Pair& auxPair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void handleNewPairs(VectorElement* ptr, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, Pair replacedPair);
void clearFlags(VectorElement** pairContext, HashTable& hashTable, Pair replacedPair); 
void decrementOldPairs(VectorElement** pairContext, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, Pair replacedPair);
VectorElement* newPairs(VectorElement** pairContext, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
vector<vector<int>> expander(vector<Pair>& grammar);
vector<int> expandRule(vector<vector<int>>& expansions, Pair rule);
void writeBinaryFile(const string& inputFilename, vector<VectorElement>& symbolVector, vector<Pair>& grammar);
string changeExtension(const string& originalFilename, const string& newExtension);
void decompressFile(const string& inputFilename, string outputFilename);
vector<int> decompressor(vector<int>& vectorOfIntegers, vector<Pair>& grammar);
void convertIntArrayToTextFile(vector<int>& array, const string& outputFilename);

PairRecord* incrementPairCount(const Pair pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void compressFile(const string& filename);
vector<int> decompressorLento(vector<int>& vectorOfIntegers, vector<Pair>& grammar);
vector<int> expandNonTerminal(int vectorElement, vector<Pair>& grammar);









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
        symbolVector.emplace_back(VectorElement{nullptr, charAsInt, nullptr});
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
void incrementPairCountFirstIteration(VectorElement& currentPosition, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
    
    const VectorElement& elementOnTheRight = *(&currentPosition + 1);
    int leftSymbol = currentPosition.symbol;
    int rightSymbol = elementOnTheRight.symbol;
    const Pair pair = {leftSymbol, rightSymbol};
    auto hashTableIterator = hashTable.find(pair);

    if (hashTableIterator == hashTable.end()){
        //inserts a new pair record in the first list of the priority queue
        priorityQueueVector[0].emplace_front(PairRecord{&currentPosition, 1});
        hashTable.emplace(pair, HashTableEntry{priorityQueueVector[0].begin(), false});
        return;
    }

    PairRecord& pairRecord = *(hashTableIterator->second.pairRecordIterator);
    pairRecord.pairCount++;
    VectorElement* nextOccurrenceOfThePair = pairRecord.firstAppearance; 
    nextOccurrenceOfThePair->prev = &currentPosition; 
    currentPosition.next = nextOccurrenceOfThePair;
    pairRecord.firstAppearance = &currentPosition;
}

//performs the first iteration (it's done backwards) through the symbol vector 
//and records the counts of the pairs.
void iterateFirstTime(vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){


    //  for (auto it = symbolVector.rbegin() + 1; it != symbolVector.rend(); ++it){
    //     incrementPairCountFirstIteration(*it, hashTable, priorityQueueVector);
    //  }

    for (int i = symbolVector.size() - 2; i >= 0; i--){
        Pair pair = getPairFromPosition(&symbolVector[i], symbolVector);
        PairRecord* pairRecordPointer = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecordPointer == nullptr){
            PairRecord newPairRecord;
            newPairRecord.pairCount = 1; 
            newPairRecord.firstAppearance = &symbolVector[i];
            priorityQueueVector[0].emplace_front(newPairRecord);
            hashTable.emplace(pair, HashTableEntry{priorityQueueVector[0].begin(), false});
        }
        else{
            symbolVector[i].next = pairRecordPointer->firstAppearance;
            pairRecordPointer->firstAppearance->prev = &symbolVector[i];
            pairRecordPointer->firstAppearance = &symbolVector[i];
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



VectorElement* getMostFrequentPair(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector, vector<Pair>& grammar){
    int i;
    
    for (i = priorityQueueVector.size() - 1; i >= 0; i--){
        if (!priorityQueueVector[i].empty())
            break;
    }

    if (i == -1)
        return nullptr;


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
    

    VectorElement *firstAppearanceMaxPair = itMaxPair->firstAppearance;
    
    listWithMaxPair.erase(itMaxPair); 

    const Pair maxPair = getPairFromPosition(firstAppearanceMaxPair, symbolVector);


    auto itHashTable = hashTable.find(maxPair);

    hashTable.erase(itHashTable);

    grammar.emplace_back(maxPair);

    return firstAppearanceMaxPair;

}

Pair getPairFromPosition(VectorElement* position, vector<VectorElement>& symbolVector){ //'position' cannot be the last nonempty position
    int leftSymbol = position->symbol;
    VectorElement* rightSymbolLocation = getNextNonEmptyPosition(position, symbolVector);
    int rightSymbol = rightSymbolLocation->symbol;
    return Pair{leftSymbol, rightSymbol};

}

VectorElement* replacePairs(VectorElement* maxPairFirstAppearance, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, int newSymbol){

    VectorElement *currentPosition = maxPairFirstAppearance;
    VectorElement *lastOccurrence; //holds the last occurrence of the pair being replaced
    while(currentPosition != nullptr){

        VectorElement *nextPosition = pairContextHandler(currentPosition, symbolVector, hashTable, priorityQueueVector, newSymbol);
        
        lastOccurrence = currentPosition;
        currentPosition = nextPosition;        
    } 

    return lastOccurrence;
}

//returns nullptr in case the current position is the last non-empty position in the symbol vector
VectorElement* getNextNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector){ 
    if (currentPosition == &symbolVector[symbolVector.size() - 1])
        return nullptr;
    VectorElement *nextNonEmptyPosition = currentPosition + 1;
    if (nextNonEmptyPosition->symbol == EMPTY)
        nextNonEmptyPosition = nextNonEmptyPosition->next;
    

    return nextNonEmptyPosition;
}

//returns nullptr in case the current position is the first non-empty position in the symbol vector
VectorElement* getPreviousNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector){
    if (currentPosition == &symbolVector[0])
        return nullptr;
    VectorElement *previousNonEmptyPosition = currentPosition - 1;
    if (previousNonEmptyPosition->symbol == EMPTY)
        previousNonEmptyPosition = previousNonEmptyPosition->prev;
    return previousNonEmptyPosition;
}

VectorElement* pairContextHandler(VectorElement* ptr, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, int newSymbol){
    //Suppose the pair being replace is "ab", and its context in a given appearance is "xaby"
    //Records for the new pairs xA and Ay (A is the new symbol) must be created if they appear at least twice in the sequence,
    //but the number of occurrences for them will not be counted yet.

    VectorElement* pairContext[4];
    pairContext[0] = getPreviousNonEmptyPosition(ptr, symbolVector);
    pairContext[1] = ptr;
    pairContext[2] = getNextNonEmptyPosition(pairContext[1], symbolVector);
    pairContext[3] = getNextNonEmptyPosition(pairContext[2], symbolVector);


    if (pairContext[0] != nullptr && pairContext[0]->symbol != newSymbol){
        Pair auxPair = {pairContext[0]->symbol, pairContext[1]->symbol}; 
        Pair newPair = {pairContext[0]->symbol, newSymbol};
        createPairRecordIfNecessary(newPair, auxPair,  hashTable, priorityQueueVector); //check if a record must be created for xA

        Pair pairThatDisappear = {pairContext[0]->symbol, pairContext[1]->symbol};

        VectorElement* previousOccurrence = pairContext[0]->prev;
        VectorElement* nextOccurrence = pairContext[0]->next;
        if (previousOccurrence != nullptr) previousOccurrence->next = nextOccurrence;
        else {
            auto itHashTable = hashTable.find(pairThatDisappear);
            if (itHashTable != hashTable.end()){
                itHashTable->second.pairRecordIterator->firstAppearance = nextOccurrence;
            }
        }
        if (nextOccurrence != nullptr) nextOccurrence->prev = previousOccurrence;
    }

    if (pairContext[3] != nullptr){

        if (pairContext[1]->symbol != pairContext[3]->symbol){
            Pair auxPair = {pairContext[2]->symbol, pairContext[3]->symbol};
            Pair newPair = {newSymbol, pairContext[3]->symbol};
            createPairRecordIfNecessary(newPair, auxPair, hashTable, priorityQueueVector); //check if a record must be created for Ay
        }

        VectorElement* previousOccurrence = pairContext[2]->prev;
        VectorElement* nextOccurrence = pairContext[2]->next;
        if (previousOccurrence != nullptr) previousOccurrence->next = nextOccurrence;
        else {
            Pair pairThatDisappear = Pair{pairContext[2]->symbol, pairContext[3]->symbol};
            auto itHashTable = hashTable.find(pairThatDisappear);
            if (itHashTable != hashTable.end()){
                itHashTable->second.pairRecordIterator->firstAppearance = nextOccurrence;
            }
        }
        if (nextOccurrence != nullptr) nextOccurrence->prev = previousOccurrence;

    }

    VectorElement* nextPosition = (pairContext[1]->next != pairContext[2]) ? pairContext[1]->next :  pairContext[2]->next; //acho que n precisa disso

    pairContext[1]->symbol = newSymbol; 

    pairContext[2]->symbol = EMPTY; 
    (pairContext[1] + 1)->next = pairContext[3];

    if (pairContext[3] != nullptr)
        (pairContext[3]-1)->prev = pairContext[1];
           
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
        priorityQueueVector[0].emplace_front(PairRecord{nullptr, 0});
        hashTable.emplace(newPair, HashTableEntry{priorityQueueVector[0].begin(), false});
    }
    else flag = !flag;

}

//Iterates through the vector backwards
void handleNewPairs(VectorElement* ptr, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, Pair replacedPair){    
    VectorElement* currentPosition = ptr;
    VectorElement* pairContext[4];

    while (currentPosition != nullptr){

        pairContext[0] = getPreviousNonEmptyPosition(currentPosition, symbolVector);
        pairContext[1] = currentPosition;
        pairContext[2] = getNextNonEmptyPosition(pairContext[1], symbolVector);


        clearFlags(&pairContext[0], hashTable, replacedPair);
        decrementOldPairs(&pairContext[0], hashTable, priorityQueueVector, replacedPair);

        currentPosition = newPairs(&pairContext[0], hashTable, priorityQueueVector);
        
    }


}


VectorElement* newPairs(VectorElement** pairContext, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){

    if (pairContext[0] != nullptr && pairContext[0]->symbol != pairContext[1]->symbol ){
        pairContext[0]->prev = nullptr;
        pairContext[0]->next = nullptr;    
        Pair pair = {pairContext[0]->symbol, pairContext[1]->symbol};
        PairRecord *pairRecord = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecord != nullptr){
            pairContext[0]->next = pairRecord->firstAppearance;
            if (pairRecord->firstAppearance != nullptr)
                pairRecord->firstAppearance->prev = pairContext[0];
            pairRecord->firstAppearance = pairContext[0];
        }
    }


    VectorElement* aux = pairContext[1]->prev;  
    if (pairContext[2] != nullptr){
        pairContext[1]->prev = nullptr; 
        pairContext[1]->next = nullptr;
        Pair pair = {pairContext[1]->symbol, pairContext[2]->symbol};
        PairRecord *pairRecord = incrementPairCount(pair, hashTable, priorityQueueVector);
        if (pairRecord != nullptr){
            pairContext[1]->next = pairRecord->firstAppearance;
            if (pairRecord->firstAppearance != nullptr)
                pairRecord->firstAppearance->prev = pairContext[1];
            pairRecord->firstAppearance = pairContext[1];
        }
    }

    if (aux != nullptr && aux->symbol == EMPTY){  //acho que nao precisa disso
            aux = aux->prev;
    }
    
    return aux;
}


void clearFlags(VectorElement** pairContext, HashTable& hashTable, Pair replacedPair){
    if (pairContext[0] != nullptr && pairContext[0]->symbol != pairContext[1]->symbol){
        Pair auxPair = Pair{pairContext[0]->symbol, replacedPair.first}; 
        auto itAux = hashTable.find(auxPair);
        if (itAux != hashTable.end())
            itAux->second.flag = false;
    }

    if (pairContext[2] != nullptr && pairContext[1]->symbol != pairContext[2]->symbol){
        Pair auxPair = Pair{replacedPair.second, pairContext[2]->symbol}; 
        auto itAux = hashTable.find(auxPair);
        if (itAux != hashTable.end())
            itAux->second.flag = false;
    }

}

void decrementOldPairs(VectorElement** pairContext, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, Pair replacedPair){
    if (pairContext[0] != nullptr && pairContext[0]->symbol != pairContext[1]->symbol){
        Pair pairThatDisappear = {pairContext[0]->symbol, replacedPair.first};
        decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector); //talvez dê pra já aproveitar o itHashTable de antes 
    }

    if (pairContext[2] != nullptr){
        if (pairContext[1]->symbol != pairContext[2]->symbol){
            Pair pairThatDisappear = {replacedPair.second, pairContext[2]->symbol};
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
    size_t expansionsSize = 0;
    ofstream file("aux.txt");
    for (int i = 0; i < grammar.size(); i++){
        expansions[i] = expandRule(expansions, grammar[i]);
        expansionsSize += expansions[i].size() ;
        file << expansionsSize*sizeof(int) << endl;

    }
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

    vector<int8_t> outputVector_int8;
    vector<int16_t> outputVector_int16;
    vector<int32_t> outputVector_int32;
    string newExtension; //extension of the binary file to be generated

    int numRules = grammar.size(); //number of rules

    int intSize; //stores how many bits are required for the "biggest" (or smallest) integer
    if ((-numRules) >= INT8_MIN){
        intSize = 8;
        newExtension = ".re8";
    }
    else if ((-numRules) >= INT16_MIN){
        intSize = 16;
        newExtension = ".re16";
    } 
    else {
        intSize = 32;
        newExtension = ".re32";
    } 

    string outputFilename = changeExtension(inputFilename, newExtension); 

    // Opens the output file for writing in binary mode
    ofstream outputFile(outputFilename, ios::binary);
    if (!outputFile.is_open()) {
        // Print an error message and exit if the file cannot be opened
        cerr << "Error: Could not open " << outputFilename << endl;
        exit(EXIT_FAILURE);
    }


    outputFile.write(reinterpret_cast<char*>(&intSize), sizeof(int));
    if (!outputFile.good()) {
        cerr << "Error writing intSize to " << outputFilename << endl;
        outputFile.close();
        exit(EXIT_FAILURE);
    }

    outputFile.write(reinterpret_cast<char*>(&numRules), sizeof(int));
    if (!outputFile.good()) {
        cerr << "Error writing numRules to " << outputFilename << endl;
        outputFile.close();
        exit(EXIT_FAILURE);
    }


    switch (intSize){ 
        case 8: 
            for (int i = 0; i < numRules; i++){
                outputVector_int8.emplace_back(static_cast<int8_t>(grammar[i].first));
                outputVector_int8.emplace_back(static_cast<int8_t>(grammar[i].second));
            }

            for (VectorElement* ptr = &symbolVector[0]; ptr != &symbolVector[0] + symbolVector.size(); ptr++){
                if (ptr->symbol == EMPTY){
                    ptr = ptr->next;
                    if (ptr == nullptr) break;
                } 
                outputVector_int8.emplace_back(static_cast<int8_t>(ptr->symbol));
            }             
            
            outputFile.write(reinterpret_cast<const char*>(outputVector_int8.data()), outputVector_int8.size() * sizeof(int8_t));
            if (!outputFile.good()) {
                cerr << "Error writing outputVector_int8 to " << outputFilename<< endl;
                outputFile.close(); // Ensure the file is closed on error
                exit(EXIT_FAILURE);
            }
            break;
        case 16:
            for (int i = 0; i < numRules; i++){
                outputVector_int16.emplace_back(static_cast<int16_t>(grammar[i].first));
                outputVector_int16.emplace_back(static_cast<int16_t>(grammar[i].second));
            }
            for (VectorElement* ptr = &symbolVector[0]; ptr != &symbolVector[0] + symbolVector.size(); ptr++){
                if (ptr->symbol == EMPTY){
                    ptr = ptr->next;
                    if (ptr == nullptr) break;
                } 
                outputVector_int16.emplace_back(static_cast<int16_t>(ptr->symbol));
            }             
            outputFile.write(reinterpret_cast<const char*>(outputVector_int16.data()), outputVector_int16.size() * sizeof(int16_t));
            if (!outputFile.good()) {
                cerr << "Error writing outputVector_int16 to " << outputFilename << endl;
                outputFile.close(); // Ensure the file is closed on error
                exit(EXIT_FAILURE);
            }
            break;
        case 32:
            for (int i = 0; i < numRules; i++){
                outputVector_int32.emplace_back(static_cast<int32_t>(grammar[i].first));
                outputVector_int32.emplace_back(static_cast<int32_t>(grammar[i].second));
            }
            for (VectorElement* ptr = &symbolVector[0]; ptr != &symbolVector[0] + symbolVector.size(); ptr++){
                if (ptr->symbol == EMPTY){
                    ptr = ptr->next;
                    if (ptr == nullptr) break;
                } 
                outputVector_int32.emplace_back(static_cast<int32_t>(ptr->symbol));
            }             
            outputFile.write(reinterpret_cast<const char*>(outputVector_int32.data()), outputVector_int32.size() * sizeof(int32_t));
            if (!outputFile.good()) {
                cerr << "Error writing outputVector_int32 to " << outputFilename << endl;
                outputFile.close(); // Ensure the file is closed on error
                exit(EXIT_FAILURE);
            }
            break;
        default:
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


void decompressFile(const string& inputFilename, string outputFilename) { 


    ifstream inputFile(inputFilename, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << inputFilename << endl;
        exit(EXIT_FAILURE);
    }

    int intSize;
    inputFile.read(reinterpret_cast<char*>(&intSize), sizeof(int));
    if (!inputFile.good()) {
        cerr << "Error reading intSize from " << inputFilename << endl;
        inputFile.close();
        exit(EXIT_FAILURE);
    }
    
    int numRules;
    inputFile.read(reinterpret_cast<char*>(&numRules), sizeof(int));
    if (!inputFile.good()) {
        cerr << "Error reading numRules from " << inputFilename << endl;
        inputFile.close();
        exit(EXIT_FAILURE);
    }

    vector<Pair> grammar;
    grammar.reserve(numRules);
    vector<int> V;

    switch (intSize){
        case 8:{
            int8_t value1, value2;
            for (int i = 0; i < numRules; i++){
                if (!inputFile.read(reinterpret_cast<char*>(&value1), sizeof(value1))) {
                    cerr << "Error reading value1 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                if (!inputFile.read(reinterpret_cast<char*>(&value2), sizeof(value2))) {
                    cerr << "Error reading value2 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                grammar.emplace_back(Pair{static_cast<int>(value1), static_cast<int>(value2)});
            }

            int8_t value;
            while(inputFile.read(reinterpret_cast<char*>(&value), sizeof(value)))
                V.emplace_back(static_cast<int>(value));
            if (!inputFile.eof()) {
                cerr << "Error reading V from " << inputFilename << endl;
                inputFile.close();
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 16:{
            int16_t value1, value2;
            for (int i = 0; i < numRules; i++){
                if (!inputFile.read(reinterpret_cast<char*>(&value1), sizeof(value1))) {
                    cerr << "Error reading value1 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                if (!inputFile.read(reinterpret_cast<char*>(&value2), sizeof(value2))) {
                    cerr << "Error reading value2 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                grammar.emplace_back(Pair{static_cast<int>(value1), static_cast<int>(value2)});
            }

            int16_t value;
            while(inputFile.read(reinterpret_cast<char*>(&value), sizeof(value)))
                V.emplace_back(static_cast<int>(value));
            if (!inputFile.eof()) {
                cerr << "Error reading V from " << inputFilename << endl;
                inputFile.close();
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 32:{
            int32_t value1, value2;
            for (int i = 0; i < numRules; i++){
                if (!inputFile.read(reinterpret_cast<char*>(&value1), sizeof(value1))) {
                    cerr << "Error reading value1 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                if (!inputFile.read(reinterpret_cast<char*>(&value2), sizeof(value2))) {
                    cerr << "Error reading value2 at rule " << i << endl;
                    exit(EXIT_FAILURE);
                }
                grammar.emplace_back(Pair{static_cast<int>(value1), static_cast<int>(value2)});
            }

            int32_t value;
            while(inputFile.read(reinterpret_cast<char*>(&value), sizeof(value)))
                V.emplace_back(static_cast<int>(value));
            if (!inputFile.eof()) {
                cerr << "Error reading V from " << inputFilename << endl;
                inputFile.close();
                exit(EXIT_FAILURE);
            }
            break;
        }
        
        default:
            cerr << "Invalid value for intSize " << endl;
            inputFile.close();
            exit(EXIT_FAILURE);
    }

    inputFile.close();

    vector<int> originalVector = decompressor(V, grammar);

    convertIntArrayToTextFile(originalVector, outputFilename);

}

vector<int> decompressor(vector<int>& vectorOfIntegers, vector<Pair>& grammar){
//reconstructs the original text as an array of integers

    vector<vector<int>> expansions = expander(grammar); //expand the non-terminal symbols
    vector<int> originalVector; //we want to reconstruct the original text
    originalVector.reserve(vectorOfIntegers.size());
    for (const auto& vectorElement: vectorOfIntegers){
        if (vectorElement < 0){
            for (const auto& expansionElement: expansions[-vectorElement-1]){
                originalVector.push_back(expansionElement);
            }
        }
        else 
            originalVector.push_back(vectorElement);
    }

    originalVector.shrink_to_fit();
    
    return originalVector;

}

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

    size_t priorityQueueVectorSize = ceil(sqrt(symbolVector.size()));

    HashTable hashTable;
    vector<list<PairRecord>> priorityQueueVector(priorityQueueVectorSize);

    vector<Pair> grammar;

    iterateFirstTime(symbolVector, hashTable, priorityQueueVector); 
    removeUnnecessaryRecords(priorityQueueVector, hashTable, symbolVector);   

    int newSymbol = 0;
    auto maxPairFirstAppearance = getMostFrequentPair(priorityQueueVector, hashTable, symbolVector, grammar);
  
    while (maxPairFirstAppearance != nullptr){

        newSymbol--;


        auto maxPair = getPairFromPosition(maxPairFirstAppearance, symbolVector);

        priorityQueueVector[0].emplace_front(PairRecord{nullptr, 0});
        hashTable.emplace(Pair{newSymbol, newSymbol}, HashTableEntry{priorityQueueVector[0].begin(), false});

        priorityQueueVector[0].emplace_front(PairRecord{nullptr, 0});
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

    writeBinaryFile(filename, symbolVector, grammar);
}

vector<int> decompressorLento(vector<int>& vectorOfIntegers, vector<Pair>& grammar){
    vector<int> originalVector; //we want to reconstruct the original text
    originalVector.reserve(vectorOfIntegers.size());
    for (const auto& vectorElement: vectorOfIntegers){
        if (vectorElement < 0){
            vector<int> expansion = expandNonTerminal(vectorElement, grammar);
            for (const auto& i: expansion)
                originalVector.push_back(i);
        }
        else 
            originalVector.push_back(vectorElement);
    }

    originalVector.shrink_to_fit();
    
    return originalVector;

}

vector<int> expandNonTerminal(int vectorElement, vector<Pair>& grammar){
    if (vectorElement >= 0){
        cerr << "vectorElement must be negative\n";
        exit(EXIT_FAILURE);
    }

    vector<int> expansion;

    int grammarIndex = -vectorElement - 1;

    int symbol1 = grammar[grammarIndex].first;
    int symbol2 = grammar[grammarIndex].second;

    if (symbol1 < 0){
        vector<int> aux = expandNonTerminal(symbol1, grammar);
        for (const auto& i: aux){
            expansion.push_back(i);
        }
    }
    else
        expansion.push_back(symbol1);

    if (symbol2 < 0){
        vector<int> aux = expandNonTerminal(symbol2, grammar);
        for (const auto& i: aux){
            expansion.push_back(i);
        }
    }
    else
        expansion.push_back(symbol2);

    return expansion;
}
































int main(int argc, char *argv[]){
    int option;
    while ((option = getopt(argc, argv, "c:d:")) != -1) {
        switch (option) {
            case 'c':{
                auto start = chrono::high_resolution_clock::now();
                compressFile(optarg);
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> duration = end - start;
                cout << "The compression took " << duration.count() << " milliseconds\n";
                break;
            }
            case 'd':{  
                auto start = chrono::high_resolution_clock::now();
                decompressFile(optarg, "output.txt");
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> duration = end - start;
                cout << "The decompression took " << duration.count() << " milliseconds\n";
                break;
            }
            default:
                cerr << "Unknown option\n";
                exit(EXIT_FAILURE);
                break;
        }
    }

    return 0;
}