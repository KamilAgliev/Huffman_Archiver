#include "HaffmanTree.h"

HaffmanTree::Node::Node(size_t char_num, size_t count) {
    this->char_num = char_num;
    this->count = count;
}

bool HaffmanTree::NodePtrCmp::operator()(const std::shared_ptr<HaffmanTree::Node> &a,
                                         const std::shared_ptr<HaffmanTree::Node> &b) const {
    if (a->count != b->count) {
        return a->count < b->count;
    }
    return a->char_num < b->char_num;
}

HaffmanTree::HaffmanTree(const std::unordered_map<size_t, size_t> &counts) {
    for (auto &[char_num, count] : counts) {
        current_nodes_.Push(std::make_shared<Node>(char_num, count));
    }
    root_ = BuildTree();
    BuildHaffmanLength();
    BuildKanonicCodes();
}

std::shared_ptr<HaffmanTree::Node> HaffmanTree::BuildTree() {
    while (current_nodes_.Size() > 1) {
        std::shared_ptr<Node> a = current_nodes_.Top();
        current_nodes_.Pop();
        std::shared_ptr<Node> b = current_nodes_.Top();
        current_nodes_.Pop();
        std::shared_ptr<Node> new_node(new Node(std::min(a->char_num, b->char_num), a->count + b->count));
        new_node->left = a;
        new_node->right = b;
        a.reset();
        b.reset();
        current_nodes_.Push(new_node);
    }
    return current_nodes_.Top();
}

void HaffmanTree::BuildHaffmanLength() {
    std::queue<std::pair<size_t, std::shared_ptr<Node>>> node_queue;
    node_queue.push({0, root_});
    while (!node_queue.empty()) {
        auto current = node_queue.front();
        node_queue.pop();
        if (!current.second->left && !current.second->right) {
            symbol_lenghts_[current.second->char_num] = current.first;
        } else {
            if (!current.second->left || !current.second->right) {
                throw std::runtime_error("Haffman tree can't be built!");
            }
            node_queue.push({current.first + 1, current.second->left});
            node_queue.push({current.first + 1, current.second->right});
        }
    }
}

void HaffmanTree::BuildKanonicCodes() {
    for (auto &[char_num, length] : symbol_lenghts_) {
        haffman_codes_.push_back({char_num, length});
    }
    std::sort(haffman_codes_.begin(), haffman_codes_.end(), KanonicSort);
    std::vector<bool> current_str = {false};
    for (size_t i = 0; i < haffman_codes_.size(); ++i) {
        if (i > 0) {
            AddBinOne(current_str);
        }
        while (current_str.size() < symbol_lenghts_[haffman_codes_[i].first]) {
            current_str.push_back(false);
        }
        kanonic_codes_[haffman_codes_[i].first] = current_str;
    }
}

void HaffmanTree::AddBinOne(std::vector<bool> &bin) {
    size_t index0 = bin.size();
    while (index0 > 0 && bin[index0 - 1]) {
        --index0;
        bin[index0] = false;
    }
    if (index0 > 0) {
        bin[index0 - 1] = true;
    } else {
        throw std::runtime_error("Kanonic codes can't be built!");
    }
}

bool HaffmanTree::KanonicSort(const std::pair<size_t, size_t> &a, const std::pair<size_t, size_t> &b) {
    if (a.second != b.second) {
        return a.second < b.second;
    }
    return a.first < b.first;
}

std::unordered_map<size_t, std::vector<bool>> &HaffmanTree::GetKanonicCodes() {
    return kanonic_codes_;
}

std::shared_ptr<HaffmanTree::TrieNode> HaffmanTree::RestoreKanonicCodes(
    std::vector<std::pair<size_t, size_t>> &symbols) {
    std::vector<bool> current_str = {false};
    auto root = std::shared_ptr<TrieNode>(new TrieNode());

    for (size_t i = 0; i < symbols.size(); ++i) {
        if (i > 0) {
            AddBinOne(current_str);
        }
        while (current_str.size() < symbols[i].second) {
            current_str.push_back(false);
        }
        auto cur_node = root;
        for (const bool &bit : current_str) {
            if (!bit) {
                if (!cur_node->left) {
                    cur_node->left = std::shared_ptr<TrieNode>(new TrieNode());
                }
                cur_node = cur_node->left;
            } else {
                if (!cur_node->right) {
                    cur_node->right = std::shared_ptr<TrieNode>(new TrieNode());
                }
                cur_node = cur_node->right;
            }
        }
        cur_node->code = symbols[i].first;
    }
    return root;
}

std::vector<std::pair<size_t, size_t>> &HaffmanTree::GetHaffmanCodes() {
    return haffman_codes_;
}
