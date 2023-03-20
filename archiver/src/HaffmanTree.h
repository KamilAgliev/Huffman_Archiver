#pragma once
#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <algorithm>
#include <memory>
#include <optional>
#include "PriorityQueue.h"

class HaffmanTree {
private:
    struct Node {
        size_t char_num;
        size_t count;
        std::shared_ptr<Node> right = nullptr;
        std::shared_ptr<Node> left = nullptr;

        Node(size_t char_num, size_t count);
    };

    struct NodePtrCmp {
        bool operator()(const std::shared_ptr<HaffmanTree::Node> &a, const std::shared_ptr<HaffmanTree::Node> &b) const;
    };

    std::shared_ptr<HaffmanTree::Node> BuildTree();

    PriorityQueue<std::shared_ptr<Node>, NodePtrCmp> current_nodes_;
    std::shared_ptr<Node> root_;
    std::vector<std::pair<size_t, size_t>> haffman_codes_;
    std::unordered_map<size_t, std::vector<bool>> kanonic_codes_;
    std::unordered_map<size_t, size_t> symbol_lenghts_;

    static void AddBinOne(std::vector<bool> &str);

    void BuildHaffmanLength();

    void BuildKanonicCodes();

public:
    explicit HaffmanTree(const std::unordered_map<size_t, size_t> &counts);

    std::unordered_map<size_t, std::vector<bool>> &GetKanonicCodes();

    std::vector<std::pair<size_t, size_t>> &GetHaffmanCodes();

    static bool KanonicSort(const std::pair<size_t, size_t> &a, const std::pair<size_t, size_t> &b);

    struct TrieNode {
        std::optional<size_t> code;
        std::shared_ptr<TrieNode> right = nullptr;
        std::shared_ptr<TrieNode> left = nullptr;
    };

    static std::shared_ptr<TrieNode> RestoreKanonicCodes(std::vector<std::pair<size_t, size_t>> &symbols);
};