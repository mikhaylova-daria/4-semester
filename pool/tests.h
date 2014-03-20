#ifndef TESTS_H
#define TESTS_H
#include <thread_pool.h>
#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>

const int number  = 1000; // al: static const int - не подойдет? зачем дефайн?
int check_array[number];

struct functor1 {
    void operator()(int i, int j) {
        for( int k = 0; k < 100000; k++ ) {//d:если сильно накрутить счётчик, работает медленно, но не виснет :)
            j+= 1;
        }
        check_array[i] = i;
    }
};


BOOST_AUTO_TEST_CASE( my_test )
{
    thread_pool first_pool;

    for (int i = 0; i < number; ++i) {
        first_pool.execute<void, int>({functor1()}, i, 0);
    }

    first_pool.close();

    try {
        first_pool.execute<void, int, int>({functor1()}, 0, 1);
        BOOST_FAIL("Ожидалось исключение из-за попытки работать с закрытым ресурсом");
    } catch (my::exception exp) {
    }

    for (int i = 1; i < number; ++i) {
        BOOST_CHECK_EQUAL(check_array[i], i);
    }

}
#endif // TESTS_H
