#ifndef BANK_H_
#define BANK_H_

#include <vector>
#include "Account.h"
#include "atm.h"

class bank {
	private:
	
	vector<Account> accounts_;
	vector<atm> atms_;
	Account bank_;
	int atm_num_;
	int atm_run;
	int acc_num_;
	bool done;
	pthread_t* cut_thread;
	pthread_t* pnt_thread;
	
	public:
	
	bank(int atm_num, char* atm_files[]);
	
	int getAtmNum();
	int getAccNum();
	
	void setDone();
	bool getDone();
	
	pthread_t* getAtmThread(int i);
	pthread_t* getCutThread();
	pthread_t* getPntThread();
	
	void* startAtm(int i); // How do I pass the accounts vector back and forth?
	
	void* takeCut();
	
	void printStatus();
};

#endif 

	