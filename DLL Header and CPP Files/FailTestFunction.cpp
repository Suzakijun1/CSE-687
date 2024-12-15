#include "FailTestFunction.h"
#include <ctime>
#include <chrono>
#include <thread>


bool FailTestFunction()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return false;
}