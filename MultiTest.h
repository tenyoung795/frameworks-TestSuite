#ifndef MULTITEST_H
#define MULTITEST_H

#include <Assert.h>
#include "TestSuite.h"
#include <functional>
#include <utility>
#include <string>
#include <map>
#include <ostream>
#include <future>
#include <iostream>

using namespace std;

template <class T, class TestCase, class Result, class Map = map<TestCase, Result>>
class MultiTest
{
    static string message(const TestCase &testCase)
    {
        stringstream s;
        s << "Test case " << testCase << " failed";
        return s.str();
    }

    public:
    /*
        The function type that takes in a T and a TestCase and returns a Result.
    */
    typedef function<Result (T &, const TestCase &)> Function;
    /*
        The function to apply per TestCase.
    */
    const Function f;
    /*
        The map of TestCases and expected Results.
    */
    const Map m;

    virtual void operator()(T &t) const throw(AssertException) = 0;

    protected:
    MultiTest(const Function &f, const Map &m):
        f(f), m(m) {}
    MultiTest(Function &&f, const Map &m):
        f(move(f)), m(m) {}
    MultiTest(const Function &f, Map &&m):
        f(f), m(move(m)) {}
    MultiTest(Function &&f, Map &&m):
        f(move(f)), m(move(m)) {}

    void test(T &t, const typename Map::value_type &entry) const
    {
        assertEqualsMsg(Result, entry.second, f(t, entry.first), message(entry.first));
    }
};

template <class T, class TestCase, class Result, class Map = map<TestCase, Result>>
class SequentialMultiTest : public MultiTest<T, TestCase, Result, Map>
{
    public:
    typedef typename MultiTest<T, TestCase, Result, Map>::Function Function;
 
    SequentialMultiTest(const Function &f, const Map &m):
        MultiTest<T, TestCase, Result, Map>(f, m) {}
    SequentialMultiTest<T, TestCase, Result, Map>(Function &&f, const Map &m):
        MultiTest<T, TestCase, Result, Map>(move(f), m) {}
    SequentialMultiTest<T, TestCase, Result, Map>(const Function &f, Map &&m):
        MultiTest<T, TestCase, Result, Map>(f, move(m)) {}
    SequentialMultiTest<T, TestCase, Result, Map>(Function &&f, Map &&m):
        MultiTest<T, TestCase, Result, Map>(move(f), move(m)) {}

    void operator()(T &t) const throw(AssertException)
    {
        for (auto &entry : this->m)
        {
            test(t, entry);
        }
    }
};

template <class T, class TestCase, class Result, class Map = map<TestCase, Result>>
class ConcurrentMultiTest : public MultiTest<T, TestCase, Result, Map>
{
    public:
    typedef typename MultiTest<T, TestCase, Result, Map>::Function Function;

    ConcurrentMultiTest(const Function &f, const Map &m):
        MultiTest<T, TestCase, Result, Map>(f, m) {}
    ConcurrentMultiTest<T, TestCase, Result, Map>(Function &&f, const Map &m):
        MultiTest<T, TestCase, Result, Map>(move(f), m) {}
    ConcurrentMultiTest<T, TestCase, Result, Map>(const Function &f, Map &&m):
        MultiTest<T, TestCase, Result, Map>(f, move(m)) {}
    ConcurrentMultiTest<T, TestCase, Result, Map>(Function &&f, Map &&m):
        MultiTest<T, TestCase, Result, Map>(move(f), move(m)) {}

    void operator()(T &t) const throw(AssertException)
    {
        vector<future<void>> futures;
        auto end = this->m.cend();
        auto iter = this->m.cbegin();
        for (size_t i = 0; i < this->m.size() - 1; i++, iter++)
        {
            futures.push_back(async([&, iter]() { this->test(t, *iter); })); // capture the iterator's value
        }
        test(t, *iter);
        for (auto &f : futures)
        {
            f.get();
        }
    }
};


#endif

