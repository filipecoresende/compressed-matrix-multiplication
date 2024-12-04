#ifndef REPAIR_H
#define REPAIR_H

#include <list>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <utility>

using namespace std;

#define EMPTY INT32_MAX

using Pair = pair<int, int>;

typedef struct VectorElement
{
    int prev;
    int symbol;
    int next;
} VectorElement;

typedef struct PairRecord
{
    int firstAppearance;
    int pairCount;
} PairRecord;

// Custom hash function for Pair
struct PairHash
{
    size_t operator()(const Pair &p) const
    {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};

// Custom equality function for Pair
struct PairEqual
{
    bool operator()(const Pair &p1, const Pair &p2) const
    {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

typedef struct
{
    list<PairRecord>::iterator pairRecordIterator;
    bool flag;
} HashTableEntry;

using HashTable = unordered_map<Pair, HashTableEntry, PairHash, PairEqual>;

void readFileAndStoreCharacters(const string &filename, vector<VectorElement> &symbolVector);
void incrementPairCountFirstIteration(int currentPosition, vector<VectorElement> &symbolVector, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
void iterateFirstTime(vector<VectorElement> &symbolVector, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
void removeUnnecessaryRecords(vector<list<PairRecord>> &priorityQueueVector, HashTable &hashTable, vector<VectorElement> &symbolVector);
int getMostFrequentPair(vector<list<PairRecord>> &priorityQueueVector, HashTable &hashTable, vector<VectorElement> &symbolVector, vector<Pair> &grammar);
Pair getPairFromPosition(int position, vector<VectorElement> &symbolVector);
int replacePairs(int maxPairFirstAppearance, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector, vector<VectorElement> &symbolVector, int newSymbol);
int getNextNonEmptyPosition(int currentPosition, vector<VectorElement> &symbolVector);
int getPreviousNonEmptyPosition(int currentPosition, vector<VectorElement> &symbolVector);
int pairContextHandler(int ptr, vector<VectorElement> &symbolVector, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector, int newSymbol);
void decreasePairCount(const Pair &pair, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
void createPairRecordIfNecessary(const Pair &newPair, const Pair &auxPair, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
void handleNewPairs(int ptr, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector, vector<VectorElement> &symbolVector, Pair replacedPair);
void clearFlags(int *pairContext, vector<VectorElement> &symbolVector, HashTable &hashTable, Pair replacedPair);
void decrementOldPairs(int *pairContext, vector<VectorElement> &symbolVector, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector, Pair replacedPair);
int newPairs(int *pairContext, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector, vector<VectorElement> &symbolVector);
vector<vector<int>> expander(vector<Pair> &grammar);
vector<int> expandRule(vector<vector<int>> &expansions, Pair rule);
size_t writeBinaryFile(const string &inputFilename, vector<VectorElement> &symbolVector, vector<Pair> &grammar);
string changeExtension(const string &originalFilename, const string &newExtension);
void decompressFile(const string &inputFilename, const string &outputFilename);
vector<int> decompressor(vector<int> &vectorOfIntegers, vector<Pair> &grammar);
void convertIntArrayToTextFile(vector<int> &array, const string &outputFilename);

PairRecord *incrementPairCount(const Pair pair, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
void compressFile(const string &filename);
size_t compressor(const string &inputFilename, vector<VectorElement> &symbolVector);

vector<int> expandNonTerminal(vector<int> &originalVector, int positionOriginalVector, int nonTerminal, vector<Pair> &grammar, vector<Pair> &alreadyExpandedNonTerminals);
void handleDisappearingPair(Pair disappearingPair, int currentPosition, vector<VectorElement> &symbolVector, HashTable &hashTable, vector<list<PairRecord>> &priorityQueueVector);
pair<vector<int>, vector<Pair>> getSequenceAndGrammar(const string &inputFilename);

#endif
