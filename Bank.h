#ifndef BANK_H_
#define BANK_H_
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "Account.h"

#define MAXLINESIZE 100
//a struct to hold the ATM data
class ATMInfo
{
public:
	char CMDSfile[MAXLINESIZE];
	int ID;
};

class Bank
{
public:
	MUTEX VecWRITEMUTEX;
	int VecREADCOUNTER=0;
	MUTEX BANKACCOUNTMUTEX;
	int BANKBALANCE=0;
	vector<Account> AccountsVector;
	MUTEX VecREADMUTEX;

	Bank();//constructor
	~Bank(); //destructor
	Account* LocateAccount(int id);
	int getDistFromBeginning(Account*);
	int findbyID(int id);
	void getBalance(int ATM, int ACCOUNTID, string PassReceived);
	void newAccount(int ATM, int ACCOUNTID, string PassReceived, int ReceivedBalance);
	void deposit(int ATM, int ACCOUNTID, string PassReceived, int sum);
	void Transfer(int ATM, int ACCOUNTID, string PassReceived, int receiverAccountID, int sum);
	void withdraw(int ATM, int ACCOUNTID, string PassReceived, int sum);
};

void* CommissionThreadCut(void* arg);
void* threadPnt(void* arg);

#endif