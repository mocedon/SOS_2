#ifndef ACCOUNTS_H_
#define ACCOUNTS_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>

using namespace std;
typedef pthread_mutex_t MUTEX;
extern MUTEX logMutex;
extern FILE* logtxt;

class Account {

private:
	int ID;
	string PASSWORD;
	int BALANCE;
	MUTEX READMUTEX;
	int READCOUNTER=0;

public:
	MUTEX WRITEMUTEX;

	Account(int id, string password, int balance);
	Account(const Account& account); //copy constructor

	// get functions
	int getId() const;
	string getPassword() const;
	int getReadCount() const;
	void deposit(int ATM, string PassReceived, int sum);
	void withdraw(int ATM, string PassReceived, int sum);
	int getBalance() const;
	void getBalance(int ATM, string PassReceived);
	void Transfer(int srcAccount, int destAccountIdx, Account& destAccount, string PassReceived, int ATM, int sum);

	// helper func
	bool IdenticalPassword(int ATM, string PassReceived);
	int deductCommission(double percentage);
	~Account();
};


#endif
