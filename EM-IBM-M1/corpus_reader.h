#ifndef CORPUS_READER_H
#define CORPUS_READER_H
#include <fstream>
#include <vector>
#include <ctype.h>

class corpus_reader {
    std::ifstream i_stream;
    bool eof_flag = false;
    bool finish_sentense_flag = false;
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

    std::pair<std::vector<std::string>, std::vector<std::string> > read_sentenses() {
        std::vector<std::string> sentense_sourse_lang;
        std::vector<std::string> sentense_target_lang;
        while (!finish_sentense_flag) {
            sentense_sourse_lang.push_back(read_word());
        }
        finish_sentense_flag = false;
        while (!finish_sentense_flag) {
            sentense_target_lang.push_back(read_word());
        }
        finish_sentense_flag = false;
        return std::make_pair < std::vector<std::string>, std::vector<std::string> >
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
