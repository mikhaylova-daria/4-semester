#ifndef CORPUS_READER_H
#define CORPUS_READER_H
#include <fstream>
#include <vector>
#include <map>
//#include <ctype.h>
std::map<std::string, unsigned int> sourse_dictionary;
std::map<std::string, unsigned int> target_dictionary;
std::map<unsigned int, std::string> sourse_dictionary_decod;
std::map<unsigned int, std::string> target_dictionary_decod;
class corpus_reader {
    std::ifstream i_stream;
    bool eof_flag = false;
    bool finish_sentense_flag = false;
    unsigned int sourse_dictionary_size = 0;
    unsigned int target_dictionary_size = 0;
public:
    corpus_reader(const char* input_file_name):i_stream(input_file_name) {
    }
    ~corpus_reader(){;}

    std::string read_word() {
        char c;
        std::string word;
        c = i_stream.get();
        while (!isspace(c) && !i_stream.eof()) {
            word.push_back(c);
            c = i_stream.get();
        }
        if (c == '\n') {
            finish_sentense_flag = true;
        }
        if (i_stream.eof()) {
            finish_sentense_flag = true;
            eof_flag = true;
        }
        return word;
    }

    std::pair<std::vector<unsigned int>, std::vector<unsigned int> > read_sentenses() {
        std::vector<unsigned int> sentense_sourse_lang;
        std::vector<unsigned int> sentense_target_lang;
        while (!finish_sentense_flag) {
            std::string word = read_word();
            std::pair<std::map<std::string, unsigned int>::iterator, bool> insert_itr = sourse_dictionary.insert
                    (std::pair<std::string, unsigned int>(word, sourse_dictionary_size));
            if (insert_itr.second) {
                sourse_dictionary_decod.insert(std::pair<unsigned int, std::string>(sourse_dictionary_size, word));
                ++sourse_dictionary_size;
            }
            sentense_sourse_lang.push_back(insert_itr.first->second);
        }
        finish_sentense_flag = false;
        while (!finish_sentense_flag) {
            std::string word = read_word();
            std::pair<std::map<std::string, unsigned int>::iterator, bool> insert_itr = target_dictionary.insert
                    (std::pair<std::string, unsigned int>(word, target_dictionary_size));
            if (insert_itr.second) {
                target_dictionary_decod.insert(std::pair<unsigned int, std::string>(target_dictionary_size, word));
                ++target_dictionary_size;
            }
            sentense_target_lang.push_back(insert_itr.first->second);
        }
        finish_sentense_flag = false;
        return std::make_pair < std::vector<unsigned int>, std::vector<unsigned int> >
                (std::move(sentense_sourse_lang), std::move(sentense_target_lang));
    }

    std::vector<std::string> read_alignment() {
        std::vector<std::string> alignment;
        while (!finish_sentense_flag) {
            alignment.push_back(read_word());
        }
        finish_sentense_flag = false;
        return alignment;
    }

    bool eof() {
            return eof_flag;
    }
};

#endif // CORPUS_READER_H
