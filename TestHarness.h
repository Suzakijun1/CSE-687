/////////////////////////////////////////////////////
// TestHarness.h                                   //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#ifndef TESTHARNESS_H
#define TESTHARNESS_H
#include <chrono>
#include <ctime>
#include <iomanip>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
using std::string;
using std::cout;
using std::vector;

//Base class that declares pure virtual function "test" that takes no arguments and returns boolean value indicating test pass or failure
class iTest 
{
    public:
    virtual bool test() = 0; //Pure virtual function to be defined by derived class TestDrivers
    string name; //Keep track of the name of the test for logging purposes
};

//Derived TestDriver class that will always pass
class PassTestDriver : public iTest 
{
    public:
    PassTestDriver() : iTest() { name = "PassTestDriver"; } //Keep track of the name of the test for logging purposes
    virtual bool test(); //PassTestDriver test function: Always passes
};

//Derived TestDriver class that will always fail
class FailTestDriver : public iTest 
{
    public:
    FailTestDriver() : iTest() { name = "FailTestDriver"; } //Keep track of the name of the test for logging purposes
    virtual bool test(); //FailTestDriver test function: Always fails
};

//Derived TestDriver class that will always thrown an exception
class ThrowTestDriver : public iTest 
{
    public:
    ThrowTestDriver() : iTest() { name = "ThrowTestDriver"; } //Keep track of the name of the test for logging purposes
    virtual bool test(); //ThrowTestDriver test function: Always throws an exception
};

//TestHarness class to test callable objects in the scope of a try block and logging information about the test
class TestHarness
{
    public:
    //Executor function that takes a callable object and uses the helper functions to invoke the callable object in the scope of a try block and logs results
    template <typename T>
    void Executor(T& callable , string testName)
    {
        bool testPass = ExceptionHandler(callable, testName);
        TestLogger(testPass, testName);
    }

    //ExceptionHandler function invokes the callable object in the scope of a try block, outputs error messages if an exception is thrown, and returns a pass or fail
    //based on the return value of the callable object
    template <typename T>
    bool ExceptionHandler(T& callable, string testName)
    {
        bool result = false;
        try
        {
            //The following block of code outputs the current date and time down to millisecond for the start of the test
            auto time_now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(time_now);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
            std::tm localTime = *std::localtime(&time);
            cout << testName << " test started at " << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";

            //Attempt to invoke callable object
            result = callable();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << '\n';
        }
        return result;
    }

    void TestLogger(bool testPass, string testName); //TestLogger logs the time date stamp for the test and whether it passed or failed
    void addTest(iTest* iT); //Adds a TestDriver to the testVector to eventually be tested
    void runTests(); //Runs all tests currently in the testVector

    private:
    vector<iTest*> testVector; //Vector used to store all TestDrivers that need be run
};

#endif