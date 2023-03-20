#pragma once
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

class Stream {
private:
    std::fstream stream_;
    char type_;
    bool little_end_;
    bool eof_;
    std::unique_ptr<char[]> buffer_;
    size_t bits_rem_;
    size_t bytes_cnt_;
    size_t cur_byte_;
    const size_t byte_size_ = 8;
    const size_t buffer_size_ = 1024;

public:
    explicit Stream(std::string_view filename, char type, bool is_little_end = false);

    ~Stream();

    void ReadBits(size_t bits_count, std::vector<bool>& result);

    unsigned char ReadChar();

    void ReadBuffer();

    bool Eof();

    void ResetStream();

    void Write(const std::vector<bool>& bits);

    void WriteByte(const char& data);

    void WriteBuffer();

    void WriteNumber(size_t data, size_t bits);
};