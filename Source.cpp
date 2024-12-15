/////////////////////////////////////////////////////
// Source.cpp                                      //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#include "CommSocket.h"
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <functional>
#include <string>
#include <windows.h>
#include "TestHarness.h"
using std::string;
using std::cout;

typedef bool (*funcPass)();
int main()
{
    TestDriver TD;
    TD.runTests();
}