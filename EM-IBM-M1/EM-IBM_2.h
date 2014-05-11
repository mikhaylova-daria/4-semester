#ifndef EMIBM_2_H
#define EMIBM_2_H
#include <corpus_reader.h>
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>
#include <functional>
#include <fstream>
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
    std::vector<std::vector<std::vector<std::vector<std::vector<float>>>>> c_j_ilm;
    std::vector<std::vector<std::vector<std::vector<float>>>> c_ilm;
    std::vector<std::vector<std::vector<std::vector<float>>>> q;
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> corpus;
    std::vector<std::vector<std::vector<int>>> align;
    corpus_reader reader;
    unsigned int size_of_corpus = 0;
    unsigned int number_of_process = 4;
    unsigned int max_length_e = 0;
    unsigned int max_length_f = 0;
public:
    IBM(const char* input_file = "/home/darya/Алгоритмы/build-EM-IBM-M1-Qt_5_1_1_gcc_64-Debug/alignment-en-fr.txt"): reader(input_file) {
    }

    ~IBM(){}

    void read() {
        std::pair<std::vector<std::string>, std::vector<std::string>> p;
        while (!reader.eof()) {
            p = reader.read_sentenses();
            if (p.first.size() > max_length_e) {
                max_length_e = p.first.size();
            }
            if (p.second.size() > max_length_f) {
                max_length_f = p.second.size();
            }
            reader.read_alignment();
            if (!reader.eof()) {
                ++size_of_corpus;
                corpus.push_back(p);
            }
        }
    }

    class initializer_delta {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        initializer_delta (IBM* _parent,unsigned int number_of_start_sentense, unsigned int _step):parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()() {
            for (unsigned int k = start; k < parent->size_of_corpus; k = k + step) {
                parent->align[k].resize(parent->corpus[k].second.size());
                parent->delta[k].resize(parent->corpus[k].second.size());
                for (unsigned int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    parent->delta[k][i].resize(parent->corpus[k].first.size());
                }
            }
        }
    };


    class initializer_c_t {
        IBM* parent;
        unsigned int number;
    public:
        initializer_c_t (IBM* _parent,unsigned int _number):parent(_parent), number(_number) {
        }
        void operator()() {
            std::vector<float> c_inic;
            c_inic.resize(parent->number_of_process, 0);
            if (number == 1) {
                for (unsigned int k = 0; k < parent->size_of_corpus; ++k) {
                    for (unsigned int i = 0 ; i < parent->corpus[k].first.size(); ++i) {
                        parent->c.insert(std::pair<std::string, std::vector<float>> (parent->corpus[k].first[i], c_inic));
                        for ( unsigned int j = 0; j < parent->corpus[k].second.size(); ++j) {
                            parent->c_e_f.insert(std::pair<std::pair<std::string, std::string>, std::vector<float>>
                                         (std::pair<std::string, std::string> (parent->corpus[k].first[i],
                                                                               parent->corpus[k].second[j]), c_inic));
                        }
                    }
                }
            } else {
                for (unsigned int k = 0; k < parent->size_of_corpus; ++k) {
                    for (unsigned int i = 0 ; i < parent->corpus[k].first.size(); ++i) {
                        for (unsigned int j = 0; j < parent->corpus[k].second.size(); ++j) {
                            parent->t_map.insert(std::pair<std::pair<std::string, std::string>, float>
                                         (std::pair<std::string, std::string> (parent->corpus[k].first[i],
                                                                               parent->corpus[k].second[j]), 0.3));
                        }
                    }
                }
            }
        }
    };


    class delta_computer {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        delta_computer(IBM* _parent,unsigned int number_of_start_sentense, unsigned int _step):parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()() {
            for (unsigned int k = start; k < parent->size_of_corpus; k = k + step) {
                for (unsigned int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    float sum = 0;
                    for (unsigned int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        sum += parent->t_map.find(std::pair<std::string, std::string>
                                                  (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second;
                    }
                    //    std::cout<<sum<<std::endl;

                    for (unsigned int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        parent->delta[k][i][j] = parent->t_map.find(std::pair<std::string, std::string>
                               (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second/sum;
                    }
                }
            }
        }
    };


    class computer_of_intermediate_parametrs_t {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        computer_of_intermediate_parametrs_t(IBM* _parent, unsigned int number_of_start_sentense, unsigned int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            for (unsigned int k = start; k < parent->size_of_corpus; k = k + step) {
                for (unsigned int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    for (unsigned int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        parent->c.find(parent->corpus[k].first[j])->second[start + 1] += parent->delta[k][i][j];
                        parent->c_e_f.find(std::pair<std::string, std::string>
                                           (parent->corpus[k].first[j], parent->corpus[k].second[i]))->second[start + 1]
                                += parent->delta[k][i][j];
                   //     std::cout<<parent->delta[k][i][j]<<std::endl;
                    }
                }
            }
        }
    };

    class sum_of_intermediate_parametrs_t {
        IBM* parent;
        int number;
    public:
        sum_of_intermediate_parametrs_t(IBM* _parent, int _number):
            parent(_parent), number(_number) {
        }
        void operator()(){
            std::unordered_map<std::pair<std::string, std::string>, std::vector<float>, hash>::iterator itr_c_e_f;
            std::unordered_map<std::string, std::vector<float>>::iterator itr_c;
            if (number == 1) {
                for (itr_c = parent->c.begin(); itr_c != parent->c.end(); ++itr_c) {
                    itr_c->second[0] = 0;
                    for (unsigned int i = 1; i < itr_c->second.size(); ++i) {
                         itr_c->second[0] += itr_c->second[i];
                    }
                }
            } else {
                for (itr_c_e_f = parent->c_e_f.begin(); itr_c_e_f != parent->c_e_f.end(); ++itr_c_e_f) {
                    itr_c_e_f->second[0] = 0;
                    for (unsigned int i = 1; i < itr_c_e_f->second.size(); ++i) {
                        itr_c_e_f->second[0] += itr_c_e_f->second[i];
                    }
                }
            }
        }
    };

    class initializer_for_q {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        initializer_for_q(IBM* _parent, unsigned int number_of_start_sentense, unsigned int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {

        }
        void operator()() {
            std::vector<float> c_init;
            for (unsigned int i = 0; i < parent->number_of_process; ++i){
                c_init.push_back(0);
            }
            for (unsigned int j = start; j < parent->max_length_e; j = j + step) {
                parent->c_j_ilm[j].resize(parent->max_length_f);
                parent->q[j].resize(parent->max_length_f);
                for (unsigned int i = 0; i < parent->max_length_f; ++i) {
                    parent->c_j_ilm[j][i].resize(parent->max_length_e);
                    parent->q[j][i].resize(parent->max_length_e);
                    for (unsigned int l = 0; l < parent->max_length_e; ++l) {
                        parent->c_j_ilm[j][i][l].resize(parent->max_length_f);
                        parent->q[j][i][l].resize(parent->max_length_f);
                        for (unsigned int m = 0; m < parent->max_length_f; ++m) {
                            parent->c_j_ilm[j][i][l][m] = c_init;
                        }
                    }
                }
            }
            for (unsigned int i = start; i < parent->max_length_f; i = i + step) {
                parent->c_ilm[i].resize(parent->max_length_e);
                for (unsigned int l = 0; l < parent->max_length_e; ++l) {
                    parent->c_ilm[i][l].resize(parent->max_length_f);
                    for (unsigned int m = 0; m < parent->max_length_f; ++m) {
                       parent->c_ilm[i][l][m] = c_init;
                    }
                }
            }
        }
    };

    class computer_of_intermediate_parametrs_for_q {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        computer_of_intermediate_parametrs_for_q(IBM* _parent, unsigned int number_of_start_sentense, unsigned int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            unsigned int k;
            for (k = start; k < parent->size_of_corpus; k = k + step) {
                for (unsigned int i = 0; i < parent->corpus[k].second.size(); ++i) {
                    for (unsigned int j = 0; j < parent->corpus[k].first.size(); ++j) {
                        parent->c_ilm[i][parent->corpus[k].first.size() - 1][parent->corpus[k].second.size() - 1][start + 1]
                                += parent->delta[k][i][j];
                        parent->c_j_ilm[j][i][parent->corpus[k].first.size() - 1][parent->corpus[k].second.size() - 1][start + 1]
                                += parent->delta[k][i][j];
                    }
                }
            }
        }
    };

    class sum_of_intermediate_parametrs_for_q {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        sum_of_intermediate_parametrs_for_q(IBM* _parent, unsigned int number_of_start_sentense, unsigned int _step):
            parent(_parent), start(number_of_start_sentense), step(_step) {
        }
        void operator()(){
            for (unsigned int i = start; i < parent->c_ilm.size(); i = i + step) {
                for (unsigned int l = 0; l < parent->c_ilm[i].size(); ++l) {
                    for (unsigned int m = 0; m < parent->c_ilm[i][l].size(); ++m) {
                        for (unsigned int h = 1; h <= step; ++h) {
                            parent->c_ilm[i][l][m][0] += parent->c_ilm[i][l][m][h];
                        }
                    }
                }
            }
            for (unsigned int j = start; j < parent->c_j_ilm.size(); j = j + step) {
                for (unsigned int i = 0; i < parent->c_j_ilm[j].size(); ++i) {
                    for (unsigned int l = 0; l < parent->c_j_ilm[j][i].size(); ++l) {
                        for (unsigned int m = 0; m < parent->c_j_ilm[j][i][l].size(); ++m) {
                            for (unsigned int h = 1; h <= step; ++h) {
                                parent->c_j_ilm[j][i][l][m][0]+=parent->c_j_ilm[j][i][l][m][h];
                            }
                            parent->q[j][i][l][m] = parent->c_j_ilm[j][i][l][m][0] / parent->c_ilm[i][l][m][0];
                        }
                    }
                }
            }
        }
    };



    class initializer_for_t {
        IBM* parent;
        int number;
    public:
        initializer_for_t(IBM* _parent, int _number):
            parent(_parent), number(_number) {
        }
        void operator()(){
            std::unordered_map<std::pair<std::string, std::string>, std::vector<float>, hash>::iterator itr_c_e_f;
            std::unordered_map<std::string, std::vector<float>>::iterator itr_c;
            if (number == 1) {
                for (itr_c = parent->c.begin(); itr_c != parent->c.end(); ++itr_c) {
                    for (unsigned int i = 0; i < itr_c->second.size(); ++i) {
                        itr_c->second[i] = 0;
                    }
                }
            } else {
                for (itr_c_e_f = parent->c_e_f.begin(); itr_c_e_f != parent->c_e_f.end(); ++itr_c_e_f) {
                    for (unsigned int i = 0; i < itr_c_e_f->second.size(); ++i) {
                        itr_c_e_f->second[i] = 0;
                    }
                }
            }
        }
    };

    class reassesser {
        IBM* parent;
        unsigned int start;
        unsigned int step;
    public:
        reassesser(IBM* _parent, unsigned int number_of_start_sentense, unsigned int _step):
            parent(_parent), start(number_of_start_sentense), step(_step){
        }
        void operator()(){
            std::unordered_map<std::pair<std::string, std::string>, float, hash>::iterator itr;
            itr = parent->t_map.begin();
            for (unsigned int i = 0; i < start && itr != parent->t_map.end() ; ++i) {
                ++itr;
            }
            for (; itr != parent->t_map.end(); ) {
                itr->second = parent->c_e_f.find(itr->first)->second[0] / parent->c.find(itr->first.first)->second[0];
                for (unsigned int i = 0; i < step && itr != parent->t_map.end(); ++i) {
                    ++itr;
                }
            }
        }
    };


    void initialize() {
        delta.resize(size_of_corpus);
        align.resize(size_of_corpus);
        boost::thread_group team;
        for (unsigned int i = 0; i < 2; ++i) {
            team.add_thread(new boost::thread({initializer_delta(this, i, number_of_process - 2)}));
        };
        team.add_thread(new boost::thread({initializer_c_t(this, 0)}));
        initializer_c_t I(this, 1);
        I.operator ()();
        team.join_all();
    }

    void recompute_delta() {
        boost::thread_group team;
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({delta_computer(this,i, number_of_process - 1)}));
        }
        team.join_all();
    }

    void reassesse_parameters() {
        boost::thread_group team;
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({reassesser(this,i, number_of_process - 1)}));

        }
        team.join_all();
    }

    void initialize_subparameters() {
        boost::thread_group team;
        for (unsigned int i = 1; i < 2; ++i) {
            team.add_thread(new boost::thread({initializer_for_t(this, i)}));
        }
        team.join_all();
    }

    void compute_parameters_q() {
        q.resize(max_length_e);
        c_j_ilm.resize(max_length_e);
        c_ilm.resize(max_length_f);
        boost::thread_group team;
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({initializer_for_q(this, i, number_of_process - 1)}));
        }
        team.join_all();
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({computer_of_intermediate_parametrs_for_q(this, i, number_of_process - 1)}));
        }
        team.join_all();
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({sum_of_intermediate_parametrs_for_q(this, i, number_of_process - 1)}));
        }
        team.join_all();
    }

    void recompute_intermediate_parametrs() {
        boost::thread_group team;
        for (unsigned int i = 0; i < number_of_process - 1; ++i) {
            team.add_thread(new boost::thread({computer_of_intermediate_parametrs_t(this, i, number_of_process - 1)}));
        }
        team.join_all();
        for (unsigned int i = 1; i <= 2; ++i) {
            team.add_thread(new boost::thread({sum_of_intermediate_parametrs_t(this, i)}));
        }
        team.join_all();
    }

    void print_results() {
        std::ofstream f("output.txt");
        std::vector<int> al_for_sent;
        float max = 0;
        float y;
        for (int k = 0; k < corpus.size(); ++k) {
            for (int i = 0; i < corpus[k].second.size(); ++i) {
                al_for_sent.clear();
                max = 0;
                for (int j = 0; j < corpus[k].first.size(); ++j) {
                    y = t_map.find(std::pair<std::string, std::string>(corpus[k].first[j], corpus[k].second[i]))->second
                                            * q[j][i][corpus[k].first.size() - 1][corpus[k].second.size() - 1];
                    if (y > max) {
                        al_for_sent.clear();
                        max = y;
                        al_for_sent.push_back(j);
                    } else {
                        if (y == max) {
                            al_for_sent.push_back(j);
                        }
                    }
                }
                align[k][i] = al_for_sent;
            }
        }
        for (unsigned int k = 0; k < corpus.size(); ++k) {
            for (unsigned int j = 0; j < corpus[k].first.size(); ++j) {
                f<<corpus[k].first[j]<<" ";
            }
            f<<std::endl;
            for (unsigned int i = 0; i < corpus[k].second.size(); ++i) {
                f<<corpus[k].second[i]<<" ";
            }
            f<<std::endl;
            for (unsigned int i = 0; i < corpus[k].second.size() - 1; ++i) {
                for (unsigned int h = 0; h < align[k][i].size(); ++h) {
                    f<<align[k][i][h]<<"-"<<i<<" ";
                }
            }
            f<<std::endl;
        }

    }



    void  make_align() {
        this->read();
        this->initialize();
        this->recompute_delta();
        this->recompute_intermediate_parametrs();
        this->reassesse_parameters();
        for (unsigned int t = 0; t < 4; ++t) {
            this->initialize_subparameters();
            this->recompute_delta();
            this->recompute_intermediate_parametrs();
            this->reassesse_parameters();
        }
        this->compute_parameters_q();
        this->print_results();
        std::cout<<"end"<<std::endl;
    }
};


#endif // EMIBM_2_H
