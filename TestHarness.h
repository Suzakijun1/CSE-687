/////////////////////////////////////////////////////
// TestHarness.h                                   //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
/////////////////////////////////////////////////////

#ifndef TESTHARNESS_H
#define TESTHARNESS_H
#include "CommSocket.h"
#include <windows.h>
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

//Test functions to be run when the test driver sends a test request
bool PassFunction(); //Always passes
bool FailFunction(); //Always fails
bool ThrowFunction(); //Always throws an exception

//Returns date and time to be used for logging purposes as well as the time_date variable of the Message class
string getDateTime();

//Used to organize messages sent through socket communications
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
    string DLL_File;
    string JSONbody;
};

//Helper functions to convert between Message class object and JSON object
nlohmann::json convertMessageToJSON(Message msg); //Message to JSON
Message convertJSONtoMessage(nlohmann::json Jbody); //JSON to Message

//TestHanress class used to initialize socket communications, process test requests, and execute tests
class TestHarness
{
    public:
    //Constructor that initializes logLevel
    TestHarness(int logLevel_);

    //Destructor to call FreeLibary on the DLL instances
    ~TestHarness();

    //Initializes the socket on the server that will be the listening socket to establish communication with clients
    //Initializes all threads that will be executed to maintain communications and run tests
    void createServerSocket(); 

    //Initializes the client sockets and connects them to the listening socket on the server
    //Sends readymessages and runs the requested tests from the server until a "Break" message is received
    //Uses chilID to keep track of the executing thread
    void createClientSocket(int childID,CommSocket& clientSocket);

    //Establish a listening socket for clients to connect
    void startListening();

    //Once a client socket connects to the listening server socket, acceptClients creates a separate
    //communication socket on the server for communications with the client socket that was accepted
    //Receives ready messages from the client and sends test requests from the blocking queue until the blocking queue is empty
    void acceptClients(CommSocket& serverClientSocket);

    //Initialized at the creation of TestHarness object that specifies desired amount of logging
    int logLevel;

    //Primary server socket used for listening for clients trying to connect
    CommSocket ServerSocket;

    //Client sockets created to connect to the listening socket
    CommSocket ClientConnectorSocket1;
    CommSocket ClientConnectorSocket2;
    CommSocket ClientConnectorSocket3;

    //Sockets to be used by the server to communicate with clients once a connection has been accepted
    CommSocket ClientListenerSocket1;
    CommSocket ClientListenerSocket2;
    CommSocket ClientListenerSocket3;

    //Blocking Queue to send test requests from the server to the client
    BlockingQueue<Message> testRequestsQueue;

    //The following variables are used by child threads to load DLL files and store the functions extracted from the DLL file
    HINSTANCE ThrowDLL;
    HINSTANCE PassDLL;
    HINSTANCE FailDLL;
    typedef bool (*funcThrow)();
    typedef bool (*funcPass)();
    typedef bool (*funcFail)();
    funcThrow ThrowFunction;
    funcPass PassFunction;
    funcFail FailFunction;
    
    //Executor function to run the requested callable object in the scope of a try block and log results
    template <typename T>
    void Executor(T& callable, string testName)
    {
        bool result = false;
        {
            if(logLevel == 1)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << "Test started.\n";
            }
            if(logLevel == 2)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test started.\n";
            }
            if(logLevel == 3)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test started at " << getDateTime();
            }
        }
        try
        {
            result = callable();
        }
        catch(const std::exception& e)
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            if(logLevel == 2 || logLevel == 3)
            {
                std::cerr << "Error: " << e.what() << '\n';
            }
        }
        if(result == true)
        {
            //Outputs different completion status based on logLevel
            if(logLevel == 1)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << "Test passed.\n";
            }
            if(logLevel == 2)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test completed.\n";
                cout << "Test passed.\n";
            }
            if(logLevel == 3)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test completed at " << getDateTime();
                cout << "Test passed.\n";
            }
        }
        if(result == false)
        {
            //Outputs different completion status based on logLevel
            if(logLevel == 1)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << "Test failed.\n";
            }
            if(logLevel == 2)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test completed.\n";
                cout << "Test failed.\n";
            }
            if(logLevel == 3)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                cout << testName << " test completed at " << getDateTime();
                cout << "Test failed.\n";
            }
        }
    }
};

//TestDriver class creates messages to be sent from TestHarness on the server to the clients that will execute the tests
class TestDriver
{
    public:
    //Constructor initializes the messages to be sent
    TestDriver();

    //Creates the TestHarness object and enqueues test request messages to the TestHarness blocking queue
    void runTests();

    //Three different test requests for three unique tests to be run
    Message passTestRequest;
    Message failTestRequest;
    Message throwTestRequest;
};

#endif