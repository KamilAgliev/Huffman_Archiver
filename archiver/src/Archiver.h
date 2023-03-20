#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include "HaffmanTree.h"
#include "Stream.h"

const int ERROR_CODE = 111;
const int BYTE_SIZE = 9;

const size_t FILENAME_END = 256;
const size_t ONE_MORE_FILE = 257;
const size_t ARCHIVE_END = 258;

const std::string_view HELP_COMMAND_STR =
    "Programm works with following commands:\n"
    "archiver -c archive_name file1 [file2 ...] - archive files file1, file2, ... and save result "
    "in file archive_name\n"
    "archiver -d archive_name - unarchive files from archive archive_name and put them in current "
    "directory\n"
    "archiver -h - provides information how to work with programm\n";

const std::string_view INVALID_INPUT_STR = "Invalid command line input! Run -h command to see commands.\n";

namespace compressor {

void CompressFile(std::string_view filepath, bool is_last_file, Stream &writer);

void Compress(const std::vector<std::string_view> &filenames, std::string_view archive_name);

}  // namespace compressor

namespace decompressor {

size_t ToNum(const std::vector<bool> &bin, bool is_little = true);

void Decompress(std::string_view archive_name);

}  // namespace decompressor
