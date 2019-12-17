#ifndef ACCOUNT_H_
#define ACCOUNT_H_

#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <char*.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <cmath>
#include <vector>

typedef pthread_mutex_t MUTEX;
extern MUTEX lock_log; //should be first intiated as global in Bank.cpp
extern ofstream logtxt; //should be first initiated as global in Bank.cpp
using namespace std;

class Account {
private:
// mutex:
	MUTEX READLOCK;
	int READCOUNTER=0; // C++11 new feature, initializing here is possible
// properties of Account:
	int BALANCE;
	int ID;
	char* PASSWORD;

public:
	MUTEX WRITELOCK; //public WriteLock.

	Account(const Account& account); //copy constructor
	Account(int id, char* password, int balance);
	
	int getBalance() const; //getter of balance.
	void getBalance(int ATM, char* PassReceived); //overloading the first function, this one requires password and ATM id.
	int getReadCount() const; //getter of Readers' counter.
	int getId() const; //getter of ID
	char* getPassword() const; //getter of Password.

	int IdenticalPassword(char* PassReceived,int ATM); //checks if the password of account is same as Password received and returns 0 or 1 accordingly. otherwise adds to logtxt error with ATM id.
	int deductComission(double percentage); //locks Writelock and deducts a commision(according to percentage) from the account's 'BALANCE' property.
	//account actions:
	void withdraw(int ATM, char* PassReceived, int sum);
	void deposit(int ATM, char* PassReceived, int sum);
	void Transfer(Account& DestAccount,int SourceAccountIndex, int DestAccountIndex, char* PassReceived, int ATM, int sum);//transfers money.
};


#endif
