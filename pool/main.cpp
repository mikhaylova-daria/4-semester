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
                std::pair<std::function <return_type(arg_types...)>, arg_types... > funct = pool_ptr->tasks.extract();
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


    class concurrent_vector {
        std::vector<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > vect;
        std::mutex locker_vect;
    public:
        concurrent_vector() {}
        ~concurrent_vector() {}
        std::pair<std::function <return_type(arg_types...)>, arg_types... > extract() {
            locker_vect.lock();
            std::pair<std::function <return_type(arg_types...)>, arg_types... > answer = vect.back();
            vect.pop_back();
            locker_vect.unlock();
            return answer;
        }
        void push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args) {
            locker_vect.lock();
            vect.push_back(func_with_args);
            locker_vect.unlock();
        }

        bool empty() {
            return vect.empty();
        }
    };

public:
    bool finish;
    std::vector<boost::thread> executant_threads;
    concurrent_vector tasks;
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
        tasks.push(std::make_pair(func, args...));
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

