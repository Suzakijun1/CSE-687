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


bool PassFunction()
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool FailFunction()
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
    return false;
}

bool ThrowFunction()
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
    throw std::runtime_error("ThrowFunction will always throw an exception");
    return false;
}


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

nlohmann::json convertMessageToJSON(Message msg)
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
		{"TestToRun",msg.testToRun}
	};
	return Jbody;
}

Message convertJSONtoMessage(nlohmann::json Jbody)
{
	Message msg;
	msg.sourceAddr = Jbody["SourceAddress"];
	msg.destAddr = Jbody["DestinationAddress"];
	msg.messageType = Jbody["MessageType"];
	msg.author = Jbody["Author"];
	msg.time_date = Jbody["Time_Date"];
	msg.sndr = Jbody["Sender"];
	msg.rcvr = Jbody["Receiver"];
	msg.testToRun = Jbody["TestToRun"];
	msg.JSONbody = Jbody.dump();
	return msg;
}

TestHarness::TestHarness(int logLevel_)
{
    logLevel = logLevel_;
}

void TestHarness::createServerSocket()
{
	bool isListening = false;
	std::thread ServerThread(&TestHarness::startListening,this,std::ref(isListening));
	while(isListening==false) {}
	std::thread serverSocketThread1(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket1));
	std::thread serverSocketThread2(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket2));
	std::thread serverSocketThread3(&TestHarness::acceptClients,this,std::ref(ClientListenerSocket3));
	std::thread clientThread1(&TestHarness::createClientSocket,this,1,std::ref(ClientConnectorSocket1));
	std::thread clientThread2(&TestHarness::createClientSocket,this,2,std::ref(ClientConnectorSocket2));
	std::thread clientThread3(&TestHarness::createClientSocket,this,3,std::ref(ClientConnectorSocket3));
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
	if(ServerThread.joinable())
	{
		ServerThread.join();
	}
}

void TestHarness::createClientSocket(int childID,CommSocket& clientSocket)
{
	clientSocket.clientID = childID;
	clientSocket.startConnection("127.0.0.1",3000);
	{
		std::lock_guard<std::mutex> lock(outputMutex);
		cout << "Established connection for child thread " + std::to_string(childID) + "\n";
	}
	Message childMessage;
	childMessage.sourceAddr = "127.0.0.1";
	childMessage.destAddr = "127.0.0.1";
	childMessage.messageType = "ReadyMessage";
	childMessage.author = "Client";
	childMessage.time_date = getDateTime();
	childMessage.sndr = "Client";
	childMessage.rcvr = "Server";
	childMessage.testToRun = "N/A";
	childMessage.JSONbody = (convertMessageToJSON(childMessage)).dump();

	while(true)
	{
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
		string JSONstring = clientSocket.recvString();
		nlohmann::json JSONmsg = nlohmann::json::parse(JSONstring);
		string TestToRun = JSONmsg["TestToRun"];
		{
			std::lock_guard<std::mutex> lock(outputMutex);
			cout << "Child " << childID << " received TestToRun = " << TestToRun << "\n";
		}
		if(TestToRun == "Pass")
		{
			Executor(PassFunction,TestToRun);
		}
		if(TestToRun == "Fail")
		{
			Executor(FailFunction,TestToRun);
		}
		if(TestToRun == "Throw")
		{
			Executor(ThrowFunction,TestToRun);
		}
		if(TestToRun == "Break")
		{
			break;
		}
	}
	{
		std::lock_guard<std::mutex> lock(outputMutex);
		cout << "Child " << childID << " exiting\n";
	}	
}

void TestHarness::startListening(bool& listening)
{
	listening = ServerSocket.startListening();
}

void TestHarness::acceptClients(CommSocket& serverClientSocket)
{
	serverClientSocket.SocketConnector = ServerSocket.acceptClientConnection();

	while(true)
	{
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
			harnessMessage.testToRun = "Break";
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
			Message harnessMessage;
			{
				std::lock_guard<std::mutex> lock(outputMutex);
				if(testRequestsQueue.size() == 0)
				{
					continue;
				}
				else
				{
					harnessMessage = testRequestsQueue.deQ();
				}
			}
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

TestDriver::TestDriver()
{
    passTestRequest.sourceAddr = "127.0.0.1";
	passTestRequest.destAddr = "127.0.0.1";
	passTestRequest.messageType = "TestRequest";
	passTestRequest.author = "TestDriver";
	passTestRequest.time_date = getDateTime();
	passTestRequest.sndr = "Server";
	passTestRequest.rcvr = "Client";
	passTestRequest.testToRun = "Pass";
	passTestRequest.JSONbody = (convertMessageToJSON(passTestRequest)).dump();

	failTestRequest.sourceAddr = "127.0.0.1";
	failTestRequest.destAddr = "127.0.0.1";
	failTestRequest.messageType = "TestRequest";
	failTestRequest.author = "TestDriver";
	failTestRequest.time_date = getDateTime();
	failTestRequest.sndr = "Server";
	failTestRequest.rcvr = "Client";
	failTestRequest.testToRun = "Fail";
	failTestRequest.JSONbody = (convertMessageToJSON(failTestRequest)).dump();

	throwTestRequest.sourceAddr = "127.0.0.1";
	throwTestRequest.destAddr = "127.0.0.1";
	throwTestRequest.messageType = "TestRequest";
	throwTestRequest.author = "TestDriver";
	throwTestRequest.time_date = getDateTime();
	throwTestRequest.sndr = "Server";
	throwTestRequest.rcvr = "Client";
	throwTestRequest.testToRun = "Throw";
	throwTestRequest.JSONbody = (convertMessageToJSON(throwTestRequest)).dump();
}

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

