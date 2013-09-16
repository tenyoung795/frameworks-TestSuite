#ifndef TESTSUITE_H
#define TESTSUITE_H

#include <Assert.h>
#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <exception>
#include <stdexcept>
#include <ios>
#include <ostream>
#include <iostream>
#include <atomic>
#include <future>

using namespace std;

template <class Map = unordered_map<string, function<void ()>>>
class TestSuite
{
    public:
    typedef Map Tests;
    
    /*
        The map of tests to run.
    */
    const Tests tests;

    /*
    	Runs all tests.
        If a test throws something that is not an AssertException,
    	the suite aborts.
    
    	Returns true on success, false on failure.
     */
    bool operator()(ostream &out = clog) const
    {
        logBegin(out);
        size_t numFailed = tryTests(out);
        logEnd(numFailed, out);
        return numFailed == 0;
    }

    protected:
    TestSuite(const Tests &tests): tests(mustntBeEmpty(tests)) {}
    TestSuite(Tests &&tests): tests(move(mustntBeEmpty(tests))) {}

    /*
        Tries a particular test,
        logging to the given output stream.
        Returns true on passing, false on failure.
    */
    bool tryTest(const typename Tests::value_type &entry, ostream &out) const
    {
        out << "Executing " << entry.first << '\n';
        try
        {
            entry.second();
            out << entry.first << " passed\n";
            return true;
        }
        catch (AssertException &ae)
        {
            out << entry.first << " failed: " << ae.what() << '\n';
            return false;
        }
    }

    /*
        Logs an intro to the output stream.
    */
    virtual void logBegin(ostream &) const = 0;

    /*
        Tries all the tests.
        Returns the number of tests that failed.
    */
    virtual size_t tryTests(ostream &) const = 0;
    
    /*
        Logs the result to the output stream.
    */
    virtual void logEnd(size_t numFailed, ostream &out) const
    {
        size_t numTests = tests.size();
        out << "Tests passed: " << (numTests - numFailed) << '/' << numTests
			<< (numFailed == 0? "\nA WINNER IS YOU\n" : "\nWOW! YOU LOSE\n");      
    }

    private:
    static Tests mustntBeEmpty(const Tests &tests) throw(invalid_argument)
    {
        if (tests.empty()) throw invalid_argument("No tests");
        return tests;
    }

};

template <class Map = unordered_map<string, function<void ()>>>
class SequentialTestSuite : public TestSuite<Map>
{
    public:
    typedef Map Tests;

    SequentialTestSuite(const Tests &tests): TestSuite<Map>(tests) {}
    SequentialTestSuite(Tests &&tests): TestSuite<Map>(move(tests)) {}

    protected:
    void logBegin(ostream &out) const
    {
        out << "Beginning " << this->tests.size() << " tests sequentially\n";
    }

    size_t tryTests(ostream &out) const
    {
        size_t numFailed = 0;
        for (auto &entry : this->tests)
        {
            if (!tryTest(entry, out)) numFailed++;
        }
        return numFailed;
    }

};

template <class Map = unordered_map<string, function<void ()>>>
class ConcurrentTestSuite: public TestSuite<Map>
{
    public:
    typedef Map Tests;

    ConcurrentTestSuite(const Tests &tests): TestSuite<Map>(tests) {}
    ConcurrentTestSuite(Tests &&tests): TestSuite<Map>(move(tests)) {}

    protected:
    void logBegin(ostream &out) const
    {
        out << "Beginning " << this->tests.size() << " tests concurrently\n";
    }

    size_t tryTests(ostream &out) const
    {
        vector<future<void>> futures;
        atomic<size_t> numFailed(0);
        auto end = this->tests.cend();
        auto iter = this->tests.cbegin();
        for (size_t i = 0; i < this->tests.size() - 1; i++, iter++)
        {
            futures.push_back(async([&, i, iter]()
            {
                if (!this->tryTest(*iter, out)) numFailed++;
            }));
        }
        if (!tryTest(*iter, out)) numFailed++;
        for (auto &f : futures) f.get();
        return numFailed.load();
    }

};


#endif
