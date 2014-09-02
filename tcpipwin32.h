#ifndef TCPIP
#define TCPIP
#include <winsock.h>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
/*
    To use this class, you must link libws2_32 to your project
    The socket that you do data operations on if you're the client and
    have used connectToHost is servSock.

    If you're the server the socket you use for data operations is
    the one returned from acceptConnection().
*/

class TCP
{
	public:
		TCP();
		~TCP();
		bool connectToHost(const int PortNo, const char * IPAddress);
		bool listenToPort(const int PortNo); 
		SOCKET acceptConnection(); //returns socket to use 

		int sendData(SOCKET writeTo, char * data, const int len);
		int recieveData(SOCKET readFrom, char * data, const int len);

        //Sends or recieves data framed with a 4 byte packet specifying the length.
        //in number of bytes
        int sendFramedData(SOCKET writeTo, char * data, const int len);
        int recieveFramedData(SOCKET readFrom, char * data);

		int getSocket() const {return servSock;}
		int getPort() const {return port;}
		int closeSocket(SOCKET sockArg) {return closesocket(sockArg);}

	private:
		SOCKET servSock; //socket handle
		int port;
};

#endif // TCPIP
