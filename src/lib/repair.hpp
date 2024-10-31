#ifndef REPAIR_H
#define REPAIR_H

#include <list>

#define EMPTY INT32_MAX

using Pair = std::pair<int, int>;

typedef struct VectorElement{
    int prev;
    int symbol;
    int next;
} VectorElement;

typedef struct PairRecord{
    int firstAppearance;
    int pairCount;
} PairRecord;

// Custom hash function for Pair
struct PairHash {
    size_t operator()(const Pair& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

// Custom equality function for Pair
struct PairEqual {
    bool operator()(const Pair& p1, const Pair& p2) const {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

typedef struct{
    std::list<PairRecord>::iterator pairRecordIterator;
    bool flag;
} HashTableEntry;

using HashTable = std::unordered_map<Pair, HashTableEntry, PairHash, PairEqual>;


void readFileAndStoreCharacters(const std::string& filename, std::vector<VectorElement>& symbolVector);
void incrementPairCountFirstIteration(int currentPosition, std::vector<VectorElement>& symbolVector, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
void iterateFirstTime(std::vector<VectorElement>& symbolVector, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
void removeUnnecessaryRecords(std::vector<std::list<PairRecord>>& priorityQueueVector, HashTable& hashTable, std::vector<VectorElement>& symbolVector);
int getMostFrequentPair(std::vector<std::list<PairRecord>>& priorityQueueVector, HashTable& hashTable, std::vector<VectorElement>& symbolVector, std::vector<Pair>& grammar);
Pair getPairFromPosition(int position, std::vector<VectorElement>& symbolVector);
int replacePairs(int maxPairFirstAppearance, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector, std::vector<VectorElement>& symbolVector, int newSymbol);
int getNextNonEmptyPosition(int currentPosition, std::vector<VectorElement>& symbolVector);
int getPreviousNonEmptyPosition(int currentPosition, std::vector<VectorElement>& symbolVector);
int pairContextHandler(int ptr, std::vector<VectorElement>& symbolVector, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector, int newSymbol);
void decreasePairCount(const Pair& pair, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
void createPairRecordIfNecessary(const Pair& newPair, const Pair& auxPair, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
void handleNewPairs(int ptr, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector, std::vector<VectorElement>& symbolVector, Pair replacedPair);
void clearFlags(int* pairContext, std::vector<VectorElement>& symbolVector, HashTable& hashTable, Pair replacedPair); 
void decrementOldPairs(int* pairContext, std::vector<VectorElement>& symbolVector, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector, Pair replacedPair);
int newPairs(int* pairContext, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector, std::vector<VectorElement>& symbolVector);
std::vector<std::vector<int>> expander(std::vector<Pair>& grammar);
std::vector<int> expandRule(std::vector<std::vector<int>>& expansions, Pair rule);
void writeBinaryFile(const std::string& inputFilename, std::vector<VectorElement>& symbolVector, std::vector<Pair>& grammar);
std::string changeExtension(const std::string& originalFilename, const std::string& newExtension);
void decompressFile(const std::string& inputFilename, const std::string& outputFilename);
std::vector<int> decompressor(std::vector<int>& vectorOfIntegers, std::vector<Pair>& grammar);
void convertIntArrayToTextFile(std::vector<int>& array, const std::string& outputFilename);

PairRecord* incrementPairCount(const Pair pair, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
void compressFile(const std::string& filename);
void compressor(const std::string& inputFilename, std::vector<VectorElement>& symbolVector);

std::vector<int> expandNonTerminal(std::vector<int>& originalVector, int positionOriginalVector, int nonTerminal, std::vector<Pair>& grammar, std::vector<Pair>& alreadyExpandedNonTerminals);
void handleDisappearingPair(Pair disappearingPair, int currentPosition, std::vector<VectorElement>& symbolVector, HashTable& hashTable, std::vector<std::list<PairRecord>>& priorityQueueVector);
std::pair<std::vector<int>,std::vector<Pair>> getSequenceAndGrammar(const std::string& inputFilename);



#endif
