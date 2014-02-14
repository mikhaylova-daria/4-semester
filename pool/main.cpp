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
        void listen() {
            std::unique_lock<std::mutex> locker(pool_ptr->lock_for_queuecheck);
            pool_ptr->queuecheck.wait(locker);
            if (!pool_ptr->tasks.empty()) {
                std::pair<std::function <return_type(arg_types...)>, arg_types... > funct = pool_ptr->tasks.back();
                pool_ptr->tasks.pop_back();
                locker.unlock();
                funct.first(funct.second);
            }

        }
    };

    struct start_executent_thread {
        void operator()(thread_pool* pool_ptr, int id) {
            thread_pool::executant_thread executant(pool_ptr, id);
            while (true) {
                executant.listen();
            }
        }
    };


//    class concurrent_vector {
//        std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > vect;
//    public:
//        concurrent_vector() {}
//        ~concurrent_vector() {}
//        std::pair<std::function <return_type(arg_types...)>, arg_types... > top() {
//            std::pair<std::function <return_type(arg_types...)>, arg_types... > answer = vect.front()
//            return
//        }

//    };

public:
    bool finish;
    std::vector<boost::thread> executant_threads;
    std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > tasks;
    std::mutex lock_for_queuecheck;
    std::condition_variable queuecheck;
public :
    thread_pool() {
        finish = false;
        executant_threads = std::vector<boost::thread>(4);
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i] = boost::thread {start_executent_thread(), this, i};
        }
    }
    ~thread_pool() {
        while (!tasks.empty()) {
            queuecheck.notify_one();
        }
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i].interrupt();
            std::cout<<"нить завершилась"<<std::endl;
        }

    }

    void execute(std::function<return_type(arg_types...)> func, arg_types ... args) {
        lock_for_queuecheck.lock();
        tasks.push_back(std::make_pair(func, args...));
        lock_for_queuecheck.unlock();
        queuecheck.notify_one();
    }
};


int main()
{
    {
    thread_pool<void, int> first_pool;
    for (int i = 0; i < number; ++i) {
        first_pool.execute(functor1(), i);
    }
    }
    for (int i = 1; i < number; ++i) {
        if (check_array[i] != i) {
            std::cout<<"fail"<<i<<std::endl;
        }
    }
    return 0;
}

