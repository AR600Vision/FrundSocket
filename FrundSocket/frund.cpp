#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")	//Winsock Library

#define KDRIVE 21
#define IN_BUFLEN 300					//KDRIVE*9  //Max length of buffer
#define OUT_BUFLEN 100

//Парааметры подключения к программе управления
#define CONTROL_IP "127.0.0.1"
#define CONTROL_PORT 55556

//Параметры подключения к ROS
#define ROS_IP "192.168.1.104"
#define ROS_PORT 12833

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

//Отправка данных
/*extern "C"*/ void gcontrol(double t, double* xin, double* xout, double* par)
{
	if (isFirst)
	{
		initSockets();
		//Инициаилзируем еще наш сокет
		isFirst = false;
	}

	//Отправка управления
	//control_request(t, xin, xout);

	//Отправка в ROS
	ros_request(t, par);
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
	initSocket(CONTROL_PORT, CONTROL_IP, socket_control, si_control);
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
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
}

//Отправка данных системе управления
void control_request(double t, double* xin, double* xout)
{

	int slen = sizeof(si_control);
	if (sendto(socket_control, (char*)xin, IN_BUFLEN * sizeof(double), 0, (struct sockaddr *) &si_control, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if (recvfrom(socket_control, (char*)xout, OUT_BUFLEN * sizeof(double), 0, (struct sockaddr *) &si_control, &slen) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	return;
}

/* ОТПРАВКА КОМАНД В ROS

-------------------- ФОРМАТ ДАННЫХ --------------------
*  данные пеередаются в виде массива double
*
*  Запрос:
*  |---------|------|------ ... ---|
*  | node id | rviz | data         |
*  |---------|------|------ ... ---|
*
*  node id      номер вызываемой ноды
*  rviz         входные данные беруться из запроса (0) или из RVIZ (1) (или еще откуда-нибудь)
*
*  Ответ:
*  |---------|--------|--------|--------------|------ ... ---|
*  | node id | status | actual | elapsed time | data         |
*  |---------|--------|--------|--------------|------ ... ---|
*
*  node id       Номер ноды, которая отвечает (такой же, как в запросе
*  status        Код ошибки
*  actual        Является ли эти данные ответом на текущий (1) или один из предыдущих (0)
*                запросов. В этой версии ответ в пределах одного запроса невозможен, всегда 0
*  elapsed time  Время, прошедшее с момента запроса. В этой версии всегда 0.
*
*  КОДЫ ОШИБОК:
*   0     Без ошибок
*   1     Неверный формат пакета
*   2     Запрошенная нода не найдена
*   3     Неизвестная внутренняя ошибка в ноде
*/
void ros_request(double t, double * par)
{
	//Определение размера передаваемых и получаемых данных
	int sendDataSize;		//Размер передаваемых данных	
	int recvCount;			//Размер реально полученного массива

	//Параметр sendDataSize зависит от типа вызываемой ноды
	//Что делать с recvCount???

	sendDataSize = 5;

	int slen = sizeof(si_ros);
	if (sendto(socket_ros, (char*)par, sendDataSize * sizeof(double), 0, (struct sockaddr *) &si_ros, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if ((recvCount = recvfrom(socket_ros, (char*)par, OUT_BUFLEN * sizeof(double), 0, (struct sockaddr *) &si_ros, &slen)) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}
