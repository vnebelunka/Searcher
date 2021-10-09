#include "Searcher.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

Searcher::Node::Node(int op1,std::vector<unsigned char>*v1, Node *l, Node *r) : v(v1) {
    op = op1;
    left = l;
    right = r;
}

inline int dict_op_to_code(std::string s){
    if(s == "&")
        return 1;
    else return 2; // "|"
}

Searcher::Node* Searcher::create_tree(std::vector<std::string> vs, int first_index, int last_index) {

    using Node = Searcher::Node;

    if((unsigned)last_index >= vs.size())
        return nullptr;
    if (last_index == first_index) {
        // return { 0, dict_word_to_pointer[vs[0]], 0, 0 };
        auto it = inverse_index_compressed.find(vs[last_index]);
        if(it == inverse_index_compressed.end()){
            return nullptr;
        }
        Node *head = new Node{0, &it->second, nullptr, nullptr};
        return head;
    }
    int i = first_index;

    while ((vs[i] != "|" && vs[i] != "&" && vs[i] != "(") && i < last_index) {
        i++;
    }
    if (vs[i] == "(") {
        int balance = 1, j = i + 1;
        while (balance != 0) {
            if (vs[j] == "(") {
                balance++;
            }
            if (vs[j] == ")") {
                balance--;
            }
            j++;
        }
        --j;
        // return { dict_op_to_code[vs[j + 1]], 0, create_tree(vs, i, j), create_tree(vs, j + 2, last_index) };
        if(j + 1 <= last_index) {
            return new Node(dict_op_to_code(vs[j + 1]), nullptr, create_tree(vs, i + 1, j - 1),
                            create_tree(vs, j, last_index)); // если херня с верху не работает
        }
        else{
            return create_tree(vs, i+1, j-1);
        }
    }
    else {
        // return { dict_op_to_code[vs[i]], 0, create_tree(vs, first_index, i - 1), create_tree(vs, i + 1, last_index) };
        return new Node (dict_op_to_code(vs[i]), nullptr, create_tree(vs, first_index, i - 1), create_tree(vs, i + 1, last_index)); // аналогично
    }
}


unsigned binsearch(std::vector<unsigned>&v, unsigned anch) {
    if (anch > v.back()) {
        return 0;
    }
    unsigned sz = v.size();
    unsigned l = 0, r = sz;
    while (l + 1 != r) {
        unsigned m = (l + r) / 2;
        if (v[m] > anch) {
            r = m;
            continue;
        }
        if (v[m] < anch) {
            l = m;
            continue;
        }
        return v[m];
    }
    if (anch > v[l]) {
        return v[r];
    }
    return v[l];
}


unsigned Searcher::proceed(Node *node, unsigned id){
    if(node->op == 0){
        std::vector<unsigned> temp = decompress_varbyte(*node->v);
        return binsearch(temp, id);
    }
    if(!node->left || !node->right){
        return 0;
    }
    unsigned n1 = proceed(node->left, id), n2 = proceed(node->right, id);
    if(node->op == 2){
        if(n1 == 0 && n2 == 0)
            return 0;
        if(n1 == 0 || n2 == 0)
            return n1 + n2;
        return std::min(n1, n2);
    }
    if(node->op == 1){
        while (n1 != n2 && n1 != 0 && n2 != 0){
            if(n1 > n2)
                n2 = proceed(node->right, n1);
            else
                n1 = proceed(node->left, n2);
        }
        if(n1 == 0 || n2 == 0)
            return 0;
        return n1; // == n2
    }
    std::cerr<<"Can't proceed operand";
    return 0;
}


std::vector<unsigned> Searcher::stream_search(std::vector<std::string> words){
    Node * head = create_tree(words, 0, words.size() - 1);
    std::vector<unsigned> ans;
    unsigned id = 0;
    while(1){
        id = proceed(head, id);
        if(id == 0)
            break;
        else
            ans.push_back(id++);
    }
    return ans;
}