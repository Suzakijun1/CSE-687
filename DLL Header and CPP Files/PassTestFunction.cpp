#include "PassTestFunction.h"
#include <ctime>
#include <chrono>
#include <thread>


bool PassTestFunction()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}