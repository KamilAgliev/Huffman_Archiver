#include "Archiver.h"

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "-h") {
        std::cout << HELP_COMMAND_STR << "\n";
    } else if (argc == 3 && std::string(argv[1]) == "-d") {
        try {
            decompressor::Decompress(argv[2]);
            std::cout << "Files unarchived from " << argv[2] << "\n";
        } catch (const std::runtime_error &e) {
            std::cout << e.what() << "\n";
            return ERROR_CODE;
        }
    } else if (argc >= 4 && std::string(argv[1]) == "-c") {
        try {
            std::vector<std::string_view> file_names;
            for (int i = 3; i < argc; ++i) {
                file_names.push_back(argv[i]);
            }

            compressor::Compress(file_names, argv[2]);

            std::cout << "Files ";
            for (int i = 3; i < argc; ++i) {
                std::cout << argv[i] << " ";
            }
            std::cout << "archived to " << argv[2] << "\n";
        } catch (const std::runtime_error &e) {
            std::cout << e.what() << "\n";
            return ERROR_CODE;
        }
    } else {
        std::cout << INVALID_INPUT_STR << "\n";
        return ERROR_CODE;
    }
    return 0;
}
