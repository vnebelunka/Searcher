#include <fstream>
#include <iostream>
#include "Searcher.h"
#include <cstring>

int main(int argc, char **argv) {
    setlocale(LC_ALL, "utf-8");
    Searcher searcher;
    if(argc != 1 && !strcmp(argv[1], "--prep")) {
        for(int i = 1; i <= 8; ++i){
            std::string filename = "/home/nebil/CLionProjects/Searcher/data/lenta.ru_0";
            filename += i + '0';
            filename += ".txt";
            std::cerr << "prep index from file: " << filename <<"\n";
            searcher.prep_index(filename);
            std::cerr << "adding " << filename << " to inverse_index\n";
            searcher.prep_inverse_index();
        }
        std::cerr << "------\n" << "Index size is " << searcher.get_index_size() << "\n------\n";
        std::cerr<<"writing inverse_index.txt"<<"\n";
        searcher.write_inverse_index("./data/inverse_index.txt");
        std::cerr<<"writing map {id : url}\n";
        searcher.write_ids_to_urls("./data/ids_urls.txt");
        //
        std::cerr<<"compressing index\n";
        searcher.compress_varbyte_index();
        std::cerr<<"------\nCompressed size is " << searcher.get_compressed_size()<<"\n------\n";
        searcher.write_compressed_index("./data/compressed.txt");
    }
    else {
        std::cerr<<"loading index from file\n";
        searcher.load_inverse_index("./data/inverse_index.txt");
        searcher.load_ids_to_urls("./data/ids_urls.txt");
    }
    std::cerr<<"compressing index\n";
    searcher.compress_varbyte_index();
    std::cout<<"Ready to request!\n\n";
    std::string request;
    while(getline(std::cin, request)){
        std::vector<std::string> words = searcher.split(request, ' ');
        searcher.search(words);
    }
    return 0;
}
