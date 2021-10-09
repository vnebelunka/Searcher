#include <string>
#include <vector>
#include <map>
#include <set>
#ifndef SEARCHER_SEARCHER_H
#define SEARCHER_SEARCHER_H


/*
 * TODO: refactor; Serelalize/deserealize; Binsearch; CleverSearch; compession; Make; divide main
 *
 */

class Searcher{
private:
    struct doc{
        std::string url;
        unsigned docID;
        std::vector<std::string> words;
    };


    std::vector<doc> index;
    std::map<std::string, std::vector<unsigned>> inverse_index;
    size_t index_size;
    std::map<std::string, std::vector<unsigned char>> inverse_index_compressed;
    size_t compressed_size;
    unsigned docid = 1;

public:
    std::map<unsigned, std::string> ID_url;

    void prep_index(const std::string& file_name);
    void prep_inverse_index();

    void write_inverse_index(const std::string& file_name);
    void write_ids_to_urls(const std::string& file_name);
    void write_compressed_index(const std::string& file_name);

    void load_ids_to_urls(const std::string& file_name);
    void load_inverse_index(const std::string& file_name);
    void load_compressed_index(const std::string& file_name);

    void compress_varbyte_index();


    Searcher(){
        index_size = 0;
        compressed_size = 0;
    }


    void search(std::vector<std::string>);


    size_t get_index_size();
    size_t get_compressed_size();


    struct Node {
        int op;
        const std::vector<unsigned char> *v;
        Node *left;
        Node *right;
        Node(int op,std::vector<unsigned char>*, Node *l, Node *r);
    };

    std::vector<std::string> split(const std::string& s, char delimiter);

private:
    std::vector<unsigned> naive_search(std::vector<std::string>);
    std::vector<unsigned> stream_search(std::vector<std::string>);
    static std::vector<unsigned char> compress_varbyte(std::vector<unsigned> v);
    static std::vector<unsigned> decompress_varbyte(const std::vector<unsigned char>& v);


    Node* create_tree(std::vector<std::string> vs, int first_index, int last_index);
    unsigned proceed(Node *node, unsigned id);
};




#endif //SEARCHER_SEARCHER_H
