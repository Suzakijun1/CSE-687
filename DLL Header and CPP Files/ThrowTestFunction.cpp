#include "ThrowTestFunction.h"
#include <stdexcept>
#include <ctime>
#include <chrono>
#include <thread>


bool ThrowTestFunction()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    throw std::runtime_error("ThrowFunction will always throw an exception");
    return false;
}