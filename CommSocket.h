/////////////////////////////////////////////////////
// CommSocket.h                                    //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
//-------------------------------------------------//
//The implementation of CommSocket is based on     //
//Dr. Jim Fawcett's Sockets implementation         //
/////////////////////////////////////////////////////

#ifndef COMMSOCKET_H
#define COMMSOCKET_H

#include <winsock2.h>
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
#include <ws2tcpip.h>
using std::string;
using std::cout;
using std::vector;
extern std::mutex outputMutex;

#pragma comment(lib, "Ws2_32.lib")

//Used to establish communications between server and client
class CommSocket
{
    public:
    CommSocket();
    ~CommSocket();
    bool startListening();
    SOCKET acceptClientConnection();
    void startConnection(const std::string& ip, size_t port);
    bool sendString(const string& str);
    string recvString();
    WSADATA wsaData;
    int iResult;
    struct addrinfo *result = NULL, hints;
    SOCKET SocketConnector;
    SOCKET SocketListener;
    int clientID;
    size_t ListenerPort;
};   


#endif