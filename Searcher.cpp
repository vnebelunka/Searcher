#include "Searcher.h"
#include <utility>
#include <vector>
#include <fstream>
#include <regex>
#include <algorithm>
#include <iterator>
#include <iostream>

std::vector<std::string> Searcher::split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    std::set<char> marks = {',', ':', '?', '!', '.', ')'};
    while (std::getline(tokenStream, token, delimiter))
    {
        if(marks.find(token.back()) != marks.end())
            token.pop_back();
        tokens.push_back(token);
    }
    return tokens;
}

void Searcher::prep_index(const std::string &file_name) {
    //TODO: лемматизация
    setlocale(LC_ALL, "utf-8");
    std::ifstream file(file_name);
    if(!file.is_open()){
        std::cerr<<"Can't open file:" << file_name<<std::endl;
        return;
    }
    std::string text, temp;
    doc curdoc;
    std::regex url_regex (
            R"(http:[-a-zA-Z0-9@:%_+.~#?&//=]+)");
    std::smatch url_match_result;
    while(getline(file,temp)){
        std::transform(temp.begin(), temp.end(), temp.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        std::smatch m;
        if(std::regex_search(temp, m, url_regex)){
            if(!curdoc.url.empty()) {
                index.push_back(curdoc);
                ID_url.insert({curdoc.docID, curdoc.url});
            }
            curdoc.url = m[0];
            curdoc.words.clear();
            curdoc.docID = docid++;
        }
        else{
            std::vector<std::string> words = split(temp, ' ');
            for(const auto& word: words){
                curdoc.words.push_back(word);
            }
        }
    }
}


void Searcher::prep_inverse_index() {
    for(const auto& doc : index){
        for(const auto& word : doc.words){
            auto it = inverse_index.find(word);
            if(it != inverse_index.end()){
                if(it->second.back() != doc.docID) {
                    it->second.push_back(doc.docID);
                }
            }
            else{
                std::vector<unsigned> temp;
                temp.push_back(doc.docID);
                inverse_index.insert({word, temp});
            }
        }
    }
    index.clear();
}

std::vector<unsigned> Searcher::naive_search(std::vector<std::string> words) {
    std::string curword = words[0];
    std::vector<unsigned> ans;
    if(!inverse_index.empty()) {
        auto it = inverse_index.find(curword);
        if(it == inverse_index.end()){
            std::cout<<"Cant find word in index: " << curword << std::endl;
            return {};
        }
        ans = it->second;
    }
    else{
        auto it = inverse_index_compressed.find(curword);
        if(it == inverse_index_compressed.end()){
            std::cout<<"Cant find word in index: " << curword << std::endl;
            return {};
        }
        ans = decompress_varbyte(it->second);
    }
    for(int i = 1; i < words.size(); ++i){
        curword = words[i];
        std::vector<unsigned> temp;
        if(!inverse_index.empty()) {
            auto it = inverse_index.find(curword);
            if(it == inverse_index.end()){
                std::cout<<"Cant find word in index: " << curword << std::endl;
                return {};
            }
            temp = it->second;
        }
        else{
            auto it = inverse_index_compressed.find(curword);
            if(it == inverse_index_compressed.end()){
                std::cout<<"Cant find word in index: " << curword << std::endl;
                return {};
            }
            temp = decompress_varbyte(it->second);
        }
        std::vector<unsigned> intersect;
        set_intersection(ans.begin(), ans.end(), temp.begin(), temp.end(),
                        std::inserter(intersect, intersect.begin()));
        ans = std::move(intersect);
    }
    return ans;
}


void Searcher::search(std::vector<std::string> words){
    //auto ids = naive_search(std::move(words));
    auto ids = stream_search(std::move(words));
    std::cout<<"Results are:\n";
    for(auto id : ids){
        std::cout<<ID_url.find(id)->second<<std::endl;
    }
    std::cout<<"Ready for next search\n";
}


void Searcher::write_inverse_index(const std::string &file_name) {
    setlocale(LC_ALL, "utf-8");
    std::ofstream fout;
    fout.open(file_name);
    for (const auto &el : inverse_index) {
        fout << el.first <<": ";
        for (const auto& s : el.second) {
            fout << s << " ";
        }
        fout<<std::endl;
    }
}

void Searcher::load_inverse_index(const std::string &file_name){
    setlocale(LC_ALL, "utf-8");
    std::ifstream fin;
    fin.open(file_name);
    std::string s;
    while(getline(fin, s)){
        std::vector<std::string> words = split(s, ' ');
        std::string word = words[0];
        std::vector<unsigned> ids;
        for(int i = 1; i < words.size(); ++i){
            ids.push_back(std::stoi(words[i]));
        }
        inverse_index.insert({word, ids});
    }
}





void Searcher::write_ids_to_urls(const std::string &file_name) {
    setlocale(LC_ALL, "utf-8");
    std::ofstream fout;
    fout.open(file_name);
    for(const auto& el: ID_url){
        fout<<el.first<<" "<<el.second<<std::endl;
    }
}

void Searcher::load_ids_to_urls(const std::string &file_name) {
    setlocale(LC_ALL, "utf-8");
    std::ifstream fin;
    fin.open(file_name);
    std::string s;
    while(getline(fin, s)) {
        std::vector<std::string> words = split(s, ' ');
        ID_url.insert({std::stoi(words[0]), words[1]});
    }
}

void Searcher::write_compressed_index(const std::string &file_name) {
    setlocale(LC_ALL, "utf-8");
    std::ofstream fout;
    fout.open(file_name);
    for (const auto &el : inverse_index_compressed) {
        fout << el.first <<": ";
        for (const auto& s : el.second) {
            fout << s << " ";
        }
        fout<<std::endl;
    }
}

void Searcher::load_compressed_index(const std::string &file_name) {
    setlocale(LC_ALL, "utf-8");
    std::ifstream fin;
    fin.open(file_name);
    std::string s;
    while(getline(fin, s)){
        std::vector<std::string> words = split(s, ' ');
        std::string word_s = words[0];
        std::vector<unsigned char> temp(words.size());
        for(int i = 1; i < words.size(); ++i)
            temp.push_back((unsigned char)words[i][0]);
        //std::vector<unsigned> ids = decompress_varbyte(temp);
        inverse_index_compressed.insert({word_s, temp});
        //for(int i = 1; i < words.size(); ++i){
        //    ids.push_back((temp[i]));
        //}
        //inverse_index.insert({word_s, ids});
    }
}


std::vector<unsigned char> Searcher::compress_varbyte(std::vector<unsigned int> v) {
    std::vector<unsigned char> ans;
    std::vector<unsigned> diff(v.size());
    diff[0] = v[0];
    for(int i = 1; i < v.size(); ++ i){
        diff[i] = v[i] - v[i-1];
    }

    for(auto a : diff){
        unsigned char temp = 0;
        while(a >= 128){
            temp |= a & 0x7f; // 0111_1111
            ans.push_back(temp);
            temp = 0;
            a >>= 7;
        }
        if(a == 128){
            ans.push_back((unsigned char) 0x81);
        }
        else{
            temp = 0x80;
            temp |= a & 0x7f; // 0111_1111
            ans.push_back(temp);
        }
    }
    return ans;
}

std::vector<unsigned> Searcher::decompress_varbyte(const std::vector<unsigned char>& v) {
    std::vector<unsigned> ans;
    int shift = 0;
    int temp = 0;
    for(auto a : v){
        if(a >= 128){
            temp |= (a & 0x7f) << (7 * shift);
            shift = 0;
            if(ans.empty()) {
                ans.push_back(temp);
            }
            else{
                ans.push_back(temp + ans.back());
            }
            temp = 0;
        }
        else{
            temp |= a << (8 * shift);
            ++shift;
        }
    }
    return ans;
}

void Searcher::compress_varbyte_index(){
    index_size = 0;
    for(auto it = inverse_index.begin(); it != inverse_index.end();){
        std::string word = it->first;
        std::vector<unsigned char> temp = compress_varbyte(it->second);
        inverse_index_compressed.insert({word, temp});
        compressed_size += temp.size() * sizeof(unsigned char);
        inverse_index.erase(it++);
    }
}

size_t Searcher::get_index_size() {
    if(index_size != 0 && !inverse_index.empty())
        return index.size();
    size_t ans = 0;
    for(const auto& token : inverse_index){
        ans += token.second.size() * sizeof(unsigned);
    }
    index_size = ans;
    return ans;
}

size_t Searcher::get_compressed_size() {
   return compressed_size;
}