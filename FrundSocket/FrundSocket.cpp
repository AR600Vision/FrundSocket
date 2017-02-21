// FrundSocket.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "frund.h"

int main()
{
	double* xin = new double[1000];
	double* xout = new double[1000];
	double par[] = { 0, 1, 2, 3, 4 };

	gcontrol(1, xin, xout, par);

    return 0;
}

