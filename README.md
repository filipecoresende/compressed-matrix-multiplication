# Matrix Compression and Multiplication

This project provides an implementation for compressing (and decompressing) matrices using the method proposed by [Ferragina et al., 2022](#references), which enables efficient operations on compressed matrices. This implementation also includes functionality for performing matrix-vector multiplication on the compressed matrices.


## Features
- **Compress CSV File**: Compress a matrix (in the form of CSV file) into a compressed format.
- **Decompress CSV File**: Reconstruct the original matrix (CSV file) from the compressed format.
- **Matrix-Vector Multiplication**: Perform multiplication of a compressed matrix by a vector on the right.
- **Time Measurement**: Measure the time taken by the operations for performance benchmarking.

## Requirements

- **C++ Compiler**: A C++ compiler (e.g., g++) that supports C++20 or higher.
- **Make**: To build the program using `make`.

## Building the Program

To compile and build the executable, you will need to use `make`. The Makefile will handle the compilation and linking process. Simply run the following command in the terminal:

```bash
make
```


## Usage

After building the program, you can use it with the following commands:

### Compress a matrix

To compress a matrix in CSV format, use:

```bash
   ./program -c <csv_filename> <number_of_blocks>
```
### Decompress a matrix

To decompress a previously compressed matrix, use:

```bash
./program -d <original_csv_filename> <number_of_blocks>
```

### Multiply Compressed Matrix by a Vector

To perform matrix-vector multiplication with a compressed matrix, use:

```bash
./program -m <original_csv_filename> <vector_filename> <number_of_blocks>
```

### Time Measurement

To measure the time taken by any of the above operations (compression, decompression, or multiplication), use:

```bash
./program -t <operation_command>
```
Where `<operation_command>` can be any of the commands for compression (-c), decompression (-d), or matrix-vector multiplication (-m).

## Example

Consider the CSV file covtype.csv provided in the dataset directory.

To compress it into 4 blocks:

```bash
./program -c dataset/covtype.csv 4
```

To decompress the compressed matrix into `decomp_output.csv` and output the time taken by this operation:

```bash
./your_program_name -t -d dataset/covtype.csv 4
```

To multiply the compressed matrix by the vector `example_vector.csv` in the dataset directory:

```bash
./your_program_name -m dataset/covtype.csv dataset/example_vector.csv 4
```

## Notes

Make sure that the number of blocks specified for decompression and multiplication matches the original compression block count.


## References

1. Ferragina, P., Gagie, T., Köppl, D., Manzini, G., Navarro, G., Striani, M., & Tosoni, F. (2022). *Improving Matrix-vector Multiplication via Lossless Grammar-Compressed Matrices*. arXiv. https://arxiv.org/abs/2203.14540.
2. Larsson, N. J., & Moffat, A. (2000). Off-line dictionary-based compression. *Proceedings of the IEEE*, 88(11), 1722-1732. https://doi.org/10.1109/5.892708.