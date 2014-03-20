#ifndef TESTS_H
#define TESTS_H
#include <mutex.h>
#define BOOST_TEST_MODULE mutex_order_tests
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE (mutex_order_tests)

std::vector<group_priority_mutex<> *> mutexes;
std::vector<group_priority_mutex<1> *> mutexes_1;


BOOST_AUTO_TEST_CASE( my_test )
{
    for (int i = 0; i < 100; ++i) {
        mutexes.push_back(new group_priority_mutex<>(i));
    }

    try {
        for (int i = 0; i < 100; ++i) {
            mutexes[i]->lock();
        }
        BOOST_FAIL("После блокировки mutex с меньшим приоритетом был заблокирован mutex с бОльшим, хотя ожидалось исключение");
    } catch(my::exception &exc) {
    }
}

BOOST_AUTO_TEST_CASE( my_test_1 )
{
    for (int i = 0; i < 100; ++i) {
        mutexes_1.push_back(new group_priority_mutex<1>(i));
    }

    try {
        for (int i = 99; i >= 0; --i) {
            mutexes_1[i]->lock();
        }
    } catch(my::exception &exc) {
        BOOST_FAIL("Неожиданное исключение");
    }

    try {
        for (int i = 0; i < 50; ++i) {
            mutexes_1[i]->unlock();
            mutexes_1[99-i]->unlock();
        }
    } catch(my::exception &exc) {
        BOOST_FAIL("Неожиданное исключение");
    }
}

BOOST_AUTO_TEST_SUITE_END()
#endif // TESTS_H
