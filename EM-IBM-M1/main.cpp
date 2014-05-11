#include <iostream>
#include <corpus_reader.h>
#include <EM-IBM_2.h>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <boost/thread.hpp>
#include <test.h>


int main()
{
    IBM ibm;
    ibm.make_align();
    report();
    return 0;
}

