#include "Bank.h"
#include "Account.h"
#define MAXCMD 10
#define PASSWORD "password"
#define RANDMAX 2
#define THREAD_SUCCESS(x) if (x) return -1;

Bank* bank;
MUTEX logMutex;
MUTEX finishedMutex;
int ATMsLeft;
FILE* logtxt;
int TotalATMs;

const auto& MUTEX_INIT = pthread_mutex_init;
const auto& LOCK = pthread_mutex_lock;
const auto& UNLOCK = pthread_mutex_unlock;
const auto& MUTEX_DESTROY= pthread_mutex_destroy;
Bank::Bank() // this is the bank constructor
{
	AccountsVector = vector<Account>();
	MUTEX_INIT(&VecWRITEMUTEX, NULL);
	MUTEX_INIT(&VecREADMUTEX, NULL);
	MUTEX_INIT(&BANKACCOUNTMUTEX, NULL);
}
Bank::~Bank() // this is the bank destructor including the mutex_destroy calls.
{
	MUTEX_DESTROY(&VecWRITEMUTEX);
	MUTEX_DESTROY(&VecREADMUTEX);
	MUTEX_DESTROY(&BANKACCOUNTMUTEX);
}
void* CommissionThreadCut(void* arg)
{
	LOCK(&finishedMutex);
	while (ATMsLeft) // while there are ATMS still left. namely ATMSLeft is greater than zero.
	{
		UNLOCK(&finishedMutex);
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER++;
		if (bank->VecREADCOUNTER == 1)
		{
			LOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);
		double tmprand=(double)(rand() % 100);
		double percentage = (tmprand / 50) + RANDMAX;
		for (int i=0, int CMSN=0;i<bank->AccountsVector.size();i++)
		{
			Account &currentAcc = const_cast<Account&>(bank->AccountsVector[i]);
			int CMSN = bank->AccountsVector[i].deductCommission(percentage);
			LOCK(&bank->BANKACCOUNTMUTEX);
			bank->BANKBALANCE =bank->BANKBALANCE+ CMSN;
			UNLOCK(&bank->BANKACCOUNTMUTEX);
			LOCK(&logMutex);
			fprintf(logtxt, "Bank: commissions of %f %% were charged, the bank gained %d $ from account %d\n",percentage,commission,bank->AccountsVector[i].getId());
			UNLOCK(&logMutex);
		}
		
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER--;
		if (!bank->VecREADCOUNTER)
		{
			UNLOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);
		sleep(3);
	}
}

void* threadPnt(void* arg)
{
	LOCK(&finishedMutex);
	while (ATMsLeft)
	{
		UNLOCK(&finishedMutex);
		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER++;
		if (bank->VecREADCOUNTER == 1)
		{
			LOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &currentAcc = const_cast<Account&>(bank->AccountsVector[i]);
			LOCK(&(currentAcc.WRITEMUTEX));
		}
		
		//remove the previous screen:
		printf("\033[2J"); // delete previous screen
		// locate marker in the top left corner
		printf("\033[1;1H");
		printf( "Current Bank Status\n");
		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &currentAcc = const_cast<Account&>(bank->AccountsVector[i]);
			printf("Account %d: Balance – %d $ , Account Password – %s\n",currentAcc.getId(),currentAcc.getBalance(),currentAcc.getPassword().c_str());
		}
		
		LOCK(&bank->BANKACCOUNTMUTEX);
		printf(".\n.\nThe Bank has %d $\n",bank->BANKBALANCE);
		UNLOCK(&bank->BANKACCOUNTMUTEX);
		for (int i=0; i< bank->AccountsVector.size(); i++)
		{
			Account &currentAcc = const_cast<Account&>(bank->AccountsVector[i]);
			UNLOCK(&(currentAcc.WRITEMUTEX));
		}

		LOCK(&bank->VecREADMUTEX);
		bank->VecREADCOUNTER--;
		if (!bank->VecREADCOUNTER)
		{
			UNLOCK(&bank->VecWRITEMUTEX);
		}
		UNLOCK(&bank->VecREADMUTEX);

		usleep(500000);
	}
}

void* threadAtm(void* data)
{	
	int i = 0, num_arg = 0;
	char* command[MAXCMD], buffer[MAXLINESIZE];
	char* delimiters = (char*)" \t\n";
	ATMInfo ATMInfoType = *(ATMInfo*)data;
	FILE* source = fopen(ATMInfoType.CMDSfile, "r");
	while (fgets(buffer, MAXLINESIZE, source) != NULL)
	{
		if (strcmp(buffer, "\n") != 0)
		{ //here we are making sure this is not just \n.
			command[0] = strtok(buffer, delimiters);
			for (i = 1; i < MAXCMD; i++)
			{
				command[i] = strtok(NULL, delimiters);
				if (command[i] != NULL)
					num_arg++;
			}
			if (!(bank->findbyID(atoi(command[1]))) && (strcmp(command[0], "O")) && (strcmp(command[0], "T")))
				// searching for the account id for performing the cmd:
			{
				LOCK(&logMutex);
				fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATMInfoType.ID,atoi(command[1]));
				UNLOCK(&logMutex);
			}
			
			else
			{
				// moving on to the perfoming of the given cmd:
				if (!strcmp(command[0], "T"))
				{
					bank->Transfer(ATMInfoType.ID, atoi(command[1]), string(command[2]), atoi(command[3]), atoi(command[4]));
				}
				else if (!strcmp(command[0], "W"))
				{
					bank->withdraw(ATMInfoType.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (!strcmp(command[0], "O"))
				{
					bank->newAccount(ATMInfoType.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (!strcmp(command[0], "D"))
				{
					bank->deposit(ATMInfoType.ID, atoi(command[1]), string(command[2]), atoi(command[3]));
				}
				else if (!strcmp(command[0], "B"))
				{
					bank->getBalance(ATMInfoType.ID, atoi(command[1]), string(command[2]));
				}
				else
				{
					break;
				}
			}
			usleep(100);
		}
	}
	fclose(source);
	LOCK(&finishedMutex);
	ATMsLeft--;
	UNLOCK(&finishedMutex);
	pthread_exit(NULL);
}
void Bank::Transfer(int ATM, int desiredAccountID, string PassReceived, int receiverAccountID, int sum)
{
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	if (!findbyID(desiredAccountID))
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATM,desiredAccountID);
		UNLOCK(&logMutex);
	}
	else if (!findbyID(receiverAccountID))
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account id %d does not exist\n",ATM,receiverAccountID);
		UNLOCK(&logMutex);
	}
	else
	{
		Account* desiredAccount = LocateAccount(desiredAccountID);
		int srcAccount = getDistFromBeginning(desiredAccount);
		Account* receiverAccount = LocateAccount(receiverAccountID);
		int receiverAccountIndex = getDistFromBeginning(receiverAccount);
		if (receiverAccount != NULL)
		{
			if (desiredAccount != NULL)
				desiredAccount->Transfer(srcAccount, receiverAccountIndex, (*receiverAccount), PassReceived, ATM, sum);
		}
	}
	
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (!VecREADCOUNTER) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}

void Bank::withdraw(int ATM, int ACCOUNTID, string PassReceived, int sum)
{
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	Account* desiredAccount = LocateAccount(ACCOUNTID);
	if (desiredAccount != NULL)
	{
		desiredAccount->withdraw(ATM, PassReceived, sum);
	}
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (!VecREADCOUNTER) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}

void Bank::deposit(int ATM, int ACCOUNTID, string PassReceived, int sum)
{
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	Account* desiredAccount = LocateAccount(ACCOUNTID);
	if (desiredAccount != NULL)
	{
		desiredAccount->deposit(ATM, PassReceived, sum);
	}
	
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (!VecREADCOUNTER) {
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}

void Bank::newAccount(int ATM, int ACCOUNTID, string PassReceived, int ReceivedBalance)
{
	LOCK(&VecWRITEMUTEX);
	sleep(1);
	Account* desiredAccount = LocateAccount(ACCOUNTID);
	if (desiredAccount == NULL)
	{
		Account newAccount(ACCOUNTID, PassReceived, ReceivedBalance);
		AccountsVector.push_back(newAccount);
		LOCK(&logMutex);
		fprintf(logtxt, "%d: New account id is %d with password %d and initial balance %d\n",ATM,ACCOUNTID,atoi(PassReceived.c_str()),ReceivedBalance);
		UNLOCK(&logMutex);
	}
	else
	{
		LOCK(&logMutex);
		fprintf(logtxt, "Error %d: Your transaction failed - account with the same id exists\n",ATM );
		UNLOCK(&logMutex);
	}
	UNLOCK(&VecWRITEMUTEX);
}



void Bank::getBalance(int ATM, int ACCOUNTID, string PassReceived)
{
	LOCK(&VecREADMUTEX);
	sleep(1);
	VecREADCOUNTER++;
	if (VecREADCOUNTER == 1)
	{
		LOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
	Account* desiredAccount = LocateAccount(ACCOUNTID);
	if (desiredAccount != NULL)
	{
		desiredAccount->getBalance(ATM, PassReceived);
	}
	LOCK(&VecREADMUTEX);
	VecREADCOUNTER--;
	if (!VecREADCOUNTER)
	{
		UNLOCK(&VecWRITEMUTEX);
	}
	UNLOCK(&VecREADMUTEX);
}

int Bank::findbyID(int id)
{
	return LocateAccount(id)==NULL?0:1;
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

Account* Bank::LocateAccount(int id)
{
	for (int i=0;i<AccountsVector.size();i++)
	{
		if (AccountsVector[i].getId()==id)
			return &AccountsVector[i];
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	bank = new Bank();
	ATMsLeft = 0;
	TotalATMs = atoi(argv[1]);
	ATMsLeft = TotalATMs;
	if (TotalATMs +2 != argc) 
	{ 
		printf("illegal arguments\n");
		return -1;
	}

	MUTEX_INIT(&finishedMutex, NULL);
	MUTEX_INIT(&logMutex, NULL);
	logtxt = fopen("log.txt", "w+");
	ATMInfo* ATMInfoArr = new ATMInfo[TotalATMs]; 
	pthread_t* atms = new pthread_t[TotalATMs];
	pthread_t commission;
	pthread_t print;
	int ts;
	for (int i = 0; i < TotalATMs; i++)
	{
		(ATMInfoArr[i]).ID = i + 1;
		strcpy((ATMInfoArr[i]).CMDSfile, argv[i + 2]);
		if (ts=pthread_create(&atms[i], NULL, threadAtm, (void*)&ATMInfoArr[i]))
			{
				fprintf(stderr,"error: could not run pthread_create\n");
				THREAD_SUCCESS(ts);
			}
	}

	if (ts=pthread_create(&commission, NULL, CommissionThreadCut, NULL))
	{
		fprintf(stderr,"error: could not run pthread_create\n");
		THREAD_SUCCESS(ts);
	}

	if (ts=pthread_create(&print, NULL, threadPnt, NULL))
	{
		fprintf(stderr,"error: could not run pthread_create\n");
		THREAD_SUCCESS(ts);
	}
	for (int i = 0; i < TotalATMs; i++)
	{
		if (ts=pthread_join(atms[i], NULL))
		{
			fprintf(stderr,"error: could not run pthread_join\n");
			THREAD_SUCCESS(ts);
		}
	}
	
	if (ts=pthread_join(commission, NULL))
	{
		fprintf(stderr,"error: could not run pthread_join\n");
		THREAD_SUCCESS(ts);
	}

	if (ts=pthread_join(print, NULL))
	{
		fprintf(stderr,"error: could not run pthread_join\n");
		THREAD_SUCCESS(ts);
	}

	if(ts=fclose(logtxt)==-1)
	{
		fprintf(stderr,"error: closed more than once\n");
		THREAD_SUCCESS(ts);
	}
	logtxt=NULL;
	delete atms;
	delete ATMInfoArr;
	delete bank;

	return 0;
}