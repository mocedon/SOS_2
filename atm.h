#ifndef ATM_H_
#define ATM_H_

#include "Account.h"

class atm {   // bit - bank interaction transmission
	private:
	char* cmd_file;
	pthread_t* atm_thread;
	
	public:
	atm(char* file); // Constructor with cmds file
	
	pthread_t* getTid();
	
	void run_atm(); // Run all cmd to the ATM
	
	void bit_init(Account& acc, char* pas, int bal); // Start new account
	
	void bit_push(Account& acc, char* pas, int push); // Deposit money to account
	
	void bit_pull(Account& acc, char* pas, int pull); // Withdraw money from account
	
	void bit_blame(Account& acc, char* pas); // Get the balance for an account
	
	void bit_checkout(Account& acc, char* pass, Account& to, int sum); // Transfer money 
	};
	
#endif
