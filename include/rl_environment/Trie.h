//
// Created by anthony on 7/23/23.
//

#ifndef CODEMEALIFE_TRIE_H
#define CODEMEALIFE_TRIE_H

#include <concepts>
#include <type_traits>
#include <vector>
#include <array>


template <typename T, std::size_t max_children = 20>
class Trie
{
public:
    struct TrieNode
    {
        std::array<T, max_children> keys{};
        std::array<TrieNode*, max_children> children{};
        int num_keys{};

        TrieNode() = default;
    };

private:
    TrieNode* root;

public:
    Trie() : root(new TrieNode()) {}

    void destructor_helper(TrieNode* t)
    {
        if (t == nullptr)
            return;

        for (int i = 0; i < t->num_keys; ++i)
            destructor_helper(t->children[i]);

        delete t;
    }

    ~Trie()
    {
        destructor_helper(root);
    }

    void insert_sequence(const std::vector<T>& seq)
    {
        TrieNode* cur = root;

        for (auto & ele : seq)
        {
            int i, nk = cur->num_keys;

            for (i = 0; i < nk; ++i)
            {
                if (cur->keys[i] == ele)
                {
                    cur = cur->children[i];
                    break;
                }
            }

            if (i == nk)
            {
                ++cur->num_keys;
                cur->keys[i] = ele;
                cur->children[i] = new TrieNode();
                cur = cur->children[i];
            }
        }

        cur->keys[cur->num_keys++] = T{}; /* Use default value of T to denote end-of-sequence, this means zero can't be a valid token for integer types */
        /* No need to put a new TrieNode object in cur->children because it's never going to be accessed anyway */
    }

    class Iterator
    {
        TrieNode* cur = nullptr;

    public:
        explicit Iterator(TrieNode* root)
        {
            this->cur = root;
        }

        bool operator == (const Iterator& rhs) const
        {
            return this->cur == rhs.cur;
        }

        std::pair<const std::array<T, max_children>*, int> get_options()
        {
            if (cur == nullptr)
                return {nullptr, 0};

            return {&cur->keys, cur->num_keys};
        }

        void inc(const T& option)
        {
            if (cur == nullptr)
                return;

            for (int i = 0; i < this->cur->num_keys; ++i)
            {
                if (this->cur->keys[i] == option)
                    this->cur = this->cur->children[i];
            }
        }

        bool is_end() const
        {
            return this->cur->num_keys == 1 and this->cur->keys[0] == T{};
        }
    };

    Iterator begin()
    {
        return Iterator(root);
    }

    Iterator end()
    {
        return Iterator(nullptr);
    }
};


#endif //CODEMEALIFE_TRIE_H
