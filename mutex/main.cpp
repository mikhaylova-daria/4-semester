#include <iostream>
#include <boost/thread.hpp>
#include <queue>
#include <set>



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


template < int id_group = 0>
class group_priority_mutex {
    static std::set<int> lock_priorities;
    static std::set<int> all_priorities;
    static boost::mutex locker_for_lock_priorities;
    static boost::mutex locker_for_all_priorities;
    const int priority;
    boost::mutex locker;
public:
    group_priority_mutex() = delete;
    group_priority_mutex(const group_priority_mutex& ) = delete;
    group_priority_mutex(int _priority): priority(_priority){
        boost::unique_lock<boost::mutex> loker_all_set(locker_for_all_priorities);
        if (all_priorities.empty()) {
            all_priorities.insert(priority);
        } else {
            if (all_priorities.find(priority) != all_priorities.end()) {
                throw (my::exception("Wrong priority: mutex with this priority already exist"));
            } else {
                all_priorities.insert(priority);
            }
        }
    }
    group_priority_mutex& operator()(const group_priority_mutex&) = delete;
    ~group_priority_mutex(){}

    /*d: приоритет каждого следующего lock должен быть меньше минимума предыдущего*/

    void lock() {
        std::cout<<(*lock_priorities.begin())<<" "<<priority<<std::endl;
        boost::unique_lock<boost::mutex> locker_lock_set(locker_for_lock_priorities);
        if (lock_priorities.empty()) {
            lock_priorities.insert(priority);
            locker_lock_set.unlock();
            locker.lock();
        } else {
            if ((*lock_priorities.begin()) < priority) {
                throw (my::exception("Wrong priority"));
            } else {
                lock_priorities.insert(priority);
                locker_lock_set.unlock();
                locker.lock();
            }
        }
    }

    void unlock() {
        boost::unique_lock<boost::mutex> locker_lock_set(locker_for_lock_priorities);
        typename std::set<int>::iterator itr = lock_priorities.find(priority);
        if (itr == lock_priorities.end()) {
            throw (my::exception("Wrong priority"));
        } else {
            lock_priorities.erase(itr);
            locker.unlock();
        }
    }

};

template <int id_group>
std::set<int> group_priority_mutex<id_group>::lock_priorities;


template <int id_group>
std::set<int> group_priority_mutex<id_group>::all_priorities;

template <int id_group>
boost::mutex group_priority_mutex<id_group>::locker_for_lock_priorities;

template <int id_group>
boost::mutex group_priority_mutex<id_group>::locker_for_all_priorities;


int main() {
    group_priority_mutex<> first(1);
    group_priority_mutex<> second(0);
    first.lock();
    second.lock();



    return 0;
}
