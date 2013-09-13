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
#include <mutex>
#include <future>

using namespace std;

size_t checkConcurrencyLevel(size_t suiteSize, size_t concurrencyLevel) throw (out_of_range);

template <class State>
class TestSuite
{
    public:
	/*
     * A test is a function that takes in a reference to some state and returns nothing,
     * but throws an exception on a known failure.
     */
    typedef function<void (State &) throw(AssertException)> Test;

    /*
     * A map of Tests to their names. 
     */
    typedef unordered_map<string, Test> Tests;
    
    /*
     * The map of tests to run.
     */
    const Tests tests;

    /*
     * The concurrency level of the test suite,
     * meaning the number of futures to use.
     * It must be between 1 and the size of a non-empty suite inclusive,
     * or always 1 for an empty suite.
     */
    const size_t concurrencyLevel;

    /*
     * Runs the test suite.
     * First, the test suite is set up;
     * if that throws an exception, the suite aborts.
     * Otherwise, run all the tests, then tear down the suite.
     * If a test throws something that is not an AssertException,
     * the suite aborts.
     *
     * Returns true on success, false on failure.
     */
    bool operator()(ostream &out = clog) const
    {
        size_t numTests = tests.size();
        out << "Executing " << numTests << " tests at concurrency level " << concurrencyLevel << '\n';

        class state // wrapper around State to use setUp and tearDown
        {
            const TestSuite<State> &suite;
            public:
            State s;
            state(const TestSuite<State> &suite): suite(suite), s(suite.setUp()) {}
            ~state() { suite.tearDown(s); }
        } s(*this);

        
        atomic<size_t> numFailed(0);

        auto tryTest = [&](const pair<const string, Test> &p)
        {
            out << "Executing " << p.first << '\n';
            try
            {
                p.second(s.s);
                out << p.first << " passed\n";
            }
            catch (AssertException &ae)
            {
                out << p.first << " failed: " << ae.what() << '\n';
                numFailed++;
            }
        };
        
        size_t testsPerFuture = numTests / concurrencyLevel;
        vector<future<void>> futures;
        
        // concurrencyLevel - 1 because one of the threads is the caller thread
        auto iter = tests.cbegin();
        for (size_t i = 0; i < concurrencyLevel - 1; i++)
        {
            Tests futureTests;
            for (size_t j = 0; j < testsPerFuture; j++)
            {
                futureTests[iter->first] = iter->second;
                iter++;
            }
            futures.push_back(async([&](Tests myTests)
            {
                for (auto &entry : myTests)
                {
                    tryTest(entry);
                }
            }, futureTests));
        }
        
        for (auto end = tests.cend(); iter != end; iter++)
        {
            tryTest(*iter);
        }

        for (auto &f : futures)
        {
            f.get();
        }

        size_t numPassed = numTests - numFailed;
        bool passed = numFailed == 0;
        out << "Tests passed: " << numPassed << '/' << numTests
			<< (passed? "\nA WINNER IS YOU\n" : "\nWOW! YOU LOSE\n");      
        return passed;
    }

    protected:
    /*
     * Sets up the test suite,
     * returning the state to be tested.
     * If State has no default constructor,
     * this must be overloaded.
     */
    virtual State setUp() const { return State{}; };

    /*
     * Tears down the test suite.
     * Does nothing by default.
     */
    virtual void tearDown(State &s) const {}

    /**
     * Constructs a test suite using the given tests
     * to be ran at a concurrency level; 0 means the maximum, which is default.
     * Throws an out of range error if a bad concurrency level is given.
     *
     * If this is a concurrent test suite and at least ONE test mutates the state,
     * then ALL tests must lock the state appropriately.
     */
    TestSuite(const Tests &t, size_t l = 0) throw(out_of_range):
        tests(t), concurrencyLevel(checkConcurrencyLevel(tests.size(), l)) {}

    TestSuite(Tests &&t, size_t l = 0) throw(out_of_range):
        tests(move(t)), concurrencyLevel(checkConcurrencyLevel(tests.size(), l)) {}

};

#endif
