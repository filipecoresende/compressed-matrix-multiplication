#include <unistd.h>
#include <iostream>

int main(int argc, char *argv[]) {
    int opt;
    // optstring: "a:b:c" where
    //   'a' expects an argument
    //   'b' expects an argument
    //   'c' does not expect an argument
    while ((opt = getopt(argc, argv, "a:b:c")) != -1) {
        switch (opt) {
            case 'a':
                std::cout << optind << std::endl;
                std::cout << "Option -a with value: " << optarg << std::endl;
                break;
            case 'b':
                std::cout << optind << std::endl;
                std::cout << "Option -b with value: " << optarg << std::endl;
                break;
            case 'c':
                std::cout << optind << std::endl;
                std::cout << "Option -c" << std::endl;
                break;
            case '?':  // Invalid option
                std::cerr << "Unknown option: " << char(optopt) << std::endl;
                break;
        }
    }

    // Remaining arguments (non-options) after options are parsed
    for (int index = optind; index < argc; index++) {
        std::cout << "Non-option argument: " << argv[index] << std::endl;
    }

    return 0;
}
