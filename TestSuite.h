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

template <class State,
    class Tests = unordered_map<string, function<void (State &) throw(AssertException)>>>
class TestSuite
{
    static Tests &mustntBeEmpty(const Tests &tests) throw(invalid_argument)
    {
        if (tests.empty()) throw invalid_argument("No tests");
        return tests;
    }

    public:
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
        size_t numFailed = tryTests(makeState(), out);
        logEnd(numFailed, out);
        return numFailed == 0;
    }

    protected:
    TestSuite(const Tests &tests): tests(mustntBeEmpty(tests)) {}
    TestSuite(Tests &&tests): tests(move(mustntBeEmpty(tests))) {}

    /*
        Creates the state to be tested.
        By default, this uses the default initializer;
        if this is not possible for a given type this must be overloaded.
    */
    virtual State makeState() const
    {
        return State{};
    }

    /*
        Tries a particular test,
        logging to the given output stream.
        Returns true on passing, false on failure.
    */
    bool tryTest(State &state, const typename Tests::value_type &entry, ostream &out) const
    {
        out << "Executing " << entry.first << '\n';
        try
        {
            entry.second(state);
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
    virtual size_t tryTests(State &, ostream &) const = 0;
    
    /*
        Logs the result to the output stream.
    */
    virtual void logEnd(size_t numFailed, ostream &out) const
    {
        size_t numTests = tests.size();
        out << "Tests passed: " << (numTests - numFailed) << '/' << numTests
			<< (numFailed == 0? "\nA WINNER IS YOU\n" : "\nWOW! YOU LOSE\n");      
    }

};

template <class State,
    class Tests = unordered_map<string, function<void (State &) throw(AssertException)>>>
class SequentialTestSuite : TestSuite<State, Tests>
{
    public:
    SequentialTestSuite(const Tests &tests): TestSuite<State, Tests>(tests) {}
    SequentialTestSuite(Tests &&tests): TestSuite<State, Tests>(move(tests)) {}

    protected:
    void logBegin(ostream &out) const
    {
        out << "Beginning " << this->tests.size() << " tests sequentially\n";
    }

    size_t tryTests(State &s, ostream &out) const
    {
        size_t numFailed = 0;
        for (auto &entry : this->tests)
        {
            if (!tryTest(s, entry, out)) numFailed++;
        }
        return numFailed;
    }

};

template <class State,
    class Tests = unordered_map<string, function<void (State &) throw(AssertException)>>>
class ConcurrentTestSuite: TestSuite<State, Tests>
{
    public:
    ConcurrentTestSuite(const Tests &tests): TestSuite<State, Tests>(tests) {}
    ConcurrentTestSuite(Tests &&tests): TestSuite<State, Tests>(move(tests)) {}

    protected:
    void logBegin(ostream &out) const
    {
        out << "Beginning " << this->tests.size() << " tests concurrently\n";
    }

    size_t tryTests(State &s, ostream &out) const
    {
        vector<future<void>> futures;
        atomic<size_t> numFailed(0);
        auto end = this->tests.cend();
        auto penultimate = end - 1;
        for (auto iter = this->tests.cbegin(); iter != penultimate; iter++)
        {
            futures.push_back([&, iter]
            {
                if (!tryTest(s, *iter, out)) numFailed++;
            });
        }
        if (!tryTest(s, *penultimate, out)) numFailed++;
        return numFailed.load();
    }

};


#endif
