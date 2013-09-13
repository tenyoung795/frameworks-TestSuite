#include "TestSuite.h"
#include <Assert.h>

#include <cstddef>
#include <stdexcept>
#include <string>

using namespace std;

size_t checkConcurrencyLevel(size_t suiteSize, size_t concurrencyLevel) throw(out_of_range)
{
    if (concurrencyLevel == 0)
    {
        return suiteSize == 0? 1 : suiteSize;
    }
    if ((suiteSize == 0 && concurrencyLevel != 1) || concurrencyLevel > suiteSize)
    {
        throw out_of_range("Bad concurrency level " + to_string(concurrencyLevel) 
							+ " for " + to_string(suiteSize) + " tests");
    }
    return concurrencyLevel;
}

