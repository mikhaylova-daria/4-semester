#ifndef EMIBM_2_H
#define EMIBM_2_H
#include <corpus_reader.h>
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>
#include <functional>

struct hash {
    std::hash<std::string> hasher;
    size_t operator()(const std::pair<std::string, std::string>& p) const {
        std::string conc;
        conc.append(p.first);
        conc.append(p.second);
        size_t answer = hasher(conc);
        return answer;
    }
};

class IBM {
public:
    std::unordered_map<std::pair<std::string, std::string>, std::vector<float>, hash> c_e_f;
    std::unordered_map<std::string, std::vector<float>> c;
    std::unordered_map<std::pair<std::string, std::string>, float, hash> t_map;
    std::vector<std::vector<std::vector<float>>> delta;
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> corpus;
    std::vector<std::vector<int>> align;
    corpus_reader reader;
    int size_of_corpus = 0;
    int number_of_process = 4;
public:
    IBM(): reader("/home/darya/Алгоритмы/build-EM-IBM-M1-Qt_5_1_1_gcc_64-Debug/alignment-de-en.txt")/*, c(size_of_corpus), delta(size_of_corpus), t(size_of_corpus)*/{
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
        //всякая хрень, которую нужно вынести в другие функции
        align.resize(size_of_corpus);
        for (int i = 0; i < size_of_corpus; ++i) {
            align[i].resize(corpus[i].first.size());
        }

        delta = std::vector<std::vector<std::vector<float>>>(size_of_corpus + 1);

        for (int k = 0; k < size_of_corpus; ++k) {
            delta[k].resize(corpus[k].second.size() + 1);
            for (int i = 0; i <= corpus[k].second.size(); ++i) {
                delta[k][i].resize(corpus[k].first.size() + 1);;
            }
            std::vector<float> c_inic;
            c_inic.resize(number_of_process);
            c_inic[0] = 0;
            for (int i = 0 ; i < corpus[k].first.size(); ++i) {
                c.insert(std::pair<std::string, std::vector<float>>("", c_inic));
                c.insert(std::pair<std::string, std::vector<float>> (corpus[k].first[i], c_inic));
                for (int j = 0; j < corpus[k].second.size(); ++j) {
                    c_e_f.insert(std::pair<std::pair<std::string, std::string>, std::vector<float>>
                                 (std::pair<std::string, std::string> (corpus[k].first[i], corpus[k].second[j]), c_inic));
                    c_e_f.insert(std::pair<std::pair<std::string, std::string>, std::vector<float>>
                                 (std::pair<std::string, std::string> ("", corpus[k].second[j]), c_inic));
                    t_map.insert(std::pair<std::pair<std::string, std::string>, float>
                                 (std::pair<std::string, std::string> (corpus[k].first[i], corpus[k].second[j]), 0.5));
                    t_map.insert(std::pair<std::pair<std::string, std::string>, float>
                                 (std::pair<std::string, std::string> ("", corpus[k].second[j]), 0.5));

                }
            }
        }
        std::cout <<"!!!"<<std::endl;
    }

    class delta_computer {
        IBM* parent;
        int start;
        int step;
    public:
        delta_computer(IBM* _parent, int number_of_start_sentense, int _step):parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            for (int k = start; k < parent->size_of_corpus; k = k + step) {
                for (int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    float sum = 0;
                    for (int h = 0; h < parent->corpus[k].first.size(); ++h) {
                        sum += parent->t_map.find(std::pair<std::string, std::string>(parent->corpus[k].first[h], parent->corpus[k].second[i]))->second;
                    }
                    sum += parent->t_map.find(std::pair<std::string, std::string>("", parent->corpus[k].second[i]))->second;
                    for (int h = 0; h < parent->corpus[k].first.size(); ++h) {
                        parent->delta[k][i][h] = parent->t_map.find(std::pair<std::string, std::string>
                               (parent->corpus[k].first[h], parent->corpus[k].second[i]))->second/sum;
                    }
                    parent->delta[k][i][parent->corpus[k].first.size()] =
                            parent->t_map.find(std::pair<std::string, std::string>("", parent->corpus[k].second[i]))->second/sum;
                }
            }
        }
    };


    class computer_of_intermediate_parametrs {
        IBM* parent;
        int start;
        int step;
    public:
        computer_of_intermediate_parametrs(IBM* _parent, int number_of_start_sentense, int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            for (int k = start; k < parent->size_of_corpus; k = k + step) {
                for (int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    for (int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        parent->c.find(parent->corpus[k].first[j])->second[start + 1] += parent->delta[k][i][j];
                        parent->c_e_f.find(std::pair<std::string, std::string>
                                           (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second[start + 1]
                                += parent->delta[k][i][j];
                    }
                    parent->c.find("")->second[start + 1] += parent->delta[k][i][parent->corpus[k].first.size()];
                    parent->c_e_f.find(std::pair<std::string, std::string> ("", parent->corpus[k].second[i]))->second[start+1]
                            += parent->delta[k][i][parent->corpus[k].first.size()];
                }
            }
        }
    };

    class sum_of_intermediate_parametrs {
        IBM* parent;
        int start;
        int step;
    public:
        sum_of_intermediate_parametrs(IBM* _parent, int number_of_start_sentense, int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            for (int k = start; k < parent->size_of_corpus; k = k + step) {
                for (int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    for (int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        for (int h = 1; h <= step; ++h) {
                            parent->c.find(parent->corpus[k].first[j])->second[0] += parent->c.find(parent->corpus[k].first[j])->second[h];
                            parent->c_e_f.find(std::pair<std::string, std::string>
                                           (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second[0]
                                += parent->c_e_f.find(std::pair<std::string, std::string>
                                                      (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second[h];
                        }
                    }
                    for (int h = 1; h <= step; ++h) {
                        parent->c.find("")->second[0] += parent->c.find("")->second[h];
                        parent->c_e_f.find(std::pair<std::string, std::string> ("", parent->corpus[k].second[i]))->second[0]
                                += parent->c_e_f.find(std::pair<std::string, std::string> ("", parent->corpus[k].second[i]))->second[h];

                    }
                }
            }
        }
    };

    class initializer {
        IBM* parent;
        int start;
        int step;
    public:
        initializer(IBM* _parent, int number_of_start_sentense, int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            parent->c.clear();
            parent->c_e_f.clear();
            std::vector<float> c_inic;
            c_inic.resize(parent->number_of_process);
            c_inic[0] = 0;
            for (int k = start; k < parent->size_of_corpus; k = k + step) {
                for (int i = 0 ; i < parent->corpus[k].first.size(); ++i) {
                    parent->c.insert(std::pair<std::string, std::vector<float>>("", c_inic));
                    parent->c.insert(std::pair<std::string, std::vector<float>> (parent->corpus[k].first[i], c_inic));
                    for (int j = 0; j < parent->corpus[k].second.size(); ++j) {
                        parent->c_e_f.insert(std::pair<std::pair<std::string, std::string>, std::vector<float>>
                                     (std::pair<std::string, std::string> (parent->corpus[k].first[i], parent->corpus[k].second[j]), c_inic));
                        parent->c_e_f.insert(std::pair<std::pair<std::string, std::string>, std::vector<float>>
                                     (std::pair<std::string, std::string> ("", parent->corpus[k].second[j]), c_inic));
                    }
                }
            }
        }
    };

    class reassesser {//переделать: итерируемся по t и ищем по ключу t соответствующие значения с()
        IBM* parent;
        int start;
        int step;
    public:
        reassesser(IBM* _parent, int number_of_start_sentense, int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            std::unordered_map<std::pair<std::string, std::string>, float, hash>::iterator itr;
            int k;
            for (k = start; k < parent->size_of_corpus; k = k + step) {
                for (int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    for (int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        parent->t_map.find(std::pair<std::string, std::string>(parent->corpus[k].first[j], parent->corpus[k].second[i]))->second
                                = parent->c_e_f.find(std::pair<std::string, std::string>
                                (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second[0]
                                / parent->c.find(parent->corpus[k].first[j])->second[0];
                    }
                    parent->t_map.find(std::pair<std::string, std::string>("", parent->corpus[k].second[i]))->second
                            = parent->c_e_f.find(std::pair<std::string, std::string>("", parent->corpus[k].second[i]))->second[0]
                            / parent->c.find("")->second[0];
                }
            }
        }

    };


    void recompute_delta() {
        boost::thread_group team;
        for (int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({delta_computer(this,i, number_of_process - 1)}));
        }
        team.join_all();
    }

    void reassesse_parametres() {
        boost::thread_group team;
        for (int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({reassesser(this, i, number_of_process - 1)}));
        }
        team.join_all();
    }

    void initialize_subparameters() {
        boost::thread_group team;
        for (int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({initializer(this,i, number_of_process - 1)}));
        }
        team.join_all();
    }

    void recompute_intermediate_parametrs() {
        boost::thread_group team;
        for (int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({computer_of_intermediate_parametrs(this, i, number_of_process - 1)}));
        }
        team.join_all();
        for (int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({sum_of_intermediate_parametrs(this, i, number_of_process - 1)}));
        }

    }



    void  foo() {
        this->recompute_delta();
        //std::cout<<"1"<<std::endl;
        this->recompute_intermediate_parametrs();
        //std::cout<<"1"<<std::endl;
        this->reassesse_parametres();
        //std::cout<<"1"<<std::endl;
        for (int t = 0; t < 1; ++t) {
            this->initialize_subparameters();
            this->recompute_delta();
            std::cout<<"1"<<std::endl;
            this->recompute_intermediate_parametrs();
          //  std::cout<<"1"<<std::endl;
            this->reassesse_parametres();
          //  std::cout<<"1"<<std::endl;
        }


//        for (int p = 0; p < 5; ++p) {
//            //int o;
//            //std::cin>>o;
//            for (int k = 0; k < size_of_corpus; ++k) {
//                c.find("")->second = 0;
//                for (int i = 0 ; i < corpus[k].first.size(); ++i) {
//                    c.find(corpus[k].first[i])->second = 0;
//                    for (int j = 0; j < corpus[k].second.size(); ++j) {
//                        c_e_f.find(std::pair<std::string, std::string>
//                                      ("", corpus[k].second[j]))->second = 0;
//                        c_e_f.find(std::pair<std::string, std::string> (corpus[k].first[i], corpus[k].second[j]))->second = 0;
//                    }
//                }
//            }
//            std::cout<<"72"<<std::endl;
//            for (int k = 0; k < size_of_corpus; ++k) {
//                for (int i = 0; i < corpus[k].second.size(); ++i) {
//                    float sum = 0;
//                    for (int h = 0; h < corpus[k].first.size(); ++h) {
//                        sum += t_map.find(std::pair<std::string, std::string>(corpus[k].first[h], corpus[k].second[i]))->second;
//                    }
//                    sum += t_map.find(std::pair<std::string, std::string>("", corpus[k].second[i]))->second;


//                    for (int j = 0; j < corpus[k].first.size(); ++j) {
//                        delta[k][i][j] = (double)(t_map.find(std::pair<std::string, std::string>(corpus[k].first[j], corpus[k].second[i]))->second)/(double)sum;
//                        c.find(corpus[k].first[j])->second += delta[k][i][j];
//                    //    std::cout<<t_map.find(std::pair<std::string, std::string>(corpus[k].first[j], corpus[k].second[i]))->second<<
//                      //          " "<<sum<<std::endl;
//                        //std::cout<<c.find(corpus[k].first[j])->second<<std::endl;
//                        c_e_f.find(std::pair<std::string, std::string> (corpus[k].first[j], corpus[k].second[i]))->second += delta[k][i][j];
//             //           std::cout<<c_e_f.find(std::pair<std::string, std::string> (corpus[k].first[j], corpus[k].second[i]))->second <<std::endl;
//                    }
//                    delta[k][i][corpus[k].first.size()] = t_map.find(std::pair<std::string, std::string>("", corpus[k].second[i]))->second/sum;
//                    c.find("")->second += delta[k][i][corpus[k].first.size()];
//                    c_e_f.find(std::pair<std::string, std::string> ("", corpus[k].second[i]))->second += delta[k][i][corpus[k].first.size()];

//                }
//            }
//            std::cout<<"86"<<std::endl;
//            for (int k = 0; k < size_of_corpus; ++k) {
//                for (int i = 0; i < corpus[k].second.size(); ++i) {
//                    for (int j = 0; j < corpus[k].first.size(); ++j) {
//                        t_map.find(std::pair<std::string, std::string>(corpus[k].first[j], corpus[k].second[i]))->second = c_e_f.find(std::pair<std::string, std::string>(corpus[k].first[j], corpus[k].second[i]))->second / c.find(corpus[k].first[j])->second;
//                    }
//                    t_map.find(std::pair<std::string, std::string>("", corpus[k].second[i]))->second = c_e_f.find(std::pair<std::string, std::string>("", corpus[k].second[i]))->second / c.find("")->second;
//                }
//            }
//        }

std::cout<<"97"<<std::endl;
        for (int i = 0; i < corpus[1].second.size(); ++i) {
//            for (int l = 0; l < corpus[1].second.size(); ++l) {
//              std::cout<<corpus[1].second[l]<<" ";
//            }
            float max = 0;
            int a;
            for (int j = 0; j < corpus[1].first.size(); ++j) {
                //std::cout<<t_map.find
                  //           (std::pair<std::string, std::string>(corpus[1].first[j], corpus[1].second[i]))->second<<std::endl;;
                if (t_map.find
                        (std::pair<std::string, std::string>(corpus[1].first[j], corpus[1].second[i]))->second >= max) {
                    a = j;
                    max = t_map.find
                            (std::pair<std::string, std::string>(corpus[1].first[j], corpus[1].second[i]))->second;

                }
            }
            if (t_map.find
                    (std::pair<std::string, std::string>("", corpus[1].second[i]))->second >= max) {
                a = -1;
                max = t_map.find
                        (std::pair<std::string, std::string>("", corpus[1].second[i]))->second;

            }


            std::cout << a<<std::endl;
//        }
    }
    }
};

#endif // EMIBM_2_H
