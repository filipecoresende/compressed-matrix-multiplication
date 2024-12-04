#include <iostream>
#include <sstream>
#include <cstdlib>

#include "matrix_compressor.hpp"

size_t compressCSVFile(const string &filename, int numBlocks)
{

  ifstream file(filename);
  if (!file.is_open())
  {
    cerr << "Error: Could not open " << filename << endl;
    exit(EXIT_FAILURE);
  }

  size_t numRows = 0;
  size_t numCol = 0;
  string row;

  if (getline(file, row))
  {
    numRows++;

    istringstream ss(row);
    string value;

    // Split by commas
    while (getline(ss, value, ','))
      numCol++;
  }

  while (getline(file, row))
    numRows++;
  file.clear();
  file.seekg(0, ios_base::beg);

  int smallBlockSize = numRows / numBlocks; // number of lines per block

  int largeBlockSize = smallBlockSize + 1;

  int remainder = numRows % numBlocks; // number of large blocks

  int blockCount = 0;

  size_t sizeFinal = 0;
  while (!file.eof())
  {
    blockCount++;

    int numRows = smallBlockSize;
    if (remainder > 0)
    {
      numRows = largeBlockSize;
      remainder--;
    }

    pair<vector<double>, vector<VectorElement>> csrvRepresentation = generateCSRV(file, numCol, numRows);

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

  file.close();

  return sizeFinal;
}

void reconstructCSVFile(const string &originalCSVFilename, int numBlocks)
{
  ofstream csvOutFile;

  csvOutFile.open("decomp_output.csv");
  csvOutFile.close();

  csvOutFile.open("decomp_output.csv", ios::app);

  for (int i = 1; i <= numBlocks; i++)
  {
    string filename1 = changeExtension(originalCSVFilename, to_string(i) + ".re32");
    string filename2 = changeExtension(originalCSVFilename, to_string(i) + ".V");

    pair<vector<int>, vector<Pair>> auxPair = getSequenceAndGrammar(filename1);
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

int encodePair(Pair pair, int numCol)
{
  return 1 + pair.first * numCol + pair.second;
}

pair<vector<double>, vector<VectorElement>> generateCSRV(ifstream &file, int numCol, int numRows)
{

  vector<VectorElement> csrvVector;
  string line;
  vector<double> uniqueValues;
  unordered_map<double, int> uniqueValueMap;

  int lineCount = 0;
  while (lineCount < numRows && getline(file, line))
  {

    if (line.empty())
      continue;

    lineCount++;

    stringstream ss(line);
    string cell;

    int colIndex = 0;
    while (getline(ss, cell, ','))
    {
      double value = stod(cell);
      if (value == 0.0)
      {
        colIndex++;
        continue;
      }
      auto itHashTable = uniqueValueMap.find(value);
      Pair pair;
      if (itHashTable == uniqueValueMap.end())
      {
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

size_t writeAuxiliaryDataToBinaryFile(const string &inputFilename, int numCol, vector<double> &uniqueValues)
{

  size_t sizeFinal = 0;
  string outputFilename = changeExtension(inputFilename, ".V");

  ofstream outputFile(outputFilename, ios::binary);
  if (!outputFile.is_open())
  {
    // Print an error message and exit if the file cannot be opened
    cerr << "Error: Could not open " << outputFilename << endl;
    exit(EXIT_FAILURE);
  }

  outputFile.write(reinterpret_cast<const char *>(&numCol), sizeof(numCol));
  sizeFinal += sizeof(numCol);
  if (!outputFile.good())
  {
    cerr << "Error writing numCol to " << outputFilename << endl;
    outputFile.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  outputFile.write(reinterpret_cast<const char *>(uniqueValues.data()), uniqueValues.size() * sizeof(double));
  sizeFinal += uniqueValues.size() * sizeof(double);
  if (!outputFile.good())
  {
    cerr << "Error writing outputVector to " << outputFilename << endl;
    outputFile.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  outputFile.close();

  return sizeFinal;
}

pair<int, vector<double>> readAuxiliaryData(const string &filename)
{
  ifstream file(filename, ios::binary);
  if (!file.is_open())
  {
    cerr << "Error opening file: " << filename << endl;
    exit(EXIT_FAILURE);
  }

  int numCol;
  file.read(reinterpret_cast<char *>(&numCol), sizeof(int));
  if (!file.good())
  {
    cerr << "Error reading numCol from " << filename << endl;
    file.close(); // Ensure the file is closed on error
    exit(EXIT_FAILURE);
  }

  vector<double> uniqueValues;
  double value;
  while (file.read(reinterpret_cast<char *>(&value), sizeof(double)))
    uniqueValues.push_back(value);

  if (!file.eof())
  {
    cerr << "Error reading uniqueValues from " << filename << endl;
    file.close();
    exit(EXIT_FAILURE);
  }

  file.close();

  return make_pair(numCol, uniqueValues);
}

void generateCSVFromCSRV(vector<int> &csrvMatrixDecompressed, int numCol, vector<double> &uniqueValues, ofstream &csvOutFile)
{

  ostringstream buffer;

  int lastColumn = -1;
  for (size_t index = 0; index < csrvMatrixDecompressed.size(); index++)
  {
    int element = csrvMatrixDecompressed[index];
    if (element != 0)
    {
      int i = (element - 1) / numCol;
      int j = (element - 1) % numCol;
      int numZeros = j - lastColumn - 1;
      for (int k = 0; k < numZeros; k++)
        buffer << 0 << ',';
      buffer << uniqueValues[i];
      if (j < numCol - 1)
        buffer << ',';
      lastColumn = j;
    }
    else
    {
      int numZeros = numCol - lastColumn - 1;
      if (numZeros != 0)
      {
        for (int k = 0; k < numZeros - 1; k++)
          buffer << 0 << ',';
        buffer << 0;
      }
      buffer << '\n';
      lastColumn = -1;
    }
  }

  csvOutFile << buffer.str();
}
