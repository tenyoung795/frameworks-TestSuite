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

template <class TestCase, class Result, class Map = std::map<TestCase, Result>>
class MultiTest
{
    static std::string message(const TestCase &testCase)
    {
        std::stringstream s;
        s << "Test case " << testCase << " failed";
        return s.str();
    }

    public:
    /*
        The function type that takes in a TestCase and returns a Result.
    */
    typedef std::function<Result (TestCase)> Function;
    /*
        The function to apply per TestCase.
    */
    const Function f;
    /*
        The std::map of TestCases and expected Results.
    */
    const Map m;

    virtual void operator()() const throw(AssertException) = 0;

    protected:
    MultiTest(const Function &f, const Map &m):
        f(f), m(m) {}
    MultiTest(Function &&f, const Map &m):
        f(move(f)), m(m) {}
    MultiTest(const Function &f, Map &&m):
        f(f), m(move(m)) {}
    MultiTest(Function &&f, Map &&m):
        f(move(f)), m(move(m)) {}

    void test(const typename Map::value_type &entry) const
    {
        assertEqualsMsg(Result, entry.second, f(entry.first), message(entry.first));
    }
};

template <class TestCase, class Result, class Map = std::map<TestCase, Result>>
class SequentialMultiTest : public MultiTest<TestCase, Result, Map>
{
    public:
    typedef typename MultiTest<TestCase, Result, Map>::Function Function;
 
    SequentialMultiTest(const Function &f, const Map &m):
        MultiTest<TestCase, Result, Map>(f, m) {}
    SequentialMultiTest(Function &&f, const Map &m):
        MultiTest<TestCase, Result, Map>(move(f), m) {}
    SequentialMultiTest(const Function &f, Map &&m):
        MultiTest<TestCase, Result, Map>(f, move(m)) {}
    SequentialMultiTest(Function &&f, Map &&m):
        MultiTest<TestCase, Result, Map>(move(f), move(m)) {}

    void operator()() const throw(AssertException)
    {
        for (auto &entry : this->m)
        {
            test(entry);
        }
    }
};

template <class TestCase, class Result, class Map = std::map<TestCase, Result>>
class ConcurrentMultiTest : public MultiTest<TestCase, Result, Map>
{
    public:
    typedef typename MultiTest<TestCase, Result, Map>::Function Function;

    ConcurrentMultiTest(const Function &f, const Map &m):
        MultiTest<TestCase, Result, Map>(f, m) {}
    ConcurrentMultiTest(Function &&f, const Map &m):
        MultiTest<TestCase, Result, Map>(move(f), m) {}
    ConcurrentMultiTest(const Function &f, Map &&m):
        MultiTest<TestCase, Result, Map>(f, move(m)) {}
    ConcurrentMultiTest(Function &&f, Map &&m):
        MultiTest<TestCase, Result, Map>(move(f), move(m)) {}

    void operator()() const throw(AssertException)
    {
        if (this->m.empty()) return;
        std::vector<std::future<void>> futures;
        auto end = this->m.cend();
        auto iter = this->m.cbegin();
        for (size_t i = this->m.size(); i > 1; i++, iter++)
        {
            futures.push_back(std::async([&, iter]() { this->test(*iter); })); // capture the iterator's value
        }
        test(*iter);
        for (auto &f : futures)
        {
            f.get();
        }
    }
};


#endif

