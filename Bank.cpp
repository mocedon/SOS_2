#include "Bank.h"
#define MAXCMD 10
//global locks
MUTEX logMutex;
MUTEX finishedMutex;

int ATMsLeft;
int TotalATMs;
FILE* logtxt;

Bank* bank;

const auto& MUTEX_INIT = pthread_mutex_init;
const auto& LOCK = pthread_mutex_lock;
const auto& UNLOCK = pthread_mutex_unlock;
const auto& MUTEX_DESTROY= pthread_mutex_destroy;
Bank::Bank()
{
	AccountsVector = vector<Account>();
	BANKBALANCE = 0;
	MUTEX_INIT(&VecWRITEMUTEX, NULL);
	pthread_mutex_init(&VecREADMUTEX, NULL);
	VecREADCOUNTER = 0;
	pthread_mutex_init(&BANKACCOUNTMUTEX, NULL);
}
Bank::~Bank()
{
	MUTEX_DESTROY(&VecWRITEMUTEX);
	MUTEX_DESTROY(&VecREADMUTEX);
	MUTEX_DESTROY(&BANKACCOUNTMUTEX);
}

void* atmThreadFunc(void* data)
{
	ATMInfo ATMInfo_t = *(ATMInfo*)data; // struct to hold the ATM data
	
	// read instructions from file
	FILE* from = fopen(ATMInfo_t.CMDSfile, "r");
	int i = 0, num_arg = 0;
	char* command[MAXCMD], buffer[MAXLINESIZE];
	char* delimiters = (char*)" \t\n";
	while (fgets(buffer, MAXLINESIZE, from) != NULL)
	{
		if (strcmp(buffer, "\n") != 0) //verify that this is not an empty line
		{
			command[0] = strtok(buffer, delimiters);
			for (i = 1; i < MAXCMD; i++)
			{
				command[i] = strtok(NULL, delimiters);
				if (command[i] != NULL)
					num_arg++;
			}
			
			// check if account id exists for relevant command
			if (!(bank->findbyID(atoi(command[1]))) && (strcmp(command[0], "O") != 0) && (strcmp(command[0], "T") != 0))
			{
				LOCK(&logMutex);
				fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATMInfo_t.ID,atoi(command[1]));
				UNLOCK(&logMutex);
			}
			
			// execute command
			else
			{
				char* transType = command[0];
				if (strcmp(transType, "O") == 0)
				{
					bank->addAccount(ATMInfo_t.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (strcmp(transType, "D") == 0)
				{
					bank->deposit(ATMInfo_t.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (strcmp(transType, "W") == 0)
				{
					bank->withdraw(ATMInfo_t.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (strcmp(transType, "B") == 0)
				{
					bank->getBalance(ATMInfo_t.ID, atoi(command[1]), string(command[2]));
				}
				else if (strcmp(transType, "T") == 0)
				{
					bank->Transfer(ATMInfo_t.ID, atoi(command[1]), string(command[2]), atoi(command[3]), atoi(command[4]));
				}
				else
				{
					break;
				}
			}
			usleep(100);
		}
	}
	fclose(from);
	
	//update that an ATM has finished
	LOCK(&finishedMutex);
	ATMsLeft--;
	UNLOCK(&finishedMutex);
	
	pthread_exit(NULL);
}


void* DeductComissionThread(void* arg)
{
	LOCK(&finishedMutex);
	while (ATMsLeft)
	{
		UNLOCK(&finishedMutex);
		
		// lock accounts vector
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER++;
		if (bank->VecREADCOUNTER == 1)
		{
			LOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		double percentage = ((double)(rand() % 101) / 50) + 2; //get a random number between 2-4
		int commission = 0;
		for (int i=0;i<bank->AccountsVector.size();i++)
		{
			Account &curr_acc = const_cast<Account&>(bank->AccountsVector[i]);
			int commission = bank->AccountsVector[i].deductCommission(percentage); //take commission
			LOCK(&bank->BANKACCOUNTMUTEX);
			bank->BANKBALANCE += commission; //add commission to bank
			UNLOCK(&bank->BANKACCOUNTMUTEX);
			LOCK(&logMutex);
			fprintf(logtxt, "Bank: commissions of %f %% were charged, the bank gained %d $ from account %d\n",percentage,commission,bank->AccountsVector[i].getId());
			UNLOCK(&logMutex);
		}
		
		//unlock accounts vector
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER--;
		if (bank->VecREADCOUNTER == 0)
		{
			UNLOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		sleep(3); // sleep for 3 secs
	}
}


void* BankPrintThread(void* arg)
{
	LOCK(&finishedMutex);
	while (ATMsLeft)
	{
		UNLOCK(&finishedMutex);
		
		// lock accounts vector
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER++;
		if (bank->VecREADCOUNTER == 1)
		{
			LOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		// lock all bank accounts locks
		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &curr_acc = const_cast<Account&>(bank->AccountsVector[i]);
			LOCK(&(curr_acc.WRITEMUTEX));
		}

		// Print bank status
		printf("\033[2J"); // delete previous screen
		printf("\033[1;1H"); // put the marker at the top left corner of the screen
		printf( "Current Bank Status\n");
		//print status of every account in the bank
		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &curr_acc = const_cast<Account&>(bank->AccountsVector[i]);
			printf("Account %d: Balance - %d $ , Account Password - %s\n",curr_acc.getId(),curr_acc.getBalance(),curr_acc.getPassword().c_str());
		}
		
		LOCK(&bank->BANKACCOUNTMUTEX);
		printf(".\n.\nThe Bank has %d $\n",bank->BANKBALANCE);
		UNLOCK(&bank->BANKACCOUNTMUTEX);

		// Unlock all bank accounts locks
		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &curr_acc = const_cast<Account&>(bank->AccountsVector[i]);
			UNLOCK(&(curr_acc.WRITEMUTEX));
		}

		// unlock accounts vector
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER--;
		if (bank->VecREADCOUNTER == 0)
		{
			UNLOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		usleep(500000); //sleep for 0.5 secs
	}
}


void Bank::addAccount(int ATM, int ACCOUNTID, string PassReceived, int ReceivedBalance)
{
	//lock accounts vector
	LOCK(&VecWRITEMUTEX);
	sleep(1);
	Account* wantedAccount = LocateAccount(ACCOUNTID);
	
	//find the wanted accounts and make the transfer
	if (wantedAccount != NULL) //if account already exists
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account with the same id exists\n",ATM );
		UNLOCK(&logMutex);
	}
	else
	{
		Account newAccount(ACCOUNTID, PassReceived, ReceivedBalance);
		AccountsVector.push_back(newAccount);
		LOCK(&logMutex);
		fprintf(logtxt, "%d: New account id is %d with password %d and initial balance %d\n",ATM,ACCOUNTID,atoi(PassReceived.c_str()),ReceivedBalance);
		UNLOCK(&logMutex);
	}
	UNLOCK(&VecWRITEMUTEX);
}


void Bank::deposit(int ATM, int ACCOUNTID, string PassReceived, int sum)
{
	//lock accounts vector
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	
	//find the wanted account and make the deposit
	Account* wantedAccount = LocateAccount(ACCOUNTID);
	if (wantedAccount != NULL) //if wanted account exists
	{
		wantedAccount->deposit(ATM, PassReceived, sum);
	}
	
	//unlock accounts vector
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (VecREADCOUNTER == 0) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}


void Bank::withdraw(int ATM, int ACCOUNTID, string PassReceived, int sum)
{
	//lock accounts vector
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	
	//find the wanted account and make the withdraw
	Account* wantedAccount = LocateAccount(ACCOUNTID);
	if (wantedAccount != NULL) //if wanted account exists
	{
		wantedAccount->withdraw(ATM, PassReceived, sum);
	}
	
	//unlock accounts vector
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (VecREADCOUNTER == 0) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}


void Bank::getBalance(int ATM, int ACCOUNTID, string PassReceived)
{
	//lock accounts vector
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	
	//find the wanted account and get the balance
	Account* wantedAccount = LocateAccount(ACCOUNTID);
	if (wantedAccount != NULL) //if wanted account exists
	{
		wantedAccount->getBalance(ATM, PassReceived);
	}
	
	//unlock accounts vector
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (VecREADCOUNTER == 0) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}


void Bank::Transfer(int ATM, int wantedAccountID, string PassReceived, int receiverAccountID, int sum)
{
	//lock accounts vector
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	
	//check that both accounts exist and make the transfer
	//if one of the accounts doesn't exist, print an error
	if (!findbyID(wantedAccountID))
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATM,wantedAccountID);
		UNLOCK(&logMutex);
	}
	else if (!findbyID(receiverAccountID))
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATM,receiverAccountID);
		UNLOCK(&logMutex);
	}
	else //if both accounts exist make the transfer
	{
		Account* wantedAccount = LocateAccount(wantedAccountID);
		int srcAccount = getDistFromBeginning(wantedAccount);
		Account* receiverAccount = LocateAccount(receiverAccountID);
		int receiverAccountIndex = getDistFromBeginning(receiverAccount);
		if (wantedAccount != NULL && receiverAccount != NULL)
		{
			wantedAccount->Transfer(srcAccount, receiverAccountIndex, (*receiverAccount), PassReceived, ATM, sum);
		}
	}
	
	//unlock accounts vector
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (VecREADCOUNTER == 0) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}


Account* Bank::LocateAccount(int id)
{
	for (int i=0;i<AccountsVector.size();i++)
	{
		if (AccountsVector[i].getId()==id)
			return &AccountsVector[i];
	}
	return NULL;
}

int Bank::getDistFromBeginning(Account* acc)
{
	for (int i=0;i<AccountsVector.size();i++)
	{
		if (&AccountsVector[i]==acc)
			return i;
	}
	return -1;
}


int Bank::findbyID(int id)
{
	return LocateAccount(id)==NULL?0:1;
}


int main(int argc, char *argv[])
{
	bank = new Bank();
	ATMsLeft = 0;
	// Check if legal arguments
	TotalATMs = atoi(argv[1]);
	ATMsLeft = TotalATMs;
	if (TotalATMs != argc - 2) 
	{ 
		printf("illegal arguments\n");
		return -1;
	}

	pthread_mutex_init(&finishedMutex, NULL);

	// Execute input file
	pthread_mutex_init(&logMutex, NULL);
	logtxt = fopen("log.txt", "w+");
	pthread_t* atms = new pthread_t[TotalATMs];
	ATMInfo* ATMInfoArr = new ATMInfo[TotalATMs]; 

	// Create ATM, commission and print threads
	pthread_t commission;
	pthread_t print;
	int i;
	for (i = 0; i < TotalATMs; i++)
	{
		(ATMInfoArr[i]).ID = i + 1;
		strcpy((ATMInfoArr[i]).CMDSfile, argv[i + 2]);
		if (pthread_create(&atms[i], NULL, atmThreadFunc, (void*)&ATMInfoArr[i]))
			{
				fprintf(stderr,"error: could not run pthread_create\n");
				return -1;
			}
	}

	if (pthread_create(&commission, NULL, DeductComissionThread, NULL)) //Create the commission thread
	{
		fprintf(stderr,"error: could not run pthread_create\n");
		return -1;
	}

	if (pthread_create(&print, NULL, BankPrintThread, NULL)) //Create the print thread
	{
		fprintf(stderr,"error: could not run pthread_create\n");
		return -1;
	}

	// Wait for all threads to finish
	for (i = 0; i < TotalATMs; i++)
	{
		if (pthread_join(atms[i], NULL))
		{
			fprintf(stderr,"error: could not run pthread_join\n");
			return -1;
		}
	}
	
	if (pthread_join(commission, NULL))
	{
		fprintf(stderr,"error: could not run pthread_join\n");
		return -1;
	}

	if (pthread_join(print, NULL))
	{
		fprintf(stderr,"error: could not run pthread_join\n");
		return -1;
	}

	if(fclose(logtxt)==-1)
	{
		fprintf(stderr,"error: closed more than once\n");
	}
	logtxt=NULL;
	delete atms;
	delete ATMInfoArr;
	delete bank;

	return 0;
}