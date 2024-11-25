/////////////////////////////////////////////////////
// TestHarness.h                                   //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#ifndef TESTHARNESS_H
#define TESTHARNESS_H
#include "CommSocket.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <sstream>
#include "BlockingQueue.h"
#include "json.h"
using std::string;
using std::cout;
using std::vector;
extern std::mutex DQmutex;


bool PassFunction();
bool FailFunction();
bool ThrowFunction();
string getDateTime();

class Message
{
    public:
    Message() {};
    string sourceAddr;
    string destAddr;
    string messageType;
    string author;
    string time_date;
    string sndr;
    string rcvr;
    string testToRun;
    string JSONbody;
};

nlohmann::json convertMessageToJSON(Message msg);
Message convertJSONtoMessage(nlohmann::json Jbody);

class TestHarness
{
    public:
    TestHarness(int logLevel_);

    
    void createServerSocket(); 
    void createClientSocket(int childID,CommSocket& clientSocket);
    void startListening(bool& listening);
    void acceptClients(CommSocket& serverClientSocket);
    void sendMessage(Message msg);
    void processMessage(Message msg);
    int logLevel;
    size_t serverPort;
    CommSocket ServerSocket;
    CommSocket ClientConnectorSocket1;
    CommSocket ClientConnectorSocket2;
    CommSocket ClientConnectorSocket3;
    CommSocket ClientListenerSocket1;
    CommSocket ClientListenerSocket2;
    CommSocket ClientListenerSocket3;
    BlockingQueue<Message> testRequestsQueue;
    template <typename T>
    void Executor(T& callable, string testName)
    {
        bool result = false;
        try
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            cout << testName << " test started at " << getDateTime();
            result = callable();
        }
        catch(const std::exception& e)
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cerr << "Error: " << e.what() << '\n';
        }
        if(result == true)
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            cout << testName << " test completed at " << getDateTime();
            cout << "Test passed.\n";
        }
        if(result == false)
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            cout << testName << " test completed at " << getDateTime();
            cout << "Test failed.\n";
        }
    }
};

class TestDriver
{
    public:
    TestDriver();
    void runTests();
    Message passTestRequest;
    Message failTestRequest;
    Message throwTestRequest;
};

#endif