#include "tcpipwin32.h"
const int SCK_VERSION1 = 0x0101;
const int SCK_VERSION2 = 0x0202;

#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3

#define AF_INET 2

#define IPPROTO_TCP 6

TCP::TCP()
{
    //ctor
}

TCP::~TCP()
{
    if (servSock)
        closesocket(servSock);
    WSACleanup();
    //dtor
}

bool TCP::connectToHost(const int PortNo, const char* IPAddress)
{
    //start up winsock
    WSADATA wsadata;
    SOCKADDR_IN target;

    int error = WSAStartup( SCK_VERSION2, & wsadata);
    if (error)
        return false;

    if (wsadata.wVersion != SCK_VERSION2)
    {
        WSACleanup();
        return false;
    }

    //fill out info neeeded to initialize a socket

    target.sin_family = AF_INET; 			//address internet
    target.sin_port = htons(PortNo); 		//port to conect on (host to network)
    target.sin_addr.s_addr = inet_addr(IPAddress); 	//target ip

    servSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //create the socket
    if (servSock == INVALID_SOCKET)
        return false; 			//couldn't create the socket


    if (connect(servSock, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
        return false; 			//couldnt connect
    else
        return true;

}

int TCP::listenToPort(const int PortNo)
{
    WSADATA wsadata;
    int error = WSAStartup(SCK_VERSION2, &wsadata);

    if (error)
        return false;

    if (wsadata.wVersion != SCK_VERSION2)
    {
        WSACleanup();
        return false;
    }

    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;

    addr.sin_port = htons(PortNo);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    //assign port to this socket

    //accept a connection from any IP using INADDR_ANY
    //or just pass inet_addr("0.0.0.0")
    //specify a specific ip address to watch for only that address

    servSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (servSock == INVALID_SOCKET)
        return false;

    if (bind(servSock, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
        return false; // will happen if you try to bind to same socket multiple times

    //listen to the port with max connections possible
    //function does not return until a connection request is made
    // u_long iMode=1;
    //ioctlsocket(s,FIONBIO,&iMode);

    listen(servSock, SOMAXCONN);
    return true;
}

SOCKET TCP::acceptConnection()
{
    SOCKET TempSock = SOCKET_ERROR;
    TempSock = accept(servSock, NULL, NULL);
    return TempSock;
}

int TCP::sendData(SOCKET writeTo, char * data, const int len)
{
    return send(writeTo, data, len, 0);
}

int TCP::receiveData(SOCKET readFrom, char * buffer, const int len)
{
    return recv(readFrom, buffer, len, 0);
}

//untested!
//Sends a 4 byte packed integer. Sending one byte at a time
//is more reliable than sending an int in one shot.
int TCP::sendFramedData(SOCKET writeTo, char* data, const int len)
{
    char lenBuff[4] = {0};

    lenBuff[0] = (unsigned char) len;
    lenBuff[1] = (unsigned char) (len >> 8);
    lenBuff[2] = (unsigned char) (len >> 16);
    lenBuff[3] = (unsigned char) (len >> 24);

    this->sendData(writeTo, lenBuff, 4);
    return this->sendData(writeTo, data, len);
}


int TCP::receiveFramedData(SOCKET readFrom, char * data)
{
    char lenBuff[4];
    char * dataBuff;
    int lenPrefix;
    int totalRead;
    int currentRead;

    //priming read:
    currentRead = totalRead = this->receiveData(readFrom, lenBuff, 4);

    //read if and while not enough data received, until all data arrives
    while (currentRead > 0 && totalRead < 4)
    {
        currentRead = this -> receiveData(readFrom, (lenBuff + totalRead), (4 - totalRead));
        totalRead += currentRead;
    }

    lenPrefix = *(int *) lenBuff; //dereferenced casted int pointer to char array

    //reset current and total read
    currentRead = 0;
    totalRead = 0;

    //dataBuff = new char [lenPrefix];

    //priming read:
    currentRead = totalRead = this->receiveData(readFrom, data, lenPrefix);

    //read if and while not enough data received, until all data arrives
    while (currentRead > 0 && totalRead < lenPrefix)
    {
        currentRead = this -> receiveData(readFrom, (data + totalRead), (lenPrefix - totalRead));
        totalRead += currentRead;
    }


    //delete [] dataBuff;
    return totalRead;
}
