#include <iostream>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <cstdlib>
#include <cmath>

#include "lib/repair.hpp"
#include "lib/utils.hpp"

#define END_ROW 0

using namespace std;

time_t t_total=0;clock_t c_total=0;


size_t compressCSVFile(const string& filename, int numBlocks);
void reconstructCSVFile(const string& originalCSVFilename, int numBlocks);
int getNumberOfColumns(const string& csvFilename);
int encodePair(Pair pair, int numCol);
pair<vector<double>,vector<VectorElement>> generateCSRV(ifstream& file, int numCol, int blockSize);
void generateCSVFromCSRV(vector<int>& csrvMatrixDecompressed, int numCol, vector<double>& uniqueValues, ofstream& csvOutFile);
size_t writeAuxiliaryDataToBinaryFile(const string& inputFilename, int numCol, vector<double>& uniqueValues);
pair<int, vector<double>> readAuxiliaryData(const string& filename);
void print_usage(const char* program_name);

size_t compressCSVFile(const string& filename, int numBlocks){

  int numCol = getNumberOfColumns(filename);

  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Error: Could not open " << filename << endl;
    exit(EXIT_FAILURE);
  }

  size_t rows=0;
  string line;
  while (getline(file, line)) rows++;
  file.clear();
  file.seekg(0, std::ios_base::beg);

  int smallBlockSize = rows / numBlocks;//number of lines per block

  int largeBlockSize = smallBlockSize + 1;

  int remainder = rows % numBlocks; //number of large blocks

  int blockCount = 0;

  size_t sizeFinal=0;
  while (!file.eof()) {
    blockCount++;

    int blockSize = smallBlockSize;
    if (remainder > 0) {
        blockSize = largeBlockSize;
        remainder--;
    }

    pair<vector<double>,vector<VectorElement>> csrvRepresentation =  generateCSRV(file, numCol, blockSize);


    vector<VectorElement> csrvVector = csrvRepresentation.second;
    if (csrvVector.empty()) 
      return sizeFinal;

    csrvVector.shrink_to_fit();

    vector<double> V = csrvRepresentation.first;
    V.shrink_to_fit();

    string blockFilename = changeExtension(filename, to_string(blockCount) + ".");

    sizeFinal += writeAuxiliaryDataToBinaryFile(blockFilename, numCol, V);

    sizeFinal += compressor(blockFilename, csrvVector);
  }


  return sizeFinal;
}

void reconstructCSVFile(const string& originalCSVFilename, int numBlocks){
  ofstream csvOutFile;

  csvOutFile.open("output.csv");
  csvOutFile.close();

  csvOutFile.open("output.csv", ios::app);

  for (int i = 1; i <= numBlocks; i++) {
    string filename1 = changeExtension(originalCSVFilename, to_string(i) + ".re32");
    string filename2 = changeExtension(originalCSVFilename, to_string(i) + ".V");

    pair<vector<int>,vector<Pair>> auxPair = getSequenceAndGrammar(filename1);
    vector<int> sequence = auxPair.first;
    vector<Pair> grammar = auxPair.second;
    pair<int, vector<double>> auxiliaryData = readAuxiliaryData(filename2);
    int numCol = auxiliaryData.first;
    vector<double> uniqueValues = auxiliaryData.second;
    vector<int> csrvMatrixDecompressed = decompressor(sequence, grammar);
    generateCSVFromCSRV(csrvMatrixDecompressed, numCol, uniqueValues, csvOutFile);

  }

  csvOutFile.close();


}

int getNumberOfColumns(const string& csvFilename) {
  ifstream file(csvFilename);
  string line;

  if (!file.is_open()) {
    cerr << "Error: Could not open " << csvFilename << endl;
    exit(EXIT_FAILURE);
  }

  int numCol = 0;

  // Read the first line
  if (getline(file, line)) {
    istringstream ss(line);
    string value;

    // Split by commas
    while (getline(ss, value, ',')) {
      numCol++;
    }
  }
  else{
    cerr << "Error: Failed to read the first line of " << csvFilename << endl;
    exit(EXIT_FAILURE);
  }

  file.close();

  return numCol;

}

int encodePair(Pair pair, int numCol){
  return 1 + pair.first*numCol + pair.second;
}

pair<vector<double>,vector<VectorElement>> generateCSRV(ifstream& file, int numCol, int blockSize) {

  vector<VectorElement> csrvVector;
  string line;
  vector<double> uniqueValues;
  unordered_map<double, int> uniqueValueMap;

  int lineCount = 0;
  while (lineCount < blockSize && getline(file, line)) {

    if (line.empty()) continue;

    lineCount++;

    istringstream ss(line);
    string cell;

    int colIndex = 0;
    while (getline(ss, cell, ',')) {
      double value = stod(cell);
      if (value == 0.0){
        colIndex++;
        continue;
      } 
      auto itHashTable = uniqueValueMap.find(value);
      Pair pair;
      if (itHashTable == uniqueValueMap.end()){
        uniqueValueMap[value] = uniqueValues.size();
        pair = {uniqueValues.size(), colIndex};
        uniqueValues.push_back(value);
      }
      else 
        pair = {itHashTable->second, colIndex};
      int encodedPair = encodePair(pair, numCol);
      csrvVector.emplace_back(VectorElement{EMPTY, encodedPair, EMPTY});
      colIndex++;
    }
    csrvVector.emplace_back(VectorElement{EMPTY, END_ROW, EMPTY});
  }

  return make_pair(uniqueValues, csrvVector);
} 

size_t writeAuxiliaryDataToBinaryFile(const string& inputFilename, int numCol, vector<double>& uniqueValues){

  size_t sizeFinal=0;
  string outputFilename = changeExtension(inputFilename, ".V");

  ofstream outputFile(outputFilename, ios::binary);
  if (!outputFile.is_open()) {
    // Print an error message and exit if the file cannot be opened
    cerr << "Error: Could not open " << outputFilename << endl;
    exit(EXIT_FAILURE);
  }

  outputFile.write(reinterpret_cast<const char*>(&numCol), sizeof(numCol));
  sizeFinal+=sizeof(numCol);
  if (!outputFile.good()) {
    cerr << "Error writing numCol to " << outputFilename << endl;
    outputFile.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  outputFile.write(reinterpret_cast<const char*>(uniqueValues.data()), uniqueValues.size() * sizeof(double));
  sizeFinal+=uniqueValues.size() * sizeof(double);
  if (!outputFile.good()) {
    cerr << "Error writing outputVector to " << outputFilename << endl;
    outputFile.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  outputFile.close();

return sizeFinal;
}

pair<int, vector<double>> readAuxiliaryData(const string& filename){
  ifstream file(filename, ios::binary);
  if (!file.is_open()) {
    cerr << "Error opening file: " << filename << endl;
    exit(EXIT_FAILURE);
  }

  int numCol;
  file.read(reinterpret_cast<char*>(&numCol), sizeof(int));
  if (!file.good()) {
    cerr << "Error reading numCol from " << filename << endl;
    file.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  vector<double> uniqueValues;
  double value;
  while(file.read(reinterpret_cast<char*>(&value), sizeof(double)))
    uniqueValues.push_back(value);

  if (!file.eof()) {
    cerr << "Error reading uniqueValues from " << filename << endl;
    file.close();
    exit(EXIT_FAILURE);
  }

  file.close();

  return make_pair(numCol, uniqueValues);

}


void generateCSVFromCSRV(vector<int>& csrvMatrixDecompressed, int numCol, vector<double>& uniqueValues, ofstream& csvOutFile) {

  ostringstream buffer;

  int lastColumn = -1;
  for (size_t index = 0; index < csrvMatrixDecompressed.size(); index++){
    int element = csrvMatrixDecompressed[index];
    if (element != 0) {
      int i = (element - 1) / numCol ;
      int j = (element - 1) % numCol;
      int numZeros = j - lastColumn - 1;
      for (int k = 0; k < numZeros; k++) buffer << 0 << ',';
      buffer << uniqueValues[i];
      if (j < numCol - 1) buffer << ',';
      lastColumn = j;
    }
    else {
      int numZeros = numCol - lastColumn - 1;
      if (numZeros != 0) {
        for (int k = 0; k < numZeros - 1; k++) buffer << 0 << ',';
        buffer << 0;
      }
      buffer << '\n';
      lastColumn = -1;
    }
  }

  csvOutFile << buffer.str();

} 

void print_usage(const char* program_name) {
  cerr << "Usage: " << program_name << " -c filename blockSize | -d filename blockCount\n"
    << "Options:\n"
    << "  -c\tCompress a CSV file. Requires filename and blockSize.\n"
    << "  -d\tDecompress a CSV file. Requires filename and blockCount.\n";
}


int main(int argc, char *argv[]) {
  int opt;
  bool compress = false, decompress = false;
  string filename;
  int numBlocks = 0;
  int time = 1;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "c:d:t")) != -1) {
    switch (opt) {
      case 'c':
        compress = true;
        filename = optarg; // filename from -c argument
        // Check if block size is also provided as the next argument
        if (optind < argc && argv[optind][0] != '-') {
          numBlocks = atoi(argv[optind++]);
        } else {
          cerr << "Error: Missing block size for -c option.\n";
          print_usage(argv[0]);
          return 1;
        }
        break;
      case 'd':
        decompress = true;
        filename = optarg; // filename from -d argument
        // Check if block count is also provided as the next argument
        if (optind < argc && argv[optind][0] != '-') {
          numBlocks = atoi(argv[optind++]);
        } else {
          cerr << "Error: Missing block count for -d option.\n";
          print_usage(argv[0]);
          return 1;
        }
        break;
      case 't':
        time = 1;
      case '?':
        // Handle unknown options
        print_usage(argv[0]);
        return 1;
    }
  }


  // Validation and output
  if (compress && decompress) {
    cerr << "Error: Both -c and -d options cannot be used simultaneously.\n";
    print_usage(argv[0]);
    return 1;
  } else if (!compress && !decompress) {
    cerr << "Error: Either -c or -d option must be specified.\n";
    print_usage(argv[0]);
    return 1;
  }

  if (optind < argc) {
    cerr << "Error: Unexpected additional arguments.\n";  
    print_usage(argv[0]);
    return 1;
  }

  if(time)
    time_start(&t_total, &c_total);

  // Proceed with compression or decompression
  if (compress) {
    cout << compressCSVFile(filename, numBlocks)/pow(2,20) << " MB\n";
  } 
  else {
    reconstructCSVFile(filename, numBlocks);
  }
  if(time){
    printf("## TOTAL ##\n");
    fprintf(stderr,"%.6lf\n", time_stop(t_total, c_total));
  }

  return 0;
}
