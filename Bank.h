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
	vector<Account> AccountsVector;
	int BANKBALANCE;
	MUTEX VecWRITEMUTEX;
	MUTEX VecREADMUTEX;
	int VecREADCOUNTER;
	MUTEX BANKACCOUNTMUTEX;

	Bank();//constructor
	~Bank();
	void addAccount(int ATM, int ACCOUNTID, string PassReceived, int ReceivedBalance);
	void deposit(int ATM, int ACCOUNTID, string PassReceived, int sum);
	void withdraw(int ATM, int ACCOUNTID, string PassReceived, int sum);
	void getBalance(int ATM, int ACCOUNTID, string PassReceived);
	void Transfer(int ATM, int ACCOUNTID, string PassReceived, int receiverAccountID, int sum);


	Account* LocateAccount(int id);
	int getDistFromBeginning(Account*);
	int findbyID(int id);
};

void* CommissionThreadCut(void* arg);
void* threadPnt(void* arg);

#endif