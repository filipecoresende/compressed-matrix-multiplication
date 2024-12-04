#include "matrix_compressor.hpp"

vector<double> readVector(const string &vectorCSVFilename);
double evalTerminal(int terminal, int numCol, vector<double> &x, vector<double> &V);
void fillEvalVector(vector<double> &W, vector<double> &V, vector<Pair> &grammar, vector<double> &x, int numCol);
void computeProduct(ofstream &outFile, vector<int> &C, vector<double> &W, vector<double> &x, vector<double> &V, int numCol);
void rightMatVecMult(const string &originalCSVFilename, const string &vectorCSVFilename, int numBlocks);