#ifndef TESTS_H
#define TESTS_H
#include <thread_pool.h>
#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>

const int number  = 100000; // al: static const int - не подойдет? зачем дефайн?
int check_array[number];

struct functor1 {
    void operator()(int i) {
        int j;
        for( int k = 0; k < 100000; k++ ) {//d:если сильно накрутить счётчик, работает медленно, но не виснет :)
            j+= 1;
        }
        check_array[i] = i;
    }
};


BOOST_AUTO_TEST_CASE( my_test )
{
    std::cout<<"!!!"<<std::endl;
    thread_pool<void, int> first_pool;
    for (int i = 0; i < number; ++i) {
        first_pool.execute(functor1(), i);
    }
    first_pool.close();
//        try {
//            first_pool.execute(functor1(), 0);
//        } catch (my::exception exp) {
//            std::cout<<exp.what()<<std::endl;
//        }

       // for (int i = 1; i < number; ++i) {
    //BOOST_CHECK_EQUAL(3, 4);
      //  }

  //BOOST_CHECK( test_object.is_valid() );
}
#endif // TESTS_H
