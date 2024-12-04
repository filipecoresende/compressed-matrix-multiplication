#include <iostream>
#include <cstdlib>
#include <getopt.h>

#include "lib/right_mult.hpp"
#include "lib/utils.hpp"

time_t t_total = 0;
clock_t c_total = 0;

void printUsage(const string &program_name)
{
    cerr << "Usage:\n"
         << " To compress a CSV file:\n"
         << "   " << program_name << " -c <matrix_filename> <number_of_blocks>\n"
         << " To decompress the compressed format of the CSV file:\n"
         << "   " << program_name << " -d <original_matrix_filename> <number_of_blocks>\n"
         << " To multiply the compressed format of the CSV file by a vector:\n"
         << "   " << program_name << " -m <original_matrix_filename> <vector_filename> <number_of_blocks>\n"
         << " To measure the time taken by the operations (use with any of the above options):\n"
         << "   " << program_name << " -t\n";
}

int main(int argc, char *argv[])
{

    int opt;
    bool time = false, compress = false, decompress = false;
    bool optionChosen = false;
    string matrixFilename;
    string vectorFilename;

    int numBlocks = 0;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "c:d:m:t")) != -1)
    {
        if (optionChosen && opt != 't')
        {
            cerr << "Error: Only one option can be chosen at a time." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        switch (opt)
        {
        case 'c':
            optionChosen = true;
            compress = true;
            matrixFilename = optarg; // filename from -c argument
            // Check if block size is also provided as the next argument
            if (optind < argc && argv[optind][0] != '-')
            {
                numBlocks = atoi(argv[optind++]);
            }
            else
            {
                cerr << "Error: Missing block size for -c option.\n";
                printUsage(argv[0]);
                return 1;
            }
            break;
        case 'd':
            optionChosen = true;
            decompress = true;
            matrixFilename = optarg; // matrixFilename from -d argument
            // Check if block count is also provided as the next argument
            if (optind < argc && argv[optind][0] != '-')
            {
                numBlocks = atoi(argv[optind++]);
            }
            else
            {
                cerr << "Error: Missing block count for -d option.\n";
                printUsage(argv[0]);
                return 1;
            }
            break;
        case 'm':
            optionChosen = true;
            matrixFilename = optarg;

            if (optind < argc && argv[optind][0] != '-')
            {
                vectorFilename = argv[optind++];
            }
            else
            {
                cerr << "Error: Missing vector filename for -m option.\n";
                printUsage(argv[0]);
                return 1;
            }

            if (optind < argc && argv[optind][0] != '-')
            {
                numBlocks = atoi(argv[optind++]);
            }
            else
            {
                cerr << "Error: Missing block count for -m option.\n";
                printUsage(argv[0]);
                return 1;
            }
            break;
        case 't':
            time = true;
            break;
        case '?':
            printUsage(argv[0]);
            return 1;
        }
    }

    if (!optionChosen)
    {
        cerr << "Error: Either -c, -d, or -m option must be specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    if (optind < argc)
    {
        cerr << "Error: Unexpected additional arguments.\n";
        printUsage(argv[0]);
        return 1;
    }

    if (time)
        time_start(&t_total, &c_total);

    // Proceed with compression or decompression
    if (compress)
    {
        cout << compressCSVFile(matrixFilename, numBlocks) << " bytes\n";
    }
    else if (decompress)
    {
        reconstructCSVFile(matrixFilename, numBlocks);
    }

    else
    {
        rightMatVecMult(matrixFilename, vectorFilename, numBlocks);
    }
    


    if (time)
    {
        printf("## TOTAL ##\n");
        fprintf(stderr, "%.6lf\n", time_stop(t_total, c_total));
    }

    return 0;
}