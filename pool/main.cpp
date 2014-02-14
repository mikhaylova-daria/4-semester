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
       // std::cout<<"Hello, world!"<<i<<std::endl;
    }
};


template <typename return_type, typename ... arg_types>
class thread_pool  {

    class executant_thread {
        int id;
        thread_pool* pool_ptr;
    public:
        executant_thread(thread_pool* _pool_ptr, int _id): pool_ptr(_pool_ptr), id(_id) {}
        ~executant_thread() {}
        void listen() {
            if (!pool_ptr->tasks.empty()){
                pool_ptr->lock_for_queuecheck.lock();
                if (pool_ptr->tasks.empty()) {
                    pool_ptr->lock_for_queuecheck.unlock();
                } else {
                    std::pair<std::function <return_type(arg_types...)>, arg_types... > funct;
                    funct = pool_ptr->tasks.back();
                    pool_ptr->tasks.pop_back();
                    funct.first(funct.second);
                    pool_ptr->lock_for_queuecheck.unlock();
                }
            }
        }
    };

    struct start_executent_thread {
        void operator()(thread_pool* pool_ptr, int id) {
            thread_pool::executant_thread executant(pool_ptr, id);
            while (!pool_ptr->finish) {
                executant.listen();
            }
            while (!pool_ptr->tasks.empty()) {
                executant.listen();
            }
        }
    };


public:
    std::atomic<bool> finish;
    std::vector<boost::thread> executant_threads;
    std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > tasks;
    std::mutex lock_for_queuecheck;
public :
    thread_pool() {
        finish = false;
        executant_threads = std::vector<boost::thread>(4);
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i] = boost::thread {start_executent_thread(), this, i};
        }
    }
    ~thread_pool() {
        while (!tasks.empty()) {}
        finish = true;
        for (int i = 0; i < executant_threads.size(); ++i) {
            executant_threads[i].join();
            std::cout<<"нить завершилась"<<std::endl;
        }
    }

    void execute(std::function<return_type(arg_types...)> func, arg_types ... args) {
        lock_for_queuecheck.lock();
        tasks.push_back(std::make_pair(func, args...));
   //     std::cout<<"!!"<<std::endl;
        lock_for_queuecheck.unlock();
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
            std::cout<<"fail";
        }
    }
    return 0;
}

