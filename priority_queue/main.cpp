#include <boost/thread.hpp>
#include <queue>
#include <set>
#include <boost/thread/tss.hpp>
#include <iostream>
#include <memory>
//#include <condition_variable>


template <typename Key_t, typename Val_t>
class concurrent_priority_queue {
    class Node {
    public:
        Key_t key;
        Val_t value;
        Node(){}
        Node(Key_t _key, Val_t _value): key(_key), value(_value) {}
        explicit Node(std::pair<Key_t, Val_t> inf): key(inf.first), value(inf.second) {}
        ~Node() {}
    };
public:
    std::vector<std::shared_ptr<boost::mutex> > lockers_for_position;
    std::vector<std::shared_ptr<Node> > heap;
    enum state_of_move {UP, DOWN} state = UP;
    boost::mutex locker_for_insert;
    boost::condition_variable cond_var_insert;
    boost::mutex locker_for_extract;
    boost::condition_variable cond_var_extract;
    unsigned int count_of_sifting_up = 0;
    boost::mutex locker_for_count_of_sifting_up;
private:
    //d: предполагается, что при вызове от i элемент на позиции i уже блокирован: метод обеспечивает этот инвариант
    // в процессе работы рекурсии, но при вызове извне нужно прежде залочить позицию, от которой будет вызов;
    // Если элемент покинул какаую-либо позицию
    // После того, как элемент достиг своей позиции, снимается блокировка и та, которая поставлена в данной итерации,
    // и та, которая обеспечивалась инвариантом - т.к. обе позиции теперь корректны

    void sift_up(int i) {
        if (i == 0) {
            lockers_for_position[i / 2]->unlock();
            locker_for_count_of_sifting_up.lock();
            --count_of_sifting_up;
            if (state == DOWN && count_of_sifting_up == 0) {
                cond_var_extract.notify_all();
            }
            locker_for_count_of_sifting_up.unlock();
            return;
        }
        lockers_for_position[i / 2]->lock();
        if (heap[i]->key < heap[i / 2]->key) {
            std::swap(heap[i], heap[i / 2]);
            lockers_for_position[i]->unlock();
            sift_up(i / 2);
        } else {
            lockers_for_position[i / 2]->unlock();
            lockers_for_position[i]->unlock();
            locker_for_count_of_sifting_up.lock();
            --count_of_sifting_up;
            if (state == DOWN && count_of_sifting_up == 0) {
                cond_var_extract.notify_all();
            }
            locker_for_count_of_sifting_up.unlock();
        }
        return;
    }


    //d:сверху-вниз лочиться вся обрабатываемая ветка, снизу вверх при выходе из рекурсии разлочивается
    //с первым локом так ситуации, что и при просеиваннии вверх
    void sift_down(unsigned int i) {
        Key_t left = INFINITY;
        Key_t right = INFINITY;
        if (i == 0) {
            std::cout<<"0000"<<std::endl;
        }
        if (2*i+1 >= lockers_for_position.size()) {
            if (i == 0) {
                std::cout<<"00"<<std::endl;
            }
            lockers_for_position[i]->unlock();
            return;
        }
        std::cout <<"+++"<<i<<std::endl;
        lockers_for_position[2 * i + 1]->lock();
        lockers_for_position[2 * i + 2]->lock();
        if (2 * i + 1 <= heap.size()) {
            left = heap[2 * i + 1]->key;
        }
        if (2 * i + 2 <= heap.size()) {
            right = heap[2 * i + 2]->key;
        }
        if (left == right && right == INFINITY) {
            lockers_for_position[i]->unlock();
            lockers_for_position[2 * i + 1]->unlock();
            lockers_for_position[2 * i + 2]->unlock();
            std::cout <<"***"<<std::endl;
            if (i == 0) {
                std::cout<<"00"<<std::endl;
            }
            return;
        }
        if (right <= left && right < heap[i]->key) {
           swap(heap[2 * i + 2], heap[i]);
           sift_down(2 * i + 2);
        }
        if (left < heap[i]->key) {
           swap(heap[2 * i + 1], heap[i]);
           sift_down(2 * i + 1);
        }
        std::cout <<"---"<<std::endl;
        lockers_for_position[i]->unlock();

    }


public:
    concurrent_priority_queue(){}
    ~concurrent_priority_queue(){}
    void insert(std::pair<Key_t, Val_t> p) {
        locker_for_insert.lock();
        if (state == DOWN) {
            boost::unique_lock<boost::mutex> locker;
            cond_var_insert.wait(locker);
        }
        locker_for_count_of_sifting_up.lock();
        ++count_of_sifting_up;
        locker_for_count_of_sifting_up.unlock();
        std::shared_ptr<Node> new_node(new Node(p));
        heap.push_back(new_node);
      //  std::cout<<heap[0]->key;
        int pos = heap.size() - 1;
        lockers_for_position.push_back(std::shared_ptr<boost::mutex>(new boost::mutex()));
    //    std::cout<<"+"<<std::endl;
        locker_for_insert.unlock();
        lockers_for_position[pos]->lock();
        sift_up(pos);
  //      std::cout<<"*"<<std::endl;
//         std::cout<<heap[0]->key;

    }

    std::pair<Key_t, Val_t> extract_min() {
        locker_for_extract.lock();
        state = DOWN;
        locker_for_count_of_sifting_up.lock();
        if (count_of_sifting_up == 0) {
            std::cout <<"!!!"<<std::endl;
            locker_for_count_of_sifting_up.unlock();
        } else {
            locker_for_count_of_sifting_up.unlock();
            boost::unique_lock<boost::mutex> locker;
            cond_var_extract.wait(locker);
        }
        std::cout <<"№"<<heap[0]->key<<std::endl;
        std::pair<Key_t, Val_t> min(heap[0]->key, heap[0]->value);
        std::cout <<"^^^^"<<std::endl;
        heap[0] = heap.back();
        heap.pop_back();
        std::cout <<"%"<<std::endl;
        lockers_for_position[0]->lock();
        std::cout <<";;;"<<std::endl;
        sift_down(0);
        std::cout <<"йц"<<std::endl;

       /// cond_var_insert.notify_all();
        state = UP;
       /// cond_var_insert.notify_all();
        locker_for_extract.unlock();
        return min;
    }


};

using namespace std;

concurrent_priority_queue <double, int> heap;

struct functor1 {
    void operator()(int i) {
        heap.insert(std::pair<double, int>(i, 123));
    }
};

int main()
{
    boost::thread_group gr;

    for (int i = 123; i > 6; --i) {
        gr.add_thread(new boost::thread(functor1(), i));
    }
    gr.join_all();
        for (int i = 0; i <= 123; ++i){
            cout<<heap.extract_min().first<<std::endl;
        }
    std::cout<<heap.heap[0]->key<<std::endl;
//    for (int i = 0; i <= 123; ++i){
//        cout<<heap.extract_min().first<<std::endl;
//    }

    cout << "Hello World!" << endl;
    return 0;
}

