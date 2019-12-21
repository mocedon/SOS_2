#ifndef BANK_H_
#define BANK_H_

#include <vector>
#include <stdlib.h>
#include <string.h>
#include "Account.h"
#include "atm.h"

class Bank {
	private:
	
	vector<Account> accs_;
	vector<pthread_t*> atms_;
	Account bank_;
	int atm_num_;
	int atm_run;
	int acc_num_;
	bool done;
	pthread_t* cut_thread;
	pthread_t* pnt_thread;
	char* files_[];
	
	public:
	
	Bank(int atm_num, char* atm_files[]);
	~Bank();
	
	int getAtmNum();
	int getAccNum();
	
	void setDone();
	bool getDone();
	
	pthread_t* getAtmThread(int i);
	pthread_t* getCutThread();
	pthread_t* getPntThread();
	
	void startAtm(int i);
	
	void takeCut();
	
	void printStatus();
};

#endif 

	