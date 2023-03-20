#pragma once

#include <vector>

template <typename T, typename K = std::less<T>>
class PriorityQueue {

public:
    void Pop() {
        if (Size() == 0) {
            throw std::runtime_error("Can't pop from empty queue!");
        }
        std::swap(data_[0], data_[data_.size() - 1]);
        data_.pop_back();
        SiftDown(0);
    }

    size_t Size() const {
        return data_.size();
    }

    const T Top() const {
        if (Size() == 0) {
            throw std::runtime_error("Can't get top element from empty queue!");
        }
        return data_[0];
    }

    void Push(const T& x) {
        size_t i = data_.size();
        data_.push_back(x);
        SiftUp(i);
    }

private:
    void SiftDown(size_t i) {
        while (2 * i + 1 < data_.size()) {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            size_t j = left;
            if (right < data_.size() && K()(data_[right], data_[left])) {
                j = right;
            }
            if (K()(data_[i], data_[j])) {
                break;
            }
            std::swap(data_[i], data_[j]);
            i = j;
        }
    }

    void SiftUp(size_t i) {
        while (i > 0 && K()(data_[i], data_[(i - 1) / 2])) {
            std::swap(data_[i], data_[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    std::vector<T> data_;
};
