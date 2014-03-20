#ifndef MUTEX_H
#define MUTEX_H
#include <iostream>
#include <boost/thread.hpp>
#include <queue>
#include <set>
#include <boost/thread/tss.hpp>



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
    static boost::thread_specific_ptr<std::set<int> > lock_priorities;
    const int priority;
    boost::mutex locker;
public:
    group_priority_mutex() = delete;
    group_priority_mutex(const group_priority_mutex& ) = delete;
    group_priority_mutex(int _priority): priority(_priority){
        if (!lock_priorities.get()) {
            lock_priorities.reset(new std::set<int >());
        }
    }
    group_priority_mutex& operator =(const group_priority_mutex&) = delete;
    ~group_priority_mutex(){}

    /*d: приоритет каждого следующего lock должен быть меньше минимума предыдущего*/

    void lock() {
        if (lock_priorities->empty()) {
            lock_priorities->insert(priority);
            locker.lock();
        } else {
            if (*(lock_priorities->begin()) < priority) {
                throw (my::exception("Wrong priority1"));
            } else {
                if (lock_priorities->find(priority) == lock_priorities->end()) {
                    lock_priorities->insert(priority);
                    locker.lock();
                } else {
                    throw (my::exception("Wrong priority2"));
                }
            }
        }
    }

    void unlock() {
        typename std::set<int>::iterator itr = lock_priorities->find(priority);
        if (itr == lock_priorities->end()) {
            throw (my::exception("Wrong priority3"));
        } else {
            lock_priorities->erase(itr);
            locker.unlock();
        }
    }

};

template <int id_group>
boost::thread_specific_ptr<std::set<int> > group_priority_mutex<id_group>::lock_priorities;


#endif // MUTEX_H
