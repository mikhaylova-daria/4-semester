#include <iostream>
#include <boost/thread.hpp>
#include <boost/any.hpp>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#define number 10000

using namespace std;

int check_array[number];

struct functor1 {
    void operator()(int i) {
        check_array[i] = i;
        //std::cout<<"Hello, world!"<<i<<std::endl;
    }
};


namespace my {

class exception: public std::exception {
private:
    std::string _what;
public:
    exception(const char * _what) throw() {
           this->_what = _what;
    }
    const char* what() const throw(){
        return _what.c_str();
    }
    ~exception() throw(){}
};
}


template <typename return_type, typename ... arg_types>
class thread_pool  {

    class executant_thread {
        int id;
        thread_pool* pool_ptr;
        struct finish_flag {

        };

    public:
        executant_thread(thread_pool* _pool_ptr, int _id): pool_ptr(_pool_ptr), id(_id) {}
        ~executant_thread() {}
        void operator()() {
            while(true) {
                if (!pool_ptr->tasks.empty()) {
                    std::pair<std::function <return_type(arg_types...)>, arg_types... > funct = pool_ptr->tasks.wait_extract();
                    funct.first(funct.second);
                }
            }
        }
    };

    class concurrent_vector {
        std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > vect;
        std::mutex locker_vect;
        std::condition_variable queuecheck;
    public:
        concurrent_vector() {}
        ~concurrent_vector() {}

        std::pair<std::function <return_type(arg_types...)>, arg_types... > extract() {
            std::pair<std::function <return_type(arg_types...)>, arg_types... > answer;
            while (true) {
                locker_vect.lock();
                if (!vect.empty()) {
                    answer = vect.back();
                    vect.pop_back();
                    locker_vect.unlock();
                    break;
                }
                locker_vect.unlock();
            }
            return answer;
        }

        std::pair<std::function <return_type(arg_types...)>, arg_types... > wait_extract() {
            std::pair<std::function <return_type(arg_types...)>, arg_types... > answer;
            while (true) {
                std::unique_lock<std::mutex> locker(locker_vect);
                queuecheck.wait(locker);
                if (!vect.empty()) {
                    answer = vect.back();
                    vect.pop_back();
                    break;
                }
            }
            return answer;
        }

        std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> try_extract() {
            std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> answer;
            answer.second = false;
            if (locker_vect.try_lock()) {
                if (!vect.empty()) {
                    answer.second = true;
                    answer.first = vect.back();
                    vect.pop_back();
                    locker_vect.unlock();
                }
            }
            return answer;
        }

        void push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args) {
            locker_vect.lock();
            vect.push_back(func_with_args);
            locker_vect.unlock();
            queuecheck.notify_one();
        }

        void allow_to_exhaust() {
            while (!vect.empty()) {
                queuecheck.notify_one();
            }
        }

        bool empty() {
            return vect.empty();
        }
    };

public:
    bool finish_flag = false;
    bool start_flag = false;
    std::vector<boost::thread> executant_threads;
    concurrent_vector tasks;
    std::mutex lock_for_queuecheck;
    std::condition_variable queuecheck;
public :
    thread_pool() {}
    ~thread_pool() {
        if (!finish_flag) {
            this->close();
        }
    }

    void start() {
        start_flag = true;
        executant_threads = std::vector<boost::thread>(4);
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i] = boost::thread {executant_thread(this, i)};
        }
    }

    void execute(std::function<return_type(arg_types...)> func, arg_types ... args) {
        if (!start_flag) {
            this->start();
        }
        if (finish_flag) {
            throw (my::exception("This pool was closed!"));
        }
        tasks.push(std::make_pair(func, args...));
    }

    void close() {
        finish_flag = true;
        tasks.allow_to_exhaust();
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i].interrupt();
            std::cout<<"нить завершилась"<<std::endl;
        }
    }
};


int main()
{
    {
    thread_pool<void, int> first_pool;
    for (int i = 0; i < number; ++i) {
        first_pool.execute(functor1(), i);
    }
    first_pool.close();
    try {
        first_pool.execute(functor1(), 0);
    } catch (my::exception exp) {
        std::cout<<exp.what()<<std::endl;
    }
    }
    for (int i = 1; i < number; ++i) {
        if (check_array[i] != i) {
            std::cout<<"fail"<<i<<std::endl;
        }
    }
    return 0;
}

