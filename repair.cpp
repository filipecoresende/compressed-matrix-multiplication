#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <cmath>

#include <cassert>

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
    list<PairRecord>::iterator PairRecordIterator;
    bool flag;
} HashTableEntry;

using HashTable = unordered_map<Pair, HashTableEntry, PairHash, PairEqual>;














void readFileAndStoreCharacters(const string& filename, vector<VectorElement>& vector);
void incrementPairCountFirstIteration(VectorElement& vectorElement, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void iterateFirstTime(vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void rearrangePriorityQueue(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable);
VectorElement* getMostFrequentPair(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector);
Pair getPairFromPosition(VectorElement* location, vector<VectorElement>& symbolVector);
VectorElement* replacePairs(VectorElement* maxPairFirstAppearance, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, int newSymbol);
VectorElement* getNextNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector);
VectorElement* getPreviousNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector);
void pairContextHandler(VectorElement* ptr, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, int newSymbol);
void decreasePairCount(const Pair& pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void createPairRecordIfNecessary(const Pair& newPair, const Pair& auxPair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector);
void handleNewPairs(VectorElement* ptr, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, Pair replacedPair);














//reads from a text file and produces the first version of the symbol vector
void readFileAndStoreCharacters(const string& filename, vector<VectorElement>& vector) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    char ch;

    while (file.get(ch)) {
        int charAsInt = static_cast<int>(ch);
        vector.emplace_back(VectorElement{nullptr, charAsInt, nullptr});
    }

    file.close();
}


//Increments the count of a pair and links it to the next occurrence of the 
// pair in the symbol vector
void incrementPairCountFirstIteration(VectorElement& vectorElement, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
    
    const VectorElement& elementOnTheRight = *(&vectorElement + 1);
    int leftSymbol = vectorElement.symbol;
    int rightSymbol = elementOnTheRight.symbol;
    const Pair pair = {leftSymbol, rightSymbol};
    auto hashTableIterator = hashTable.find(pair);

    if (hashTableIterator == hashTable.end()){
        //inserts a new pair record in the first list of the priority queue
        priorityQueueVector[0].emplace_front(PairRecord{&vectorElement, 1});
        hashTable.emplace(pair, HashTableEntry{priorityQueueVector[0].begin(), false});
        return;
    }

    PairRecord& pairRecord = *(hashTableIterator->second.PairRecordIterator);
    pairRecord.pairCount++;
    VectorElement& nextOcurrenceOfThePair = *pairRecord.firstAppearance; 
    nextOcurrenceOfThePair.prev = &vectorElement; 
    vectorElement.next = &nextOcurrenceOfThePair;
    pairRecord.firstAppearance = &vectorElement;
}

//performs the first iteration (it's done backwards) through the symbol vector 
//and records the counts of the pairs.
void iterateFirstTime(vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){

     for (auto it = symbolVector.rbegin() + 1; it != symbolVector.rend(); ++it){
        incrementPairCountFirstIteration(*it, hashTable, priorityQueueVector);
     }


 }

//Puts each pair record produced in the first iteration in the appropriate list of the priority queue
void rearrangePriorityQueue(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable){
    auto& initialList = priorityQueueVector[0];
    auto it = hashTable.begin();
    while (it != hashTable.end()){
        const auto& pairRecordIterator = it->second.PairRecordIterator;
        int count = pairRecordIterator->pairCount;
        if (count == 1){
            initialList.erase(pairRecordIterator);
            it = hashTable.erase(it);
        }
        else if (count == 2)
            ++it;
        else if (count < priorityQueueVector.size() + 1) {
            auto& destinationList = priorityQueueVector[count - 2];
            destinationList.splice(destinationList.end(), initialList, pairRecordIterator);
            ++it;
        }
        else {
            auto& destinationList = priorityQueueVector[priorityQueueVector.size() - 1];
            destinationList.splice(destinationList.end(), initialList, pairRecordIterator);
            ++it;
        }
    }

}

VectorElement* getMostFrequentPair(vector<list<PairRecord>>& priorityQueueVector, HashTable& hashTable, vector<VectorElement>& symbolVector){
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

    if (itMaxPair->pairCount == 631169)
        cout << 1 << endl;
    

    VectorElement *firstAppearanceMaxPair = itMaxPair->firstAppearance;

    listWithMaxPair.erase(itMaxPair); 

    const Pair maxPair = getPairFromPosition(firstAppearanceMaxPair, symbolVector);
    auto itHashTable = hashTable.find(maxPair);


    hashTable.erase(itHashTable);

    return firstAppearanceMaxPair;

}

Pair getPairFromPosition(VectorElement* location, vector<VectorElement>& symbolVector){
    int leftSymbol = location->symbol;
    VectorElement* rightSymbolLocation = getNextNonEmptyPosition(location, symbolVector);
    int rightSymbol = rightSymbolLocation->symbol;
    return Pair{leftSymbol, rightSymbol};

}

VectorElement* replacePairs(VectorElement* maxPairFirstAppearance, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, int newSymbol){

    auto currentPosition = maxPairFirstAppearance;
    VectorElement* lastOccurrence; //holds the last occurrence of the pair being replaced
    while(currentPosition != nullptr){
        if (currentPosition->symbol == EMPTY){
            currentPosition = currentPosition->next;
        }

        if (currentPosition == nullptr) break;

        pairContextHandler(currentPosition, symbolVector, hashTable, priorityQueueVector, newSymbol);

        currentPosition->symbol = newSymbol; 
        

        VectorElement* rightSymbolPosition = getNextNonEmptyPosition(currentPosition, symbolVector);
        rightSymbolPosition->symbol = EMPTY;
        VectorElement *aux = getNextNonEmptyPosition(rightSymbolPosition, symbolVector);
        (currentPosition+1)->next = aux;

        if (aux != nullptr)
            (aux-1)->prev = currentPosition;
        lastOccurrence = currentPosition;
        currentPosition = currentPosition->next;        
    } 


    return lastOccurrence;
}

//returns nullptr in case the current position is the last non-empty position in the symbol vector
VectorElement* getNextNonEmptyPosition(VectorElement* currentPosition, vector<VectorElement>& symbolVector){ //current position is never the last in the vector
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

void pairContextHandler(VectorElement* ptr, vector<VectorElement>& symbolVector, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, int newSymbol){
    //Suppose the pair being replace is "ab", and its context in a given appearance is "xaby"
    //The counts of pairs xa and by must be decreased
    //Records for the new pairs xA and Ay (A is the new symbol) must be created if they appear at least twice in the sequence,
    //but the number of occurrences for them will not be counted yet.


    
    VectorElement* pairContext[4];
    pairContext[0] = getPreviousNonEmptyPosition(ptr, symbolVector);
    pairContext[1] = ptr;
    pairContext[2] = getNextNonEmptyPosition(pairContext[1], symbolVector);
    pairContext[3] = getNextNonEmptyPosition(pairContext[2], symbolVector);



    

    if (pairContext[0] != nullptr){
        VectorElement* previousOcurrence = pairContext[0]->prev;
        VectorElement* nextOcurrence = pairContext[0]->next;
        if (previousOcurrence != nullptr) previousOcurrence->next = nextOcurrence;
        else {
            Pair pair = Pair{pairContext[0]->symbol, pairContext[1]->symbol};
            auto itHashTable = hashTable.find(pair);
            if (itHashTable != hashTable.end()){
                itHashTable->second.PairRecordIterator->firstAppearance = nextOcurrence;
            }
        }
        if (nextOcurrence != nullptr) nextOcurrence->prev = previousOcurrence;
        Pair pairThatDisappear = {pairContext[0]->symbol, pairContext[1]->symbol};
        

        decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector); //decrease the count for "xa"
        Pair newPair = {pairContext[0]->symbol, newSymbol};
        createPairRecordIfNecessary(newPair, pairThatDisappear,  hashTable, priorityQueueVector); //check if a record must be created for xA
        
    }
    

    if (pairContext[3] != nullptr){
        VectorElement* previousOcurrence = pairContext[2]->prev;
        VectorElement* nextOcurrence = pairContext[2]->next;
        if (previousOcurrence != nullptr) previousOcurrence->next = nextOcurrence;
        else {
            Pair pair = Pair{pairContext[2]->symbol, pairContext[3]->symbol};
            auto itHashTable = hashTable.find(pair);
            if (itHashTable != hashTable.end())
                itHashTable->second.PairRecordIterator->firstAppearance = nextOcurrence;
        }
        if (nextOcurrence != nullptr) nextOcurrence->prev = previousOcurrence;
        Pair pairThatDisappear = {pairContext[2]->symbol, pairContext[3]->symbol};
        decreasePairCount(pairThatDisappear, hashTable, priorityQueueVector);//decrease the count for "by"
        Pair newPair = {newSymbol, pairContext[3]->symbol};
        createPairRecordIfNecessary(newPair, pairThatDisappear, hashTable, priorityQueueVector); //check if a record must be created for Ay
    }
    
     
}


void decreasePairCount(const Pair& pair, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector){
    auto itHashTable = hashTable.find(pair);
    if (itHashTable == hashTable.end())
        return;


    auto itPairRecord = itHashTable->second.PairRecordIterator;


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

    flag = !flag;

}

//Iterates through the vector backwards
void handleNewPairs(VectorElement* ptr, HashTable& hashTable, vector<list<PairRecord>>& priorityQueueVector, vector<VectorElement>& symbolVector, Pair replacedPair){
    VectorElement* currentPosition = ptr;

    while (currentPosition != nullptr){

        if (currentPosition->symbol == EMPTY){
            currentPosition = currentPosition->prev;
            continue;
        }



        VectorElement* elementOnTheLeft = getPreviousNonEmptyPosition(currentPosition, symbolVector);
        Pair auxPair; //it's used to assign "false" to the flags
        if (elementOnTheLeft != nullptr){
            auxPair = Pair{elementOnTheLeft->symbol, replacedPair.first};
            auto itAux = hashTable.find(auxPair);
            if (itAux != hashTable.end())
                itAux->second.flag = false;

            elementOnTheLeft->prev = nullptr;
            elementOnTheLeft->next = nullptr; 
            Pair pair = {elementOnTheLeft->symbol, currentPosition->symbol};
            auto itHashTable = hashTable.find(pair);
            if (itHashTable != hashTable.end()){
                auto itPairRecord = itHashTable->second.PairRecordIterator;
                auto &pairRecord = *itPairRecord;
                elementOnTheLeft->next = pairRecord.firstAppearance;
                if (pairRecord.firstAppearance != nullptr)
                    (pairRecord.firstAppearance)->prev = elementOnTheLeft;
                pairRecord.firstAppearance = elementOnTheLeft;
                auto& count = pairRecord.pairCount;
                count++;
                if (count > 2 && count < (priorityQueueVector.size() + 2)){
                    auto& listWithPair = priorityQueueVector[count - 3];
                    auto& destinationList = priorityQueueVector[count - 2];
                    destinationList.splice(destinationList.end(), listWithPair, itPairRecord);
                }
            }
        }


        VectorElement* aux = currentPosition->prev; 
        currentPosition->prev = nullptr; 
        currentPosition->next = nullptr; 
        VectorElement* elementOnTheRight = getNextNonEmptyPosition(currentPosition, symbolVector);
        if (elementOnTheRight != nullptr){
            auxPair = Pair{replacedPair.second, elementOnTheRight->symbol};
            auto itAux = hashTable.find(auxPair);
            if (itAux != hashTable.end())
                itAux->second.flag = false;

            Pair pair = {currentPosition->symbol, elementOnTheRight->symbol};
            auto itHashTable = hashTable.find(pair);
            if (itHashTable != hashTable.end()){
                auto itPairRecord = itHashTable->second.PairRecordIterator;
                auto &pairRecord = *itPairRecord;
                currentPosition->next = pairRecord.firstAppearance;
                if (pairRecord.firstAppearance != nullptr)
                    pairRecord.firstAppearance->prev = currentPosition;
                pairRecord.firstAppearance = currentPosition;
                auto& count = pairRecord.pairCount;
                count++;
                if (count > 2 && count < (priorityQueueVector.size() + 2)){
                    auto& listWithPair = priorityQueueVector[count - 3];
                    auto& destinationList = priorityQueueVector[count - 2];
                    destinationList.splice(destinationList.end(), listWithPair, itPairRecord);
                }
            }
        }

        currentPosition = aux;
    }




    // int sum = 0;
    // for (VectorElement* i = &symbolVector[0]; i != &symbolVector[0] + symbolVector.size(); i = i+1){
    //     auto leftPosition = i;
    //     if (i == nullptr) continue;
    //     auto rightPosition = getNextNonEmptyPosition(i, symbolVector);
    //     if (rightPosition == nullptr) break;
    //     int left = leftPosition->symbol; int right = rightPosition->symbol;
    //     if (left == -38 && right == -49) sum++;
    // }
    // cout << sum << endl;


}

































int main(){

    

    const string filename = "dataset/dna50MB.txt"; 
    vector<VectorElement> symbolVector;

    readFileAndStoreCharacters(filename, symbolVector);


    
    size_t priorityQueueVectorSize = ceil(sqrt(symbolVector.size()));
    HashTable hashTable;
    vector<list<PairRecord>> priorityQueueVector(priorityQueueVectorSize);

    iterateFirstTime(symbolVector, hashTable, priorityQueueVector);    
    
    rearrangePriorityQueue(priorityQueueVector, hashTable);

    int newSymbol = 0;
    auto maxPairFirstAppearance = getMostFrequentPair(priorityQueueVector, hashTable, symbolVector);
    while (maxPairFirstAppearance != nullptr){

        newSymbol--;
        auto maxPair = getPairFromPosition(maxPairFirstAppearance, symbolVector);
        
        auto lastOccurrence = replacePairs(maxPairFirstAppearance, hashTable, priorityQueueVector, symbolVector, newSymbol);


        handleNewPairs(lastOccurrence, hashTable, priorityQueueVector, symbolVector, maxPair);

        maxPairFirstAppearance = getMostFrequentPair(priorityQueueVector, hashTable, symbolVector);
            
    }

    
    // int newSymbol = 0;
    // while (maxPairFirstAppearance != nullptr){
    //     newSymbol--;
    //     VectorElement* lastOccurrence = replacePairs(maxPairFirstAppearance, hashTable, priorityQueueVector, symbolVector, newSymbol);
    //     handleNewPairs(lastOccurrence, hashTable, priorityQueueVector, symbolVector);
    //     break;
    // }
    
    
    
    

    // while (maxPairFirstAppearance->next != nullptr){
    //     cout << static_cast<char>(maxPairFirstAppearance->symbol)<< static_cast<char>((maxPairFirstAppearance+1)->symbol) << endl;
    //     maxPairFirstAppearance = maxPairFirstAppearance->next;
    // }

    
    
    //replacePairs(maxPairFirstAppearance, hashTable, priorityQueueVector, newSymbol);

    // cout << "testando  symbol vector" << endl;
    // for (const auto& i : symbolVector)
    // {   
    //     if (i.symbol < 0){
    //         cout << i.symbol;
    //     }
    //     else cout << static_cast<char>(i.symbol);      
    // }
    
    // cout << "testando a hash table" << endl;
    // for (const auto& element: hashTable){
    //     const auto& pairRecord = *(element.second.PairRecordIterator);
    //     if (element.first.first > 0) cout << '('<< static_cast<char>(element.first.first);
    //     else cout << '('<< element.first.first;

    //     if (element.first.second > 0) cout << ", " << static_cast<char>(element.first.second) << "): ";
    //     else cout << ", " << element.first.second << "): ";

    //     cout << pairRecord.pairCount << endl;
    // }

    // cout << "O tamanho é " << priorityQueueVectorSize << endl;
    // cout << "testando a priority queue" << endl;
    // for (const auto& element: priorityQueueVector[9]){
    //     cout << element.pairCount << " \n";
    // }

    // cout << "testando os links entre pares idênticos" << endl;
    // VectorElement *ptr = &symbolVector[44];
    // while (ptr->next!=nullptr){
    //     cout << static_cast<char>(ptr->symbol) << "," << static_cast<char>((ptr+ 1)->symbol) << '\n';
    //     ptr = ptr->next;
    // }

    // while (ptr!=nullptr){
    //     cout << static_cast<char>(ptr->symbol) << "," << static_cast<char>((ptr+ 1)->symbol) << '\n';
    //     ptr = ptr->prev;
    // }



    // cout << "testando os espacos vazios" << endl;
    // for (auto ptr = &symbolVector[0]; ptr != &symbolVector[0] + symbolVector.size();){
    //     if (ptr->symbol == EMPTY){
    //         ptr = ptr->next;
    //     }
    //     if (ptr == nullptr) break;
    //     cout << ptr->symbol << " ";
    //     ptr = ptr+1;
    // }    


    // cout << "imprimindo tudo" << endl;
    // for (auto& i: symbolVector) cout << i.symbol << " ";

    return 0;
}



