#include "Stream.h"

Stream::Stream(std::string_view filename, char type, bool is_little_end)
    : type_(type), little_end_(is_little_end), eof_(false), bits_rem_(0), bytes_cnt_(0), cur_byte_(0) {
    std::string filaname_str(filename);
    if (type_ == 'w') {
        try {
            std::ofstream file(filaname_str);
            file.close();
        } catch (...) {
            throw std::runtime_error("Invalid file name " + filaname_str + " for input/output !");
        }
    }
    stream_.open(filaname_str);
    if (!stream_.is_open() || stream_.bad()) {
        throw std::runtime_error("Can't open file " + filaname_str);
    }
    buffer_ = std::unique_ptr<char[]>(new char[buffer_size_]);
    if (type_ == 'w') {
        for (size_t j = 0; j < buffer_size_; ++j) {
            buffer_[j] = 0;
        }
    }
}

void Stream::ReadBuffer() {
    stream_.read(buffer_.get(), buffer_size_);
    bytes_cnt_ = stream_.gcount();
    if (stream_.eof()) {
        eof_ = true;
    }
    cur_byte_ = 0;
}

bool Stream::Eof() {
    if (eof_) {
        return cur_byte_ == bytes_cnt_;
    }
    if (cur_byte_ < bytes_cnt_) {
        return false;
    }
    ReadBuffer();
    bits_rem_ = byte_size_;
    return bytes_cnt_ == 0;
}

void Stream::ReadBits(size_t bits_count, std::vector<bool>& result) {
    for (size_t i = 0; i < bits_count; ++i) {
        if (bytes_cnt_ == cur_byte_ && !eof_) {
            ReadBuffer();
            bits_rem_ = byte_size_;
        }
        if (bits_rem_ == 0) {
            bits_rem_ = byte_size_;
            ++cur_byte_;
        }
        if (bytes_cnt_ == cur_byte_) {
            result[i] = false;
        } else {
            --bits_rem_;
            result[i] = ((buffer_[cur_byte_] >> bits_rem_) & 1);
            if (bits_rem_ == 0) {
                ++cur_byte_;
                bits_rem_ = byte_size_;
            }
        }
    }
    if (little_end_) {
        std::reverse(result.begin(), result.end());
    }
}

unsigned char Stream::ReadChar() {
    if (bytes_cnt_ == cur_byte_ && !eof_) {
        ReadBuffer();
    }
    if (bytes_cnt_ == cur_byte_) {
        return 0;
    }
    return buffer_[cur_byte_++];
}

void Stream::ResetStream() {
    stream_.clear();
    stream_.seekg(0);
    eof_ = false;
    bytes_cnt_ = 0;
    cur_byte_ = 0;
    bits_rem_ = 0;
}

Stream::~Stream() {
    if (type_ == 'w') {
        if (cur_byte_ > 0 || bits_rem_ > 0) {
            if (bits_rem_ > 0) {
                ++cur_byte_;
            }
            WriteBuffer();
        }
    }
    buffer_.reset();
    stream_.close();
}

void Stream::WriteBuffer() {
    stream_.write(buffer_.get(), cur_byte_);
    stream_.flush();
    for (size_t j = 0; j < cur_byte_; ++j) {
        buffer_[j] = 0;
    }
    cur_byte_ = 0;
}

void Stream::Write(const std::vector<bool>& data) {
    for (const bool& bit : data) {
        if (bit) {
            buffer_[cur_byte_] |= (1 << (byte_size_ - 1 - bits_rem_));
        }
        ++bits_rem_;
        if (bits_rem_ == byte_size_) {
            ++cur_byte_;
            bits_rem_ = 0;
        }
        if (cur_byte_ == buffer_size_) {
            WriteBuffer();
        }
    }
}

void Stream::WriteByte(const char& data) {
    if (cur_byte_ == buffer_size_) {
        WriteBuffer();
    }
    buffer_[cur_byte_] = data;
    ++cur_byte_;
}

void Stream::WriteNumber(size_t data, size_t bits) {
    std::vector<bool> bit(bits, false);
    size_t i = 0;
    while (data > 0) {
        bit[bits - i - 1] = (data & 1);
        data /= 2;
        ++i;
    }
    Write(bit);
}
