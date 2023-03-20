#include "catch.hpp"
#include "Stream.h"
#include "PriorityQueue.h"
#include "Archiver.h"

TEST_CASE("PositiveReadingWriting") {
    {
        Stream writer("test_file.txt", 'w');
        writer.WriteByte('a');
        writer.WriteByte('b');
    }

    Stream reader("test_file.txt", 'r');

    auto first_part = std::vector<bool>(9);
    reader.ReadBits(9, first_part);
    std::vector<bool> first_expected = {false, true, true, false, false, false, false, true, false};
    auto second_part = std::vector<bool>(16);
    reader.ReadBits(16, second_part);
    std::vector<bool> second_expected = {true,  true,  false, false, false, true,  false, false,
                                         false, false, false, false, false, false, false, false};
    REQUIRE(first_part == first_expected);
    REQUIRE(second_part == second_expected);
    REQUIRE(reader.Eof());

    reader.ResetStream();

    REQUIRE(!reader.Eof());
    auto resetted_first_part = std::vector<bool>(9);
    reader.ReadBits(9, resetted_first_part);
    std::vector<bool> resetted_first_expected = first_expected;
    auto resetted_second_part = std::vector<bool>(16);
    reader.ReadBits(16, resetted_second_part);
    std::vector<bool> resetted_second_expected = second_expected;
    REQUIRE(resetted_first_part == resetted_first_expected);
    REQUIRE(resetted_second_part == resetted_second_expected);
    REQUIRE(reader.Eof());

    std::remove("test_file.txt");
}

TEST_CASE("FileNotOpening") {
    bool catched = false;
    try {
        Stream writer("////asd//", 'w');
        Stream reader("/asd/", 'r');
        Stream writer2("adfaf.txt", 'w');
        Stream reader2("sdfxcv..,!.txt", 'r');
    } catch (std::runtime_error &e) {
        std::string error = e.what();
        if (error.starts_with("Invalid file name")) {
            catched = true;
        } else if (error.starts_with("Can't open file")) {
            catched = true;
        }
    }
    REQUIRE(catched);
}

TEST_CASE("QueueTest") {
    struct Node {
        size_t char_num;
        size_t count;
        std::shared_ptr<Node> right = nullptr;
        std::shared_ptr<Node> left = nullptr;

        Node(size_t char_num, size_t count) {
            this->count = count;
            this->char_num = char_num;
        }
    };

    struct NodePtrCmp {
        bool operator()(const Node &a, const Node b) const {
            if (a.count != b.count) {
                return a.count < b.count;
            }
            return a.char_num < b.char_num;
        }
    };

    PriorityQueue<Node, NodePtrCmp> queue;
    queue.Push(Node(2, 1));
    queue.Push(Node(1, 1));
    queue.Push(Node(1, 2));

    Node cur = queue.Top();
    queue.Pop();
    REQUIRE(cur.char_num == 1);
    REQUIRE(cur.count == 1);

    cur = queue.Top();
    queue.Pop();
    REQUIRE(cur.char_num == 2);
    REQUIRE(cur.count == 1);

    cur = queue.Top();
    queue.Pop();
    REQUIRE(cur.char_num == 1);
    REQUIRE(cur.count == 2);

    size_t errors = 0;
    try {
        cur = queue.Top();
        ++cur.count;
    } catch (...) {
        ++errors;
    }
    try {
        queue.Pop();
    } catch (...) {
        ++errors;
    }
    REQUIRE(errors == 2);
}

TEST_CASE("KanonicTest") {
    std::unordered_map<size_t, size_t> counts;
    counts[0] = 15;
    counts[1] = 7;
    counts[2] = 6;
    counts[3] = 6;
    counts[4] = 5;

    std::unordered_map<size_t, std::vector<bool>> expected_codes;
    expected_codes[0] = {false};
    expected_codes[1] = {true, false, false};
    expected_codes[2] = {true, false, true};
    expected_codes[3] = {true, true, false};
    expected_codes[4] = {true, true, true};

    std::vector<std::pair<size_t, size_t>> expected_order = {{0, 1}, {1, 3}, {2, 3}, {3, 3}, {4, 3}};

    HaffmanTree tree(counts);
    std::unordered_map<size_t, std::vector<bool>> kanonic_codes = tree.GetKanonicCodes();
    std::vector<std::pair<size_t, size_t>> kanonic_order = tree.GetHaffmanCodes();
    REQUIRE(kanonic_codes == expected_codes);
    REQUIRE(kanonic_order == expected_order);
}

TEST_CASE("BadDecompressedTest") {
    {
        Stream writer("bad_decompressed.arc", 'w');
        for (size_t i = 0; i < 1000; i++) {
            unsigned char cur = 'a' + i;
            writer.WriteByte(static_cast<char>(cur));
            std::vector<bool> temp(9);
            temp[i % 9] = true;
            writer.Write(temp);
        }
    }
    bool error = false;
    try {
        decompressor::Decompress("bad_decompressed.arc");
    } catch (...) {
        error = true;
    }
    std::remove("bad_decompressed.arc");
    REQUIRE(error);
}

TEST_CASE("CompressTest") {
    {
        Stream writer("test_file.txt", 'w');
        writer.WriteByte('a');
        writer.WriteByte('b');
        writer.WriteByte('a');
        writer.WriteByte('c');
        writer.WriteByte('c');
    }
    compressor::Compress({"test_file.txt"}, "test_archive.arc");
    {
        std::vector<unsigned char> expected = {7,   157, 12,  38,  51,  41,  164, 216, 115, 60, 64,  32,
                                               48,  33,  113, 124, 196, 102, 0,   0,   64,  0,  160, 32,
                                               210, 119, 239, 13,  194, 139, 79,  34,  174, 128};
        Stream reader("test_archive.arc", 'r');
        std::vector<unsigned char> cur;
        while (!reader.Eof()) {
            cur.push_back(reader.ReadChar());
        }
        REQUIRE(expected == cur);
    }
    std::remove("test_file.txt");
    std::remove("test_archive.arc");
}

TEST_CASE("DecompressTest") {
    {
        Stream writer("test_file.txt", 'w');
        writer.WriteByte('a');
        writer.WriteByte('b');
        writer.WriteByte('a');
        writer.WriteByte('c');
        writer.WriteByte('c');
    }
    compressor::Compress({"test_file.txt"}, "test_archive.arc");
    decompressor::Decompress("test_archive.arc");
    {
        Stream reader("test_file.txt", 'r');
        std::vector<unsigned char> expected = {'a', 'b', 'a', 'c', 'c'};
        std::vector<unsigned char> cur;
        while (!reader.Eof()) {
            cur.push_back(reader.ReadChar());
        }
        REQUIRE(expected == cur);
    }
}
