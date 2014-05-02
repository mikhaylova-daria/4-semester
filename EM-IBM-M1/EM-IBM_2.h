#ifndef EMIBM_2_H
#define EMIBM_2_H
#include <corpus_reader.h>
#include <map>
#include <vector>

class IBM {
public:
    std::vector<std::vector<std::vector<float>>> c_e_f_k;
    std::map<std::pair<std::string, std::string>, float> c_e_f;
    std::map<std::string, float> c;
    std::map<std::pair<std::string, std::string>, float> t_map;
    std::vector<std::vector<float>> c_e;
    std::vector<std::vector<std::vector<float>>> c_j_cond_i_l_m;
    std::vector<std::vector<std::vector<std::vector<float>>>> c_i_l_m;
    std::vector<std::vector<std::vector<float>>> delta;
    std::vector<std::vector<std::vector<double>>> t;
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> corpus;
    std::vector<std::vector<int>> align;
    corpus_reader reader;
    int size_of_corpus = 0;
public:
    IBM(/*int size_of_corpus*/): reader("/home/darya/Алгоритмы/build-EM-IBM-M1-Qt_5_1_1_gcc_64-Debug/alignment-de-en.txt")/*, c(size_of_corpus), delta(size_of_corpus), t(size_of_corpus)*/{
    }
    ~IBM(){}
    void read() {
        std::pair<std::vector<std::string>, std::vector<std::string>> p;
        while (!reader.eof()) {
            p = reader.read_sentenses();
            reader.read_alignment();
            if (!reader.eof()) {
                ++size_of_corpus;
                corpus.push_back(p);
            }
        }
        align.resize(size_of_corpus);
        for (int i = 0; i < size_of_corpus; ++i) {
            align[i].resize(corpus[i].first.size());
        }
        delta = std::vector<std::vector<std::vector<float>>>(size_of_corpus + 1);
        c_e_f_k = std::vector<std::vector<std::vector<float>>> (size_of_corpus + 1);
        c_e = std::vector<std::vector<float>>(size_of_corpus + 1);
        t = std::vector<std::vector<std::vector<double>>> (size_of_corpus + 1);
        for (int k = 1; k <= size_of_corpus; ++k) {
            delta[k].resize(corpus[k - 1].second.size() + 1);
            t[k].resize(corpus[k - 1].second.size() + 1);
            c_e[k].resize(corpus[k - 1].first.size() + 1);
            c_e_f_k[k].resize(corpus[k - 1].first.size() + 1);
            for (int i = 1; i <= corpus[k - 1].second.size(); ++i) {
                delta[k][i].resize(corpus[k - 1].first.size() + 1);
                t[k][i].resize(corpus[k - 1].first.size() + 1);
            }
            for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
                c_e_f_k[k][j].resize(corpus[k - 1].second.size() + 1);
            }
            for (int i = 0 ; i < corpus[k - 1].first.size(); ++i) {
                c.insert(std::pair<std::string, float> (corpus[k-1].first[i], 0));
                for (int j = 0; j < corpus[k-1].second.size(); ++j) {
                    c_e_f.insert(std::pair<std::pair<std::string, std::string>, float>
                                 (std::pair<std::string, std::string> (corpus[k-1].first[i], corpus[k-1].second[j]), 0));
                    t_map.insert(std::pair<std::pair<std::string, std::string>, float>
                                 (std::pair<std::string, std::string> (corpus[k-1].first[i], corpus[k-1].second[j]), (double)(1+k)/(double)(k+j)));

                }
            }
        }
//        for (int k = 1; k <= size_of_corpus; ++k) {
//            for (int i = 1; i <= corpus[k - 1].second.size(); ++i) {
//                for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
//                    t[k][i][j] = 2 - (double)(1+i+k)/(double)(k+i+j);
//                    //std::cout<<t[k][i][j]<<std::endl;
//                    //int n;
//                    //std::cin>>n;
//                }
//            }
//        }
    }

    void  foo() {
        for (int p = 0; p < 5; ++p) {
            //delta = std::vector<std::vector<std::vector<float>>>(size_of_corpus + 1);
            //c_e_f_k = std::vector<std::vector<std::vector<float>>> (size_of_corpus + 1);
            //c_e = std::vector<std::vector<float>>(size_of_corpus + 1);
          //  t = std::vector<std::vector<std::vector<double>>> (size_of_corpus + 1);
            for (int k = 1; k <= size_of_corpus; ++k) {
                for (int i = 0 ; i < corpus[k - 1].first.size(); ++i) {
                    c.insert(std::pair<std::string, float> (corpus[k-1].first[i], 0));
                    for (int j = 0; j < corpus[k-1].second.size(); ++j) {
                        c_e_f.insert(std::pair<std::pair<std::string, std::string>, float>
                                     (std::pair<std::string, std::string> (corpus[k-1].first[i], corpus[k-1].second[j]), 0));
                    }
                }
              //  delta[k].resize(corpus[k - 1].second.size() + 1);
                //t[k].resize(corpus[k - 1].second.size() + 1);
//                c_e[k] = std::vector<float>(corpus[k - 1].first.size() + 1, 0);
//                for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
//                    c_e_f_k[k][j] = std::vector<float>(corpus[k - 1].second.size() + 1, 0);
//                }
          }
//            for (int k = 1; k <= size_of_corpus; ++k) {
//                for (int i = 1; i <= corpus[k - 1].second.size(); ++i) {
//                    for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
//                  //      t[k][i][j] = (double)(1+i)/(double)(k+i+j);
//                        //std::cout<<t[k][i][j]<<std::endl;
//                        //int n;
//                        //std::cin>>n;
//                    }
//                }
//            }
          //  std::cout<<"!!!"<<std::endl;
            for (int k = 1; k <= size_of_corpus; ++k) {
                for (int i = 1; i <= corpus[k - 1].second.size(); ++i) {
                    for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
                        float sum = 0;
                       // std::cout<<"57"<<std::endl;
                        for (int h = 0; h <= corpus[k - 1].first.size(); ++h) {
                            //sum += t[k][i][h];
                            sum += t_map.find(std::pair<std::string, std::string>(corpus[k-1].first[h], corpus[k-1].second[i]))->second;
                        }
                       // std::cout<<"61"<<std::endl;
                        delta[k][i][j] = t_map.find(std::pair<std::string, std::string>(corpus[k-1].first[j], corpus[k-1].second[i]))->second/sum;
                        //delta[k][i][j] = t[k][i][j]/sum;
                        //std::cout <<delta[k][i][j]<<std::endl;
                        //std::cout<<"63 "<<k<<" "<<j<<" "<<i<<" "<<c_e_f_k[k].size()<<std::endl;
                        //int n;
                        //std::cin>>n;
                        c_e_f_k[k][j][i] += delta[k][i][j];
                        c.find(corpus[k - 1].first[j])->second += delta[k][i][j];
                        c_e_f.find(std::pair<std::string, std::string> (corpus[k - 1].first[j], corpus[k - 1].second[i]))->second += delta[k][i][j];
                       // std::cout<<"65"<<std::endl;
                        c_e[k][j] += delta[k][i][j];
                        //c_j_cond_i_l_m[j][i][l][m] += delta[k][i][j];
                        //c_i_l_m[i][l][m] += delta[k][i][j];
                    }
                }
            }
            for (int k = 1; k <= size_of_corpus; ++k) {
                for (int i = 1; i <= corpus[k - 1].second.size(); ++i) {
                    for (int j = 0; j <= corpus[k - 1].first.size(); ++j) {
                        //std::cout<<k<<" "<<i<<" "<<j<<std::endl;
                        t_map.find(std::pair<std::string, std::string>(corpus[k-1].first[j], corpus[k-1].second[i]))->second= c_e_f.find(std::pair<std::string, std::string>(corpus[k-1].first[j], corpus[k-1].second[i]))->second / c.find(corpus[k-1].first[j])->second;
                        //t[k][i][j] = c_e_f_k[k][j][i] / c_e[k][j];
                        //std::cout<<t[k][i][j]<<std::endl;
                        //q = [j][i][l][m] =
                    }

                }
            }
        }


        for (int i = 1; i <= corpus[0].second.size(); ++i) {
            for (int l = 0; l < corpus[0].second.size(); ++l) {
              std::cout<<corpus[0].second[l]<<" ";
            }
            float max = 0;
            int a = 0;
            for (int j = 0; j <= corpus[0].first.size(); ++j) {
                //std::cout << t[1][i][j]<<std::endl;
                if (t_map.find
                        (std::pair<std::string, std::string>(corpus[0].first[j], corpus[0].second[i]))->second >= max) {
                    a = j;
                    max = t_map.find
                            (std::pair<std::string, std::string>(corpus[0].first[j], corpus[0].second[i]))->second;
                    //max = t[1][i][j];
                }
            }
            std::cout << a<<std::endl;
        }
    }
};

#endif // EMIBM_2_H
