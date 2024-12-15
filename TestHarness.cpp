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
#include <sstream>
#include <stdexcept>
#include <string>
#include <winsock2.h>
using std::string;
using std::cout;
using std::vector;
std::mutex DQmutex;

#pragma comment(lib, "Ws2_32.lib")

//Returns date and time to be used for logging purposes as well as the time_date variable of the Message class
string getDateTime()
{
    auto time_now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(time_now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch())%1000;
    std::tm localTime = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&localTime,"%m/%d/%Y %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "\n";
    return oss.str();
}

//Helper functions to convert between Message class object and JSON object
nlohmann::json convertMessageToJSON(Message msg) //Message to JSON
{
	nlohmann::json Jbody;
	Jbody = 
	{
		{"SourceAddress",msg.sourceAddr},
		{"DestinationAddress",msg.destAddr},
		{"MessageType",msg.messageType},
		{"Author",msg.author},
		{"Time_Date",msg.time_date},
		{"Sender",msg.sndr},
		{"Receiver",msg.rcvr},
		{"DLL_File",msg.DLL_File}
	};
	return Jbody;
}

Message convertJSONtoMessage(nlohmann::json Jbody) //JSON to Message
{
	Message msg;
	msg.sourceAddr = Jbody["SourceAddress"];
	msg.destAddr = Jbody["DestinationAddress"];
	msg.messageType = Jbody["MessageType"];
	msg.author = Jbody["Author"];
	msg.time_date = Jbody["Time_Date"];
	msg.sndr = Jbody["Sender"];
	msg.rcvr = Jbody["Receiver"];
	msg.DLL_File = Jbody["DLL_File"];
	msg.JSONbody = Jbody.dump();
	return msg;
}

//Constructor that initializes logLevel
TestHarness::TestHarness(int logLevel_)
{
    logLevel = logLevel_;
	PassDLL = NULL;
	FailDLL = NULL;
	ThrowDLL = NULL;
}

//Destructor to call FreeLibrary on the DLL instances
TestHarness::~TestHarness()
{
	if(ThrowDLL != NULL)
	{
		FreeLibrary(ThrowDLL);
	}
	if(PassDLL != NULL)
	{
		FreeLibrary(PassDLL);
	}
	if(FailDLL != NULL)
	{
		FreeLibrary(FailDLL);
	}
}

//Initializes the socket on the server that will be the listening socket to establish communication with clients
//Initializes all threads that will be executed to maintain communications and run tests
void TestHarness::createServerSocket()
{
	//Initialize listening socket
	startListening();

	//Establishes server threads to receive ready messages and send test requests
	std::thread serverSocketThread1(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket1));
	std::thread serverSocketThread2(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket2));
	std::thread serverSocketThread3(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket3));
	
	//Establishes client threads to receive test requests and send ready messages
	std::thread clientThread1(&TestHarness::createClientSocket,this,1,std::ref(ClientConnectorSocket1));
	std::thread clientThread2(&TestHarness::createClientSocket,this,2,std::ref(ClientConnectorSocket2));
	std::thread clientThread3(&TestHarness::createClientSocket,this,3,std::ref(ClientConnectorSocket3));

	//Joins all threads created once they are completed executing
	if(clientThread1.joinable())
	{
		clientThread1.join();
	}
	if(clientThread2.joinable())
	{
		clientThread2.join();
	}
	if(clientThread3.joinable())
	{
		clientThread3.join();
	}
	if(serverSocketThread1.joinable())
	{
		serverSocketThread1.join();
	}
	if(serverSocketThread2.joinable())
	{
		serverSocketThread2.join();
	}
	if(serverSocketThread3.joinable())
	{
		serverSocketThread3.join();
	}
}

//Initializes the client sockets and connects them to the listening socket on the server
//Sends readymessages and runs the requested tests from the server until a "Break" message is received
//Uses chilID to keep track of the executing thread
void TestHarness::createClientSocket(int childID,CommSocket& clientSocket)
{
	//Establishes connection listening server socket
	clientSocket.startConnection("127.0.0.1",3000);
	{
		std::lock_guard<std::mutex> lock(outputMutex);
		cout << "Established connection for child thread " + std::to_string(childID) + "\n";
	}

	//Creates ready message to be sent when the child thread is ready
	Message childMessage;
	childMessage.sourceAddr = "127.0.0.1";
	childMessage.destAddr = "127.0.0.1";
	childMessage.messageType = "ReadyMessage";
	childMessage.author = "Client";
	childMessage.time_date = getDateTime();
	childMessage.sndr = "Client";
	childMessage.rcvr = "Server";
	childMessage.DLL_File = "N/A";
	childMessage.JSONbody = (convertMessageToJSON(childMessage)).dump();

	//Sends ready messages and receives test request until a "Break" request is sent
	while(true)
	{
		//Sends premade ready message
		bool isSent = clientSocket.sendString(childMessage.JSONbody);
		if(isSent == true)
		{
			std::lock_guard<std::mutex> lock(outputMutex);
			cout << "Child " << childID << " sent ready message\n";
		}
		if(isSent == false)
		{
			std::lock_guard<std::mutex> lock(outputMutex);
			cout << "Child " << childID << " was not able to send ready message\n";
		}

		//Receives test requests
		string JSONstring = clientSocket.recvString();
		nlohmann::json JSONmsg = nlohmann::json::parse(JSONstring);
		string DLL_File = JSONmsg["DLL_File"];
		{
			std::lock_guard<std::mutex> lock(outputMutex);
			cout << "Child " << childID << " received DLL File to Test: \"" << DLL_File << "\"\n";
		}

		//Runs tests based on the test request
		if(DLL_File == "PassTest.dll")
		{
			//Load the DLL file if it has not already been loaded
			if(PassDLL == NULL)
			{
				PassDLL = LoadLibraryExA("PassTest.dll",NULL,0);
				PassFunction = (funcPass)GetProcAddress(PassDLL,"PassTestFunction");
			}

			//Verify no errors with loading function prior to execution
			if(PassFunction != NULL)
			{
				Executor(PassFunction,"Pass");
			}
			else
			{
				std::lock_guard<std::mutex> lock(outputMutex);
				cout << "Error loading PassFunction\n";
			}
		}
		if(DLL_File == "FailTest.dll")
		{
			//Load the DLL file if it has not already been loaded
			if(FailDLL == NULL)
			{
				FailDLL = LoadLibraryExA("FailTest.dll",NULL,0);
				FailFunction = (funcFail)GetProcAddress(FailDLL,"FailTestFunction");
			}

			//Verify no errors with loading function prior to execution
			if(FailFunction != NULL)
			{
				Executor(FailFunction,"Fail");
			}
			else
			{
				std::lock_guard<std::mutex> lock(outputMutex);
				cout << "Error loading FailFunction\n";
			}
		}
		if(DLL_File == "ThrowTest.dll")
		{
			//Load the DLL file if it has not already been loaded
			if(ThrowDLL == NULL)
			{
				ThrowDLL = LoadLibraryExA("ThrowTest.dll",NULL,0);
				ThrowFunction = (funcThrow)GetProcAddress(ThrowDLL,"ThrowTestFunction");
			}

			//Verify no errors with loading function prior to execution
			if(ThrowFunction != NULL)
			{
				Executor(ThrowFunction,"Throw");
			}
			else
			{
				std::lock_guard<std::mutex> lock(outputMutex);
				cout << "Error loading ThrowFunction\n";
			}
		}

		//Exits the loop once the "Break" request is sent
		if(DLL_File == "Break")
		{
			break;
		}
	}
	{
		std::lock_guard<std::mutex> lock(outputMutex);
		cout << "Child " << childID << " exiting\n";
	}	
}

//Establish a listening socket for clients to connect
void TestHarness::startListening()
{
	ServerSocket.startListening();
}

//Once a client socket connects to the listening server socket, acceptClients creates a separate
//communication socket on the server for communications with the client socket that was accepted
//Receives ready messages from the client and sends test requests from the blocking queue until the blocking queue is empty
void TestHarness::acceptClients(CommSocket& serverClientSocket)
{
	//Once the client socket is accepted by the listening socket, the server creates a separate communication socket to communicate with the client
	serverClientSocket.SocketConnector = ServerSocket.acceptClientConnection();

	//Continue to receive ready messages and send test requests until the blocking queue is empty
	while(true)
	{
		//If blocking queue is empty, send a "Break" message to the client and then break out of its own loop
		if(testRequestsQueue.size() == 0)
		{
			Message harnessMessage;
			harnessMessage.sourceAddr = "127.0.0.1";
			harnessMessage.destAddr = "127.0.0.1";
			harnessMessage.messageType = "TestRequest";
			harnessMessage.author = "TestHarness";
			harnessMessage.time_date = getDateTime();
			harnessMessage.sndr = "Server";
			harnessMessage.rcvr = "Client";
			harnessMessage.DLL_File = "Break";
			harnessMessage.JSONbody = (convertMessageToJSON(harnessMessage)).dump();
			bool isSent = serverClientSocket.sendString(harnessMessage.JSONbody);
			if(isSent == false)
			{
				std::lock_guard<std::mutex> lock(outputMutex);
				cout << "Error sending message to client\n";
			}
			break;
		}
		else
		{
			//Double check that the queue is not empty as well as use a mutex while dequeueing the message to prevent
			//other threads from attempting this at the same time
			Message harnessMessage;
			{
				std::lock_guard<std::mutex> lock(DQmutex);
				if(testRequestsQueue.size() == 0)
				{
					continue;
				}
				else
				{
					harnessMessage = testRequestsQueue.deQ();
				}
			}

			//Verify ready message is received and send test request message
			string JSONstring = serverClientSocket.recvString();
			nlohmann::json JSONmsg = nlohmann::json::parse(JSONstring);
			string msgType = JSONmsg["MessageType"];
			if(msgType == "ReadyMessage")
			{
				bool isSent = serverClientSocket.sendString(harnessMessage.JSONbody);
				if(isSent == false)
				{
					std::lock_guard<std::mutex> lock(outputMutex);
					cout << "Error sending message to client\n";
				}
			}
		}
	}
}

//Constructor initializes the messages to be sent
TestDriver::TestDriver()
{
    passTestRequest.sourceAddr = "127.0.0.1";
	passTestRequest.destAddr = "127.0.0.1";
	passTestRequest.messageType = "TestRequest";
	passTestRequest.author = "TestDriver";
	passTestRequest.time_date = getDateTime();
	passTestRequest.sndr = "Server";
	passTestRequest.rcvr = "Client";
	passTestRequest.DLL_File = "PassTest.dll";
	passTestRequest.JSONbody = (convertMessageToJSON(passTestRequest)).dump();

	failTestRequest.sourceAddr = "127.0.0.1";
	failTestRequest.destAddr = "127.0.0.1";
	failTestRequest.messageType = "TestRequest";
	failTestRequest.author = "TestDriver";
	failTestRequest.time_date = getDateTime();
	failTestRequest.sndr = "Server";
	failTestRequest.rcvr = "Client";
	failTestRequest.DLL_File = "FailTest.dll";
	failTestRequest.JSONbody = (convertMessageToJSON(failTestRequest)).dump();

	throwTestRequest.sourceAddr = "127.0.0.1";
	throwTestRequest.destAddr = "127.0.0.1";
	throwTestRequest.messageType = "TestRequest";
	throwTestRequest.author = "TestDriver";
	throwTestRequest.time_date = getDateTime();
	throwTestRequest.sndr = "Server";
	throwTestRequest.rcvr = "Client";
	throwTestRequest.DLL_File = "ThrowTest.dll";
	throwTestRequest.JSONbody = (convertMessageToJSON(throwTestRequest)).dump();
}

//Creates the TestHarness object and enqueues test request messages to the TestHarness blocking queue
void TestDriver::runTests()
{
    TestHarness TH(3);
	TH.testRequestsQueue.enQ(passTestRequest);
	TH.testRequestsQueue.enQ(failTestRequest);
	TH.testRequestsQueue.enQ(throwTestRequest);
	TH.testRequestsQueue.enQ(passTestRequest);
	TH.testRequestsQueue.enQ(failTestRequest);
	TH.testRequestsQueue.enQ(throwTestRequest);
    TH.createServerSocket();
}

