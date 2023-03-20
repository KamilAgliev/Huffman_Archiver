#include "Archiver.h"

void compressor::CompressFile(std::string_view filepath, bool is_last_file, Stream &writer) {
    Stream reader(filepath, 'r');

    size_t slash_index = filepath.rfind('/');
    std::string_view filename = filepath.substr(slash_index == std::string_view::npos ? 0 : slash_index + 1);

    std::unordered_map<size_t, size_t> counts;
    for (unsigned char c : filename) {
        ++counts[c];
    }

    while (!reader.Eof()) {
        unsigned char current_char = reader.ReadChar();
        ++counts[current_char];
    }

    counts[FILENAME_END] = 1;
    counts[ONE_MORE_FILE] = 1;
    counts[ARCHIVE_END] = 1;

    reader.ResetStream();

    HaffmanTree tree(counts);
    std::unordered_map<size_t, std::vector<bool>> kanonic_codes = tree.GetKanonicCodes();
    std::vector<std::pair<size_t, size_t>> kanonic_order = tree.GetHaffmanCodes();

    writer.WriteNumber(kanonic_order.size(), BYTE_SIZE);

    std::vector<size_t> symbol_code_sizes = {0};
    for (auto &[char_num, length] : kanonic_order) {
        writer.WriteNumber(char_num, BYTE_SIZE);
        while (length != symbol_code_sizes.size()) {
            symbol_code_sizes.push_back(0);
        }
        ++symbol_code_sizes.back();
    }

    for (auto &cnt : symbol_code_sizes) {
        writer.WriteNumber(cnt, BYTE_SIZE);
    }

    for (unsigned char c : filename) {
        auto code_iter = kanonic_codes.find(c);
        if (code_iter == kanonic_codes.end()) {
            throw std::runtime_error(std::string("Kanonic code for symbol") + reinterpret_cast<const char *>(c) +
                                     std::string("not found!"));
        }
        writer.Write(code_iter->second);
    }
    if (!kanonic_codes.contains(FILENAME_END)) {
        throw std::runtime_error("Kanonic code for symbol FILENAME_END not found");
    }
    writer.Write(kanonic_codes[FILENAME_END]);

    while (!reader.Eof()) {
        unsigned char current_char = reader.ReadChar();
        auto code_iter = kanonic_codes.find(current_char);
        if (code_iter == kanonic_codes.end()) {
            throw std::runtime_error(std::string("Kanonic code for symbol") +
                                     reinterpret_cast<const char *>(current_char) + std::string("not found!"));
        }
        writer.Write(code_iter->second);
    }

    if (is_last_file) {
        auto code_iter = kanonic_codes.find(ARCHIVE_END);
        if (code_iter == kanonic_codes.end()) {
            throw std::runtime_error("Kanonic code for symbol ARCHIVE_END not found");
        }
        writer.Write(code_iter->second);
    } else {
        auto code_iter = kanonic_codes.find(ONE_MORE_FILE);
        if (code_iter == kanonic_codes.end()) {
            throw std::runtime_error("Kanonic code for symbol ONE_MORE_FILE not found");
        }
        writer.Write(code_iter->second);
    }
}

void compressor::Compress(const std::vector<std::string_view> &filenames, std::string_view archive_name) {
    Stream writer(archive_name, 'w');

    for (size_t i = 0; i < filenames.size(); ++i) {
        bool is_last_file = (i + 1) == filenames.size();
        CompressFile(filenames[i], is_last_file, writer);
    }
}