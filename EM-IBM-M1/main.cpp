#include <iostream>
#include <corpus_reader.h>
#include <EM-IBM_2.h>
#include <fstream>
#include <functional>
#include <boost/thread.hpp>


int main()
{
    IBM ibm;
    ibm.read();
    ibm.foo();
    std::cout<<ibm.corpus.size()<<" "<<ibm.size_of_corpus;
//    corpus_reader reader("alignment-de-fr.txt");
//    std::pair<std::vector<std::string>, std::vector<std::string>> p;
//    while (!reader.eof()) {
//        p = reader.read_sentenses();
//        if (reader.eof()) {
//            std::cout<<"@"<<std::endl;
//        }
//        reader.read_alignment();
//        for (int i = 0; i < p.first.size(); ++i) {
//            std::cout << p.first[i]<< " ";
//        }
//        std::cout<< std::endl;
//        for (int i = 0; i < p.second.size(); ++i) {
//            std::cout << p.second[i]<< " ";
//        }
//        std::cout<<std::endl;
//        std::cout<< "!"<<std::endl;
//    }
//    for (int i = 0; i < p.first.size(); ++i) {
//        std::cout << p.first[i]<< " ";
//    }
//    std::cout<< std::endl;
//    for (int i = 0; i < p.second.size(); ++i) {
//        std::cout << p.second[i]<< " ";
//    }
    std::cout << std::endl;
    return 0;
}

