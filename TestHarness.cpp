/////////////////////////////////////////////////////
// TestHarness.cpp                                 //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#include "TestHarness.h"
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
using std::string;
using std::cout;
using std::vector;

void TestHarness::TestLogger(bool testPass, string testName)
{
    //The following block of code outputs the current date and time down to the millisecond
    auto time_now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(time_now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
    std::tm localTime = *std::localtime(&time);
    cout << testName << " test completed at " << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";
    
    //Base on the result of the test, outputs if the test passed or failed
    if(testPass==true)
    {
        cout << "Test passed\n";
    }
    else
    {
        cout << "Test failed\n";
    }
}

//Adds a TestDriver to the testVector to eventually be tested
void TestHarness::addTest(iTest* iT)
{
    testVector.push_back(iT);
}

//Runs all tests currently in the testVector
void TestHarness::runTests()
{   
    //For all elements of testVector, passes the callable object test as a lambda to the Executor function
    for(int i=0; i<testVector.size(); i++)
    {
        auto* testPtr = testVector[i];
        auto lambda = [testPtr] {return (testPtr->test)(); };
        TestHarness::Executor(lambda,testVector[i]->name);
    }
}

//PassTestDriver test function: Always passes
bool PassTestDriver::test()
{
    //Outputs start time of test to milliseconds
    auto time_now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(time_now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
    std::tm localTime = *std::localtime(&time);
    cout << PassTestDriver::name << " test started at " << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";
    
    return true;
}

//FailTestDriver test function: Always fails
bool FailTestDriver::test()
{
    //Outputs start time of test to milliseconds
    auto time_now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(time_now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
    std::tm localTime = *std::localtime(&time);
    cout << FailTestDriver::name << " test started at " << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";
    
    return false;
}

//ThrowTestDriver test function: Always throws an exception
bool ThrowTestDriver::test()
{
    //Outputs start time of test to milliseconds
    auto time_now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(time_now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
    std::tm localTime = *std::localtime(&time);
    cout << ThrowTestDriver::name << " test started at " << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";

    throw std::runtime_error("ThrowTestDriver will always throw an exception");
    return false;
}