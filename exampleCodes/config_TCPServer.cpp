#include <iostream>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Thread.h>

const Poco::UInt16 SERVER_PORT = 32452;

class Session : public Poco::Net::TCPServerConnection
{
public:
	Session(const Poco::Net::StreamSocket &socket) : TCPServerConnection(socket) {    }
	virtual ~Session() {    }

	virtual void run()
	{
		try
		{
			int recvSize = 0;

			do
			{
				char buffer[256] = { 0, };
				recvSize = socket().receiveBytes(buffer, sizeof(buffer));
				std::cout << "클라이언트에서 받은 메시지: " << buffer << std::endl;


				char szSendMessage[256] = { 0, };
				sprintf_s(szSendMessage, 128 - 1, "Re:%s", buffer);
				int nMsgLen = (int)strnlen_s(szSendMessage, 256 - 1);

				socket().sendBytes(szSendMessage, nMsgLen);
			} while (recvSize > 0);

			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Session: " << exc.displayText() << std::endl;
		}
	}
};

class SessionFactory : public Poco::Net::TCPServerConnectionFactory
{
public:
	SessionFactory() {    }
	virtual ~SessionFactory() {    }

	virtual Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket &socket)
	{
		return new Session(socket);
	}
};

void PrintServerStatus(Poco::Net::TCPServer& server)
{
	printf("maxThreads:%d, maxConcurrentConnections:%d\n",
		server.maxThreads(), server.maxConcurrentConnections());
	printf("currentThreads:%d, currentConnections:%d\n",
		server.currentThreads(), server.currentConnections());
	printf("queuedConnections:%d, totalConnections:%d\n\n",
		server.queuedConnections(), server.totalConnections());
}

int main(int, char**)
{
	Poco::Net::ServerSocket sock(SERVER_PORT);
	sock.listen();

	auto pParams = new Poco::Net::TCPServerParams;
	pParams->setMaxThreads(4);
	pParams->setMaxQueued(4);
	
	Poco::Net::TCPServer server(new SessionFactory(), sock, pParams);

	std::cout << "Simple TCP Server Application." << std::endl;
	PrintServerStatus(server);
	
	server.start();

	while (true)
	{
		Poco::Thread::sleep(2000);

		PrintServerStatus(server);
	}

	server.stop();
	return 0;
}