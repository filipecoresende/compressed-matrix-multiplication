#ifndef MATRIX_COMPRESSOR_H
#define MATRIX_COMPRESSOR_H

#include <fstream>
#include "repair.hpp"

#define END_ROW 0

size_t compressCSVFile(const string &filename, int numBlocks);
void reconstructCSVFile(const string &originalCSVFilename, int numBlocks);
int encodePair(Pair pair, int numCol);
pair<vector<double>, vector<VectorElement>> generateCSRV(ifstream &file, int numCol, int numRows);
void generateCSVFromCSRV(vector<int> &csrvMatrixDecompressed, int numCol, vector<double> &uniqueValues, ofstream &csvOutFile);
size_t writeAuxiliaryDataToBinaryFile(const string &inputFilename, int numCol, vector<double> &uniqueValues);
pair<int, vector<double>> readAuxiliaryData(const string &filename);

#endif