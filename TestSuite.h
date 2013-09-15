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

template <class State, class Map = unordered_map<string, function<void (State &)>>>
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
        State state = makeState();
        size_t numFailed = tryTests(state, out);
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

    private:
    static Tests mustntBeEmpty(const Tests &tests) throw(invalid_argument)
    {
        if (tests.empty()) throw invalid_argument("No tests");
        return tests;
    }

};

template <class State>
class SequentialTestSuite : public TestSuite<State>
{
    public:
    typedef typename TestSuite<State>::Tests Tests;

    SequentialTestSuite(const Tests &tests): TestSuite<State>(tests) {}
    SequentialTestSuite(Tests &&tests): TestSuite<State>(move(tests)) {}

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

template <class State>
class ConcurrentTestSuite: public TestSuite<State>
{
    public:
    typedef typename TestSuite<State>::Tests Tests;

    ConcurrentTestSuite(const Tests &tests): TestSuite<State>(tests) {}
    ConcurrentTestSuite(Tests &&tests): TestSuite<State>(move(tests)) {}

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
        auto iter = this->tests.cbegin();
        for (size_t i = 0; i < this->tests.size() - 1; i++, iter++)
        {
            futures.push_back(async([&, i, iter]()
            {
                if (!this->tryTest(s, *iter, out)) numFailed++;
            }));
        }
        if (!tryTest(s, *iter, out)) numFailed++;
        for (auto &f : futures) f.get();
        return numFailed.load();
    }

};


#endif
