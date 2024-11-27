/////////////////////////////////////////////////////
// CommSocket.cpp                                  //
// CSE687 Object Oriented Design                   //
// TestHarness Project                             //
//-------------------------------------------------//
//The implementation of CommSocket is based on     //
//Dr. Jim Fawcett's Sockets implementation         //
/////////////////////////////////////////////////////

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
using std::string;
using std::cout;
using std::vector;
std::mutex outputMutex;

//Constructor to initialize Winsock library and member variables
CommSocket::CommSocket()
{
    iResult = WSAStartup(MAKEWORD(2,2),&wsaData);
    if(iResult != 0)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "WSAStartup Error\n";
    }
    ZeroMemory(&hints,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    ListenerPort = 3000;
    clientID = 0;
    SocketListener = INVALID_SOCKET;
    SocketConnector = INVALID_SOCKET;
}

//Destructor closes any open sockets and frees memory allocated during Winsock execution
CommSocket::~CommSocket()
{
    if(SocketListener!=INVALID_SOCKET)
    {
        closesocket(SocketListener);
    }
    if(SocketConnector!=INVALID_SOCKET)
    {
        closesocket(SocketConnector);
    }
    int error = WSACleanup();
}

//Initializes socket that will be used for listening, binds the socket to an address, and starts listening for clients
bool CommSocket::startListening()
{
    string sPort = std::to_string(ListenerPort);
    iResult = getaddrinfo(NULL,sPort.c_str(),&hints,&result);
    if(iResult != 0)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error with getaddrinfo in startListening: " + std::to_string(iResult) + "\n";
    }
    SocketListener = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if(SocketListener == INVALID_SOCKET)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error initializing listener socket\n";
    }
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Server Created SocketListener\n";
    }
    
    iResult = bind(SocketListener,result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error with bind\n";
        SocketListener = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Bind Operation Complete\n";
    }

    iResult = listen(SocketListener,SOMAXCONN);
    if(iResult == SOCKET_ERROR)
    {   
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error with listen\n";
        SocketListener = INVALID_SOCKET;
    }
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Listening Socket setup complete\n";
    }
    return true;
}

//Listening socket accepts clients attempting to connect and returns the communication socket on the server
SOCKET CommSocket::acceptClientConnection()
{
    SOCKET clientSocket = accept(SocketListener,NULL,NULL);
    if(clientSocket == INVALID_SOCKET)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        int error = WSAGetLastError();
        cout << "Error with accepting socket: " << std::to_string(error) << "\n";
    }
    return clientSocket;
}

//Initializes client socket and connects to the listening socket on the server
void CommSocket::startConnection(const string& ip, size_t port)
{
    string sPort = std::to_string(port);
    const char* pTemp = ip.c_str();
    iResult = getaddrinfo(pTemp,sPort.c_str(),&hints,&result);
    if(iResult!=0)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error with getaddrinfo in startConnection\n";
    }

    SocketConnector = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if(SocketConnector == INVALID_SOCKET)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        int error = WSAGetLastError();
        cout << "Error with initializing connector socket: " + std::to_string(error) + "\n";
    }

    iResult = connect(SocketConnector,result->ai_addr,(int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Error with connecting socket\n";
        SocketConnector = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    if(SocketConnector == INVALID_SOCKET)
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        cout << "Unable to connect to listener\n";
    }
}

//Once a communication socket is established, used to send string messages from one end to the other
bool CommSocket::sendString(const string& str)
{
    size_t bytesSent, bytesRemaining = str.size();
    const char* pBuf = str.c_str();
    char terminator = '\0';
    while(bytesRemaining>0)
    {
        bytesSent = send(SocketConnector, pBuf, bytesRemaining, 0);
        if(bytesSent == SOCKET_ERROR || bytesSent == 0)
        {
            return false;
        }
        bytesRemaining -= bytesSent;
        pBuf += bytesSent;
    }
    bytesSent = send(SocketConnector,&terminator,1,0);
    if(bytesSent == SOCKET_ERROR || bytesSent == 0)
    {
        return false;
    }
    return true;
}

//Once a communication socket is established, receive string messages that are sent through the socket
string CommSocket::recvString()
{
    static const int buflen = 600;
    char buffer[buflen];
    string str;
    while(true)
    {
        iResult = recv(SocketConnector,buffer,1,0);
        if(iResult==0 || iResult==SOCKET_ERROR)
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            int error = WSAGetLastError();
            cout << "Error in recvString " << std::to_string(error) << "\n";
            break;
        }
        if(buffer[0]=='\0')
        {
            str += '\0';
            break;
        }
        str += buffer[0];
    }
    return str;
}