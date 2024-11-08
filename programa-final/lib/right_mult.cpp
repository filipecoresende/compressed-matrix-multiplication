#include <iostream>
#include <sstream>
#include <cstdlib>

#include "right_mult.hpp"

vector<double> readVector(const string &vectorCSVFilename)
{

    ifstream inFile(vectorCSVFilename);

    if (!inFile.is_open())
    {
        cerr << "Error opening " << vectorCSVFilename << endl;
        exit(EXIT_FAILURE);
    }

    vector<double> x;
    string line;
    while (getline(inFile, line))
    {
        x.push_back(stod(line));
    }

    inFile.close();

    return x;
}

double evalTerminal(int terminal, int numCol, vector<double> &x, vector<double> &V)
{

    int l = (terminal - 1) / numCol;
    int j = (terminal - 1) % numCol;

    return V[l] * x[j];
}

void fillEvalVector(vector<double> &W, vector<double> &V, vector<Pair> &grammar, vector<double> &x, int numCol)
{

    for (int i = 0; i < W.size(); i++)
    {
        double result = 0;
        Pair rule = grammar[i];
        if (rule.first < 0)
            result += W[-rule.first - 1];
        else
            result += evalTerminal(rule.first, numCol, x, V);

        if (rule.second < 0)
            result += W[-rule.second - 1];
        else
            result += evalTerminal(rule.second, numCol, x, V);

        W[i] = result;
    }
}

void computeProduct(ofstream &outFile, vector<int> &C, vector<double> &W, vector<double> &x, vector<double> &V, int numCol)
{
    double result = 0;
    ostringstream oss;
    for (int i = 0; i < C.size(); i++)
    {

        if (C[i] == 0)
        {
            oss << result << '\n';
            result = 0;
        }
        else if (C[i] < 0)
            result += W[-C[i] - 1];
        else
            result += evalTerminal(C[i], numCol, x, V);
    }

    outFile << oss.str();
}

void rightMatVecMult(const string &originalCSVFilename, const string &vectorCSVFilename, int numBlocks)
{

    vector<double> x = readVector(vectorCSVFilename);

    ofstream outFile("mult_output.csv");
    if (!outFile.is_open())
    {
        cerr << "Error opening mult_output.csv\n";
        exit(EXIT_FAILURE);
    }
    outFile.close();

    outFile.open("mult_output.csv", ios::app);
    if (!outFile.is_open())
    {
        cerr << "Error opening mult_output.csv\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= numBlocks; i++)
    {

        string filename1 = changeExtension(originalCSVFilename, to_string(i) + ".re32");
        string filename2 = changeExtension(originalCSVFilename, to_string(i) + ".V");

        pair<vector<int>, vector<Pair>> auxPair = getSequenceAndGrammar(filename1);

        vector<int> C = get<0>(auxPair);

        vector<Pair> grammar = get<1>(auxPair);

        pair<int, vector<double>> auxiliaryData = readAuxiliaryData(filename2);

        int numCol = get<0>(auxiliaryData);

        vector<double> V = get<1>(auxiliaryData);

        vector<double> W(grammar.size());

        fillEvalVector(W, V, grammar, x, numCol);

        computeProduct(outFile, C, W, x, V, numCol);
    }

    outFile.close();
}

