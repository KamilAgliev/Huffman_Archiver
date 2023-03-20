#include "Archiver.h"

size_t decompressor::ToNum(const std::vector<bool> &bin, bool is_little) {
    size_t res = 0;
    for (size_t i = 0; i < bin.size(); ++i) {
        bool bit = is_little ? bin[i] : bin[bin.size() - 1 - i];
        if (bit) {
            res += (1 << i);
        }
    }
    return res;
}

void decompressor::Decompress(std::string_view archive_name) {
    Stream reader(archive_name, 'r', true);

    auto wrong_format_error =
        std::runtime_error("File " + std::string(archive_name) + " has wrong decompressed file format!");
    bool archive_eof = false;
    std::vector<bool> temp_buffer;

    while (!archive_eof) {
        temp_buffer = std::vector<bool>(BYTE_SIZE, false);

        if (reader.Eof()) {
            throw wrong_format_error;
        }
        reader.ReadBits(BYTE_SIZE, temp_buffer);
        size_t symbols_count = ToNum(temp_buffer);

        std::vector<std::pair<size_t, size_t>> symbols;
        for (size_t i = 0; i < symbols_count; ++i) {
            if (reader.Eof()) {
                throw wrong_format_error;
            }
            reader.ReadBits(BYTE_SIZE, temp_buffer);
            symbols.push_back({ToNum(temp_buffer), 0});
        }

        size_t current_symbol = 0;
        for (size_t current_size = 1; current_symbol < symbols_count; ++current_size) {
            if (reader.Eof()) {
                throw wrong_format_error;
            }
            reader.ReadBits(BYTE_SIZE, temp_buffer);
            size_t current_size_count = ToNum(temp_buffer);
            for (size_t j = 0; j < current_size_count; ++j) {
                if (current_symbol >= symbols.size()) {
                    throw wrong_format_error;
                }
                symbols[current_symbol].second = current_size;
                ++current_symbol;
            }
        }

        std::shared_ptr<HaffmanTree::TrieNode> trie_root = HaffmanTree::RestoreKanonicCodes(symbols);
        std::shared_ptr<HaffmanTree::TrieNode> current_node = trie_root;

        std::string filename;
        temp_buffer = {false};
        while (true) {
            while (!current_node->code.has_value()) {
                if (reader.Eof()) {
                    throw wrong_format_error;
                }
                reader.ReadBits(1, temp_buffer);
                if (!temp_buffer[0]) {
                    if (!current_node->left) {
                        throw wrong_format_error;
                    }
                    current_node = current_node->left;
                } else {
                    if (!current_node->right) {
                        throw wrong_format_error;
                    }
                    current_node = current_node->right;
                }
            }
            if (current_node->code.value() != FILENAME_END) {
                filename += static_cast<char>(current_node->code.value());
                current_node = trie_root;
            } else {
                break;
            }
        }

        Stream writer(filename, 'w');
        current_node = trie_root;
        while (true) {
            while (!current_node->code.has_value()) {
                if (reader.Eof()) {
                    throw wrong_format_error;
                }
                reader.ReadBits(1, temp_buffer);
                if (!temp_buffer[0]) {
                    if (!current_node->left) {
                        throw wrong_format_error;
                    }
                    current_node = current_node->left;
                } else {
                    if (!current_node->right) {
                        throw wrong_format_error;
                    }
                    current_node = current_node->right;
                }
            }
            if (current_node->code.value() != ONE_MORE_FILE && current_node->code.value() != ARCHIVE_END) {
                writer.WriteByte(current_node->code.value());
                current_node = trie_root;
            } else {
                break;
            }
        }

        if (current_node->code.value() == ARCHIVE_END) {
            archive_eof = true;
        }
        current_node.reset();

        std::vector<std::shared_ptr<HaffmanTree::TrieNode>> trie_nodes;
        trie_nodes.push_back(trie_root);
        trie_root.reset();
        while (!trie_nodes.empty()) {
            std::shared_ptr<HaffmanTree::TrieNode> cur_node = trie_nodes.back();
            trie_nodes.back().reset();
            trie_nodes.pop_back();
            if (cur_node->left) {
                trie_nodes.push_back(cur_node->left);
                cur_node->left.reset();
            }
            if (cur_node->right) {
                trie_nodes.push_back(cur_node->right);
                cur_node->right.reset();
            }
            cur_node.reset();
        }
    }
}
