#ifndef TEST_H
#define TEST_H
#include <fstream>
#include <corpus_reader.h>
#include <vector>
#include <set>

void report(const char* original_file = "/home/darya/Алгоритмы/build-EM-IBM-M1-Qt_5_1_1_gcc_64-Debug/alignment-en-fr.txt", const char* result_file = "output.txt" ) {
    corpus_reader original(original_file);
    corpus_reader result(result_file);
    std::ofstream output("output_test.txt");
    std::pair<std::vector<unsigned int>, std::vector<unsigned int>> p;
    std::vector<std::string> original_al;
    std::vector<std::string> result_al;
    std::set<std::string> original_al_set;
    std::ofstream worst_results("worst_results.txt");
    std::ofstream best_results("best_results.txt");
    float average_value = 0;
    int corpus_size = 0;
    int count_hits = 0;
    int count_worst = 0;
    int count_best = 0;
    while (!original.eof()) {
        ++corpus_size;
        count_hits = 0;
        p = original.read_sentenses();
        result.read_sentenses();
        original_al = original.read_alignment();
        result_al = result.read_alignment();
        original_al_set.clear();
        original_al_set.insert(original_al.begin(), original_al.end());
        for (int i = 0; i < result_al.size(); ++i) {
            if (original_al_set.find(result_al[i]) != original_al_set.end()) {
                ++count_hits;
            }
        }
        for (int i = 0; i < p.first.size(); ++i) {
            output<<sourse_dictionary_decod.find(p.first[i])->second<<" ";
        }
        output<<std::endl;
        for (int i = 0; i < p.second.size(); ++i) {
            output<<target_dictionary_decod.find(p.second[i])->second<<" ";
        }
        output<<std::endl;
        float current_result =(float)count_hits/(float)original_al.size();
        output<<current_result<<std::endl;
        average_value += current_result;
        if (current_result < 0.5) {
            ++count_worst;
            for (int i = 0; i < p.first.size(); ++i) {
                worst_results<<sourse_dictionary_decod.find(p.first[i])->second<<" ";
            }
            worst_results<<std::endl;
            for (int i = 0; i < p.second.size(); ++i) {
                worst_results<<target_dictionary_decod.find(p.second[i])->second<<" ";
            }
            worst_results<<std::endl;
            worst_results<<current_result<<std::endl;
        }
        if (current_result > 0.8) {
            ++count_best;
            for (int i = 0; i < p.first.size(); ++i) {
                best_results<<sourse_dictionary_decod.find(p.first[i])->second<<" ";
            }
            best_results<<std::endl;
            for (int i = 0; i < p.second.size(); ++i) {
                best_results<<target_dictionary_decod.find(p.second[i])->second<<" ";
            }
            best_results<<std::endl;
            best_results<<current_result<<std::endl;
        }
    }
    std::cout<<"average value: "<<average_value/corpus_size<<std::endl;
    std::cout<<"count_worst(<50% hits): "<<count_worst<<std::endl;
    std::cout<<"count_best(>80% hits): "<<count_best<<std::endl;
    std::cout<<"more information in files \"output.txt\",\"output_test.txt\", \"worst_results.txt\",\"best_results.txt\""<<std::endl;
}

#endif // TEST_H
