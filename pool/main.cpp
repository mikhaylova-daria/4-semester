#include <iostream>
#include <boost/thread.hpp>
#include <boost/any.hpp>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <queue>
#include <assert.h>

#define number 10000 // al: static const int - не подойдет? зачем дефайн?

using namespace std;

int check_array[number];

struct functor1 {
    void operator()(int i) {
        int j = 0;
        for( int k = 0; k < 1000000; k++ ) {
            j+= 1;
        }
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
    public:        executant_thread(thread_pool* _pool_ptr, int _id): pool_ptr(_pool_ptr), id(_id)
        {
            assert(_pool_ptr != 0);
        }
        ~executant_thread() {}
        void operator()() {
            while(true) {
                std::cout << "Try get" << std::endl;
                if (pool_ptr->state == ALLOWED) {
                    pool_ptr->state_of_thread[id] = 0;
                    break;
                } else {
                    pool_ptr->state_of_thread[id] = 1;
                }
                typename std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> funct = pool_ptr->tasks.wait_extract();
                if (funct.second) {
                    std:: cout << "Success" << std::endl;
                    funct.first.first(funct.first.second);
                    pool_ptr->state_of_thread[id] = 0;
                } else {
                    std::cout << "Null task" << std::endl;
                }
            }
        }
    };

    // al: можешь пояснить, почему ты делаешь обертку над стандартным вектором, а не очередью, которая по смыслу подходит больше?
    class concurrent_vector {
        std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > vect;
        std::mutex locker_vect;
        std::condition_variable queuecheck;
        thread_pool* pool_ptr;

    public:
        concurrent_vector(){}
        concurrent_vector(thread_pool* _pool_ptr):pool_ptr(_pool_ptr) {}
        ~concurrent_vector() {}
        std::pair<std::function <return_type(arg_types...)>, arg_types... > extract();
        std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> wait_extract();
        std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> try_extract();
        void push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args);
        void allow_to_exhaust();
        bool empty() {
            return vect.empty();
        }

    };
    enum state_t {NON_STARTED = -1, STARTED, FINISHED, ALLOWED, CLOSED} state;
    std::vector<boost::thread> executant_threads;
    std::vector<atomic<bool>> state_of_thread;
    concurrent_vector tasks;

public :
    thread_pool() : tasks(this), state(NON_STARTED), state_of_thread(4), executant_threads(4) {
    }
    ~thread_pool() {
        if (state != CLOSED) {
            this->close();
        }
    }

    void start() {
        if (state == NON_STARTED) {
            state = STARTED;
            for (int i = 0; i < executant_threads.size(); ++i) {
                executant_threads[i] = boost::thread {executant_thread(this, i)};
            }
        } else {
            throw (my::exception("Wrong state: NON_STARTED is expected"));
        }
    }

    void execute(std::function<return_type(arg_types...)> func, arg_types ... args) {
        if (state == NON_STARTED) {
            this->start();
        }
        if (state != STARTED) {
            throw (my::exception("Wrong state: STARTED is expected. May be this pool was closed"));
        }
        tasks.push(std::make_pair(func, args...));
    }

    void close() {
        state = FINISHED;
        tasks.allow_to_exhaust();
        for (int i = 0; i < executant_threads.size(); ++i) {
            // al: хорошо бы иметь еще возможность дождаться выполнения всех задач, а не прервать выполнение потоков.
            // возможность прервать всех - это тоже хорошо, но ей нужно пользоваться, когда мы вежливо уже попросили закончить работу.
            // allow_to_exhaust - возвращает тогда, когда очередь задач пуста, а не когда все задачи выполнены, поэтому мы можем оборвать чью то работу.

            executant_threads[i].join();
            std::cout<<"нить завершилась"<<std::endl;
        }
        state = CLOSED;
    }
};



//template <typename return_type, typename ... arg_types>
//std::pair<std::function <return_type(arg_types...)>, arg_types... >
//                                            thread_pool<return_type, arg_types...>::concurrent_vector::extract() {
//    std::pair<std::function <return_type(arg_types...)>, arg_types... > answer;
//    while (true) {
//        locker_vect.lock();
//        if (!vect.empty()) {
//            answer = vect.back();
//            vect.pop_back();
//            locker_vect.unlock();
//            break;
//        }
//        locker_vect.unlock();
//    }
//    return answer;
//}

template <typename return_type, typename ... arg_types>
std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool>
                                       thread_pool<return_type, arg_types...>::concurrent_vector::wait_extract() {
    std::pair<std::function <return_type(arg_types...)>, arg_types... > answer;
    bool flag = false;
    while (true) {
        std::unique_lock<std::mutex> locker(locker_vect);//, defer_lock);
//        if (vect.empty() && pool_ptr->state == FINISHED) {
//            flag = false;
//            break;
//        }
        queuecheck.wait(locker);//, [this](){return !vect.empty() || pool_ptr->state != ALLOWED;});
        if (!vect.empty()) {
            answer = vect.back();
            vect.pop_back();
            flag = true;
            break;
        } else {
            if (pool_ptr->state == ALLOWED || pool_ptr->state == FINISHED) {
                flag = false;
                break;
                std::cout <<"BREAK"<<std::endl;
            }
        }
    }
    return  std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool >(answer, flag);
}



//template <typename return_type, typename ... arg_types>
//std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool >
//                                       thread_pool<return_type, arg_types...>::concurrent_vector::try_extract() {
//    std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> answer;
//    answer.second = false;
//    if (locker_vect.try_lock()) {
//        if (!vect.empty()) {
//            answer.second = true;
//            answer.first = vect.back();
//            vect.pop_back();
//            locker_vect.unlock();
//        }
//    }
//    return answer;
//}


template <typename return_type, typename ... arg_types>
void thread_pool<return_type, arg_types...>::concurrent_vector::push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args) {
    locker_vect.lock();
    vect.push_back(func_with_args);
    locker_vect.unlock();
    queuecheck.notify_one();
}


template <typename return_type, typename ... arg_types>
void thread_pool<return_type, arg_types...>::concurrent_vector::allow_to_exhaust() {
    while (!vect.empty()) {
        queuecheck.notify_one();
    }
    pool_ptr->state = ALLOWED;
    std::cout<<"end"<<std::endl;
    for (int i = 0; i < pool_ptr->state_of_thread.size(); ++i) {
        std::cout<<"while before"<<std::endl;
        while (pool_ptr->state_of_thread[i]) {
           std::cout<<"while in"<<std::endl;
           queuecheck.notify_all();
        }
        std::cout<<"while after"<<std::endl;
    }
}



int main()
{
    {
    thread_pool<void, int> first_pool;
    for (int i = 0; i < number; ++i) {
        first_pool.execute(functor1(), i);
        std::cout<<i<<std::endl;
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
