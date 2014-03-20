#include <iostream>
#include <boost/thread.hpp>
#include <boost/any.hpp>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <assert.h>
#include <tests.h>




//#define number 10000 // al: static const int - не подойдет? зачем дефайн?

//using namespace std;

//int check_array[number];

//struct functor1 {
//    void operator()(int i) {
//        for( int k = 0; k < 100000; k++ ) {//d:если сильно накрутить счётчик, работает медленно, но не виснет :)
//            i+= 1;
//        }
//        check_array[i] = i;
//    }
//};


//namespace my {

//class exception: public std::exception {
//private:
//    std::string _what;
//public:
//    exception(const char * _what) throw() {
//           this->_what = _what;
//    }
//    const char* what() const throw(){
//        return _what.c_str();
//    }
//    ~exception() throw(){}
//};
//}




//template <typename return_type, typename ... arg_types>
//class thread_pool  {

//    /*d: Общая идея: при запуске thread_pool создаётся некоторое количество
//    потоков с executant_thread в качестве исполняемого функтора. Оператор () этого класса в бесконечном цикле
//    берёт задачи из потокобезопасной очереди, которая принадлежит классу thread_pool. Каждый executant_thread
//    знает ссылку на свой thread_pool и умеет отслеживать его состояние: ALLOWED означает, что в очереди больше
//    нет задач и пользователь не собирается их добавлять - условие выхода из цикла, т.е. завершения потока.
//    */
//    class executant_thread {
//        thread_pool& pool_ptr;
//        int id;
//    public:
//        executant_thread(thread_pool& _pool_ptr, int _id): pool_ptr(_pool_ptr), id(_id) {}
//        ~executant_thread() {}
//        void operator()() {
//            while(true) {
//                ///std::cout << "Try get" << std::endl;
//                if (pool_ptr.state == ALLOWED) {
//                    break;
//                }
//                typename std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> funct = pool_ptr.
//                        tasks.wait_extract();
//                if (funct.second) {
//                   /// std:: cout << "Success" << std::endl;
//                    funct.first.first(funct.first.second);
//                }
//            }
//        }
//    };

//    /*d:
//     *потокобезопасная очередь.
//     *Реализация методов после класса thread_pool
//     */
//    class concurrent_vector {
//        std::queue<std::pair<std::function<return_type(arg_types ...) >, arg_types...  > > ordinary_queue;
//        std::mutex locker_vect;
//        std::condition_variable queuecheck;
//        thread_pool& pool_ptr;

//    public:
//        concurrent_vector(){}
//        concurrent_vector(thread_pool& _pool_ptr):pool_ptr(_pool_ptr) {}
//        ~concurrent_vector() {}
//        std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool> wait_extract();
//        void push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args);
//        void allow_to_exhaust();
//        bool empty() {
//            return ordinary_queue.empty();
//        }

//    };

//    concurrent_vector tasks;
//    enum state_t {NON_STARTED = -1, STARTED, ALLOWED, CLOSED} state;
//    /*d: состояния thread_pool:
//     *NON_STARTED - потоки не запущены
//     *STARTED - потоки запущены
//     *ALLOWED - очередь  пуста, но последние задачи в процессе выполнения
//     *CLOSED - очередь пуста, все задачи гарантированно выполнены, thread_pool как ресурс больше не доступен
//     */

//    std::vector<boost::thread> executant_threads;
//public :
//    thread_pool(int size = 4) : tasks((*this)), state(NON_STARTED), executant_threads(size) {
//    }
//    ~thread_pool() {
//        if (state != CLOSED) {
//            this->close();
//        }
//    }

//    void start() {
//        if (state == NON_STARTED) {
//            state = STARTED;
//            for (unsigned int i = 0; i < executant_threads.size(); ++i) {
//                executant_threads[i] = boost::thread {executant_thread(*this, i)};
//            }
//        } else {
//            throw (my::exception("Wrong state: NON_STARTED is expected"));
//        }
//    }

//    void execute(std::function<return_type(arg_types...)> func, arg_types ... args) {
//        if (state == NON_STARTED) {
//            this->start();
//        }
//        if (state != STARTED) {
//            throw (my::exception("Wrong state: STARTED is expected. May be this pool was closed"));
//        }
//        tasks.push(std::make_pair(func, args...));
//    }

//    void close() {
//        tasks.allow_to_exhaust();
//        for (unsigned int i = 0; i < executant_threads.size(); ++i) {
//            executant_threads[i].join();
//            std::cout<<"нить завершилась"<<std::endl;
//        }
//        state = CLOSED;
//    }
//};


//template <typename return_type, typename ... arg_types>
//std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool>
//                                       thread_pool<return_type, arg_types...>::concurrent_vector::wait_extract() {
//    std::pair<std::function <return_type(arg_types...)>, arg_types... > answer;
//    bool flag = false;
//    while (true) {
//        std::unique_lock<std::mutex> locker(locker_vect);
//        queuecheck.wait(locker);
//        if (!ordinary_queue.empty()) {
//            answer = ordinary_queue.front();
//            ordinary_queue.pop();
//            flag = true;
//            break;
//        } else {
//            if (pool_ptr.state == ALLOWED) {
//                flag = false;
//                break;
//            }
//        }
//    }
//    return  std::pair<std::pair<std::function <return_type(arg_types...)>, arg_types... >, bool >(answer, flag);
//}

///*d: добавление задачи в очередь
// *по мере добавления делаем notify_one, сигнализируя простаивающим потокам о том, что очередь пополнена
//*/
//template <typename return_type, typename ... arg_types>
//void thread_pool<return_type, arg_types...>::concurrent_vector::push(std::pair<std::function<return_type(arg_types ...) >, arg_types...  > func_with_args) {
//    locker_vect.lock();
//    ordinary_queue.push(func_with_args);
//    locker_vect.unlock();
//    queuecheck.notify_one();
//}

//template <typename return_type, typename ... arg_types>
//void thread_pool<return_type, arg_types...>::concurrent_vector::allow_to_exhaust() {
//    while (!ordinary_queue.empty()) {
//        queuecheck.notify_one();
//    }
//    pool_ptr.state = ALLOWED;
//    queuecheck.notify_all();
//}



//int main()
//{
//    {
//    thread_pool<void, int> first_pool;
//    for (int i = 0; i < number; ++i) {
//        first_pool.execute(functor1(), i);
//        //std::cout<<i<<std::endl;
//    }
//    first_pool.close();
//    try {
//        first_pool.execute(functor1(), 0);
//    } catch (my::exception exp) {
//        std::cout<<exp.what()<<std::endl;
//    }
//    }
//    for (int i = 1; i < number; ++i) {
//        if (check_array[i] != i) {
//            std::cout<<"fail"<<i<<std::endl;
//        }
//    }
//    return 0;
//}
//*/
