/////////////////////////////////////////////////////
// Source.cpp                                      //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#include <stdexcept>
#include <iostream>
#include <functional>
#include "TestHarness.h"
#include <string>
using std::string;
using std::cout;

//FunctorClass to be used by the TestHarness class to test a functor callable object
class FunctorClass
{
    public:
    bool operator()()
    { 
        return true; 
    }
};

//Lambda function to be used by TestHarness class to test a lambda callable object
std::function<bool()> makeLambda()
{
    std::function<bool()> func = []()
    {
        return false;
    };
    return func;
}

int main()
{
    TestHarness TH; //Initialize TestHarness object TH
    FunctorClass fc; //Initialize FunctorClass object fc
    TH.Executor(fc,"Functor Class"); //Test passing functor as a callable ojbect
    std::function myFunc = makeLambda(); //Initialize lambda to be tested
    TH.Executor(myFunc, "Lambda"); //Test passing lambda as a callable object

    //Initialize three different TestDriver class objects
    PassTestDriver ptd;
    FailTestDriver ftd;
    ThrowTestDriver ttd;

    //Add previously created TestDrivers to the testVector
    TH.addTest(&ptd);
    TH.addTest(&ftd);
    TH.addTest(&ttd);

    //Run all tests currently in the testVector
    TH.runTests();
}