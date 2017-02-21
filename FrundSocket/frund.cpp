#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")	//Winsock Library

#define KDRIVE 21
#define IN_BUFLEN 300					//KDRIVE*9  //Max length of buffer
#define OUT_BUFLEN 100

//���������� ����������� � ��������� ����������
#define CONTROL_IP "127.0.0.1"
#define CONTROL_PORT 55556

//��������� ����������� � ROS
#define ROS_IP "192.168.1.104"
#define ROS_PORT 12833

//int slen;
bool isFirst = true;
WSADATA wsa;

struct sockaddr_in si_control;	//��������� ����������� � ��������� ����������
struct sockaddr_in si_ros;		//��������� ����������� � ROS

SOCKET socket_control;
SOCKET socket_ros;

void initSockets();
void initSocket(int port, const char* ip, SOCKET & sock, struct sockaddr_in& addr);
void control_request(double t, double* xin, double* xout);
void ros_request(double t,  double* par);

//�������� ������
/*extern "C"*/ void gcontrol(double t, double* xin, double* xout, double* par)
{
	if (isFirst)
	{
		initSockets();
		//�������������� ��� ��� �����
		isFirst = false;
	}

	//�������� ����������
	//control_request(t, xin, xout);

	//�������� � ROS
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

//�������� ������ ������� ����������
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

/* �������� ������ � ROS

-------------------- ������ ������ --------------------
*  ������ ����������� � ���� ������� double
*
*  ������:
*  |---------|------|------ ... ---|
*  | node id | rviz | data         |
*  |---------|------|------ ... ---|
*
*  node id      ����� ���������� ����
*  rviz         ������� ������ �������� �� ������� (0) ��� �� RVIZ (1) (��� ��� ������-������)
*
*  �����:
*  |---------|--------|--------|--------------|------ ... ---|
*  | node id | status | actual | elapsed time | data         |
*  |---------|--------|--------|--------------|------ ... ---|
*
*  node id       ����� ����, ������� �������� (����� ��, ��� � �������
*  status        ��� ������
*  actual        �������� �� ��� ������ ������� �� ������� (1) ��� ���� �� ���������� (0)
*                ��������. � ���� ������ ����� � �������� ������ ������� ����������, ������ 0
*  elapsed time  �����, ��������� � ������� �������. � ���� ������ ������ 0.
*
*  ���� ������:
*   0     ��� ������
*   1     �������� ������ ������
*   2     ����������� ���� �� �������
*   3     ����������� ���������� ������ � ����
*/
void ros_request(double t, double * par)
{
	//����������� ������� ������������ � ���������� ������
	int sendDataSize;		//������ ������������ ������	
	int recvCount;			//������ ������� ����������� �������

	//�������� sendDataSize ������� �� ���� ���������� ����
	//��� ������ � recvCount???

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
