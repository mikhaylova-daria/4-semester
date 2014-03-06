#include <boost/thread.hpp>
#include <queue>
#include <set>
#include <boost/thread/tss.hpp>
#include <iostream>
#include <memory>


template <typename Key_t, typename Val_t>
class concurrent_priority_queue {
    class Node {
    public:
        Key_t key;
        Val_t value;
        Node(){}
        Node(Key_t _key, Val_t _value): key(_key), value(_value) {
        }
        explicit Node(std::pair<Key_t, Val_t> inf): key(inf.first), value(inf.second) {
        }
        ~Node() {}
    };

    std::vector<boost::mutex> lockers_for_position;
    std::vector<std::shared_ptr<Node> > heap;

private:
    void sift_up(int i) {
        if (heap[i]->key < heap[i / 2]->key) {
            std::swap(heap[i], heap[i / 2]);
            sift_up(i / 2);
        }
        return;
    }

    void sift_down(int i) {
        Key_t left = INFINITY;
        Key_t right = INFINITY;
        if (2 * i + 1 <= heap.size()) {
            left = heap[2 * i + 1]->key; // левый сын
        }
        if (2 * i + 2 <= heap.size()) {
            right = heap[2 * i + 2]->key; // правый сын
        }
        if (left == right == INFINITY) return;
        if (right <= left && right < heap[i]->key) {
           swap(heap[2 * i + 2], heap[i]);
           sift_down(2 * i + 2);
        }
        if (left < heap[i]->key) {
           swap(heap[2 * i + 1], heap[i]);
           sift_down(2 * i + 1);
        }
    }


public:
    concurrent_priority_queue(){}
    ~concurrent_priority_queue(){}
    void insert(std::pair<Key_t, Val_t> p) {
        std::shared_ptr<Node> new_node(new Node(p));
         heap.push_back(new_node);
         sift_up(heap.size() - 1);
    }

    std::pair<Key_t, Val_t> extract_min() {
        std::pair<Key_t, Val_t> min(heap[0]->key, heap[0]->value);
        heap[0] = heap.back();
        heap.pop_back();
        sift_down(0);
        return min;
    }


};

using namespace std;

int main()
{

    concurrent_priority_queue <double, int> heap;
    for (int i = 123; i > -1; --i){
        heap.insert(std::pair<double, int>((double)i/(123), 2));
    }
    for (int i = 0; i <= 123; ++i){
        cout<<heap.extract_min().first<<std::endl;
    }

    cout << "Hello World!" << endl;
    return 0;
}

