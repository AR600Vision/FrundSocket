#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")	//Winsock Library

#define KDRIVE 21
#define BUFLEN 300					//KDRIVE*9  //Max length of buffer

#define SERVER "127.0.0.1"			//ip address of udp server
#define PORT 55556					//The port on which to listen for incoming data
#define ROS_IP "127.0.0.1"
#define ROS_PORT 123

//int slen;
bool isFirst = true;
WSADATA wsa;

struct sockaddr_in si_control;	//параметры подключения к программе управления
struct sockaddr_in si_ros;		//параметры подключения к ROS

SOCKET socket_control;
SOCKET socket_ros;

void initSockets();
void initSocket(int port, const char* ip, SOCKET & sock, struct sockaddr_in& addr);
void control_request(double t, double* xin, double* xout);
void ros_request(double t,  double* par);

//Вот эта функция, которую вызывает горобцов
//Данные будут переданы и возвращены через par
//Еще он как-то будет указывать, что хочет вызвать - пашину или нашу
/*extern "C"*/ void gcontrol(double t, double* xin, double* xout, double* par)
{
	if (isFirst)
	{
		initSockets();
		//Инициаилзируем еще наш сокет
		isFirst = false;
	}

	//Вызываем либо Пашину go, либо свою
	control_request(t, xin, xout);
}

void initSockets()
{
	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	initSocket(PORT, SERVER, socket_control, si_control);
	initSocket(ROS_PORT, ROS_IP, socket_ros, si_ros);
}

void initSocket(int port, const char* ip, SOCKET & sock, struct sockaddr_in & addr)
{
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.S_un.S_addr = inet_addr(SERVER);
}

//Отправка данных системе управления
void control_request(double t, double* xin, double* xout)
{

	int slen = sizeof(si_control);
	if (sendto(socket_control, (char*)xin, BUFLEN * sizeof(double), 0, (struct sockaddr *) &si_control, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if (recvfrom(socket_control, (char*)xout, 100 * sizeof(double), 0, (struct sockaddr *) &si_control, &slen) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	return;
}

//Отправка данных ROS
void ros_request(double t, double * par)
{
	/*
	1. Определять, какую ноду - ???
	2. Размер буфера - ???
	*/

	int inLen = 1;

	int slen = sizeof(si_control);
	if (sendto(socket_control, (char*)par, inLen * sizeof(double), 0, (struct sockaddr *) &si_control, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	int outLen = 1;

	if (recvfrom(socket_control, (char*)par, outLen * sizeof(double), 0, (struct sockaddr *) &si_control, &slen) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}
