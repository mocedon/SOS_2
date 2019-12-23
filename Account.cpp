#include "Account.h"
#include <unistd.h> //required for sleep

const auto& lock = pthread_mutex_lock;
const auto& unlock = pthread_mutex_unlock;
const auto& mutex_init = pthread_mutex_init;
const auto& mutex_destroy = pthread_mutex_destroy;
Account::Account(int id, string password, int balance) : ID(id), PASSWORD(password), BALANCE(balance)
{
	// readCount gets zero value using c++11 feature of static assignment inside a class.
	mutex_init(&WRITEMUTEX, NULL);
	mutex_init(&READMUTEX, NULL);
}


Account::Account(const Account& account) : ID(account.getId()), PASSWORD(account.getPassword()), BALANCE(account.getBalance())
{
	// readCount gets zero value using c++11 feature of static assignment inside a class.
	mutex_init(&WRITEMUTEX, NULL);
	mutex_init(&READMUTEX, NULL);
}

//Destructor:
Account::~Account()
{
	mutex_destroy(&WRITEMUTEX);
	mutex_destroy(&READMUTEX);
}


int Account::getReadCount() const
{
	return READCOUNTER;
}
string Account::getPassword() const
{
	return PASSWORD;
}

int Account::getBalance() const
{
	return BALANCE;
}


int Account::getId() const
{
	return ID;
}
void Account::deposit(int ATM, string PassReceived, int sum) {
	if (IdenticalPassword(ATM, PassReceived))
	{
		lock(&WRITEMUTEX);
		BALANCE =BALANCE+ sum;
		lock(&logMutex);
		fprintf(logtxt, "%d: Account %d new balance is %d after %d $ was deposited\n",ATM,ID,BALANCE,sum);
		unlock(&logMutex);
		usleep(1000000);
		unlock(&WRITEMUTEX);
	}
}

void Account::withdraw(int ATM, string PassReceived, int sum) {
	if (IdenticalPassword(ATM, PassReceived))
	{
		lock(&WRITEMUTEX);
		if (sum <= BALANCE)
		{
			BALANCE -= sum;
			lock(&logMutex);
			fprintf(logtxt, "%d: Account %d new balance is %d after %d $ was withdrew\n",ATM,ID,BALANCE,sum);
			unlock(&logMutex);
		}
		else // we want to withdraw more money than we have in the account
		{
			lock(&logMutex);
			fprintf(logtxt, "Error %d: Your transaction failed - account id %d balance is lower than %d\n",ATM,ID,sum);
			unlock(&logMutex);
		}
		sleep(1);
		unlock(&WRITEMUTEX);
	}
}


void Account::getBalance(int ATM, string PassReceived) {
		if (!IdenticalPassword(ATM, PassReceived))
		{
		return;
		}
		lock(&READMUTEX);
		READCOUNTER++;
		if (READCOUNTER == 1) // someone wants to read the value, so we need to lock the write lock
			lock(&WRITEMUTEX);
		unlock(&READMUTEX);
		lock(&logMutex);
		fprintf(logtxt, "%d: Account %d balance is %d\n",ATM,ID,BALANCE);
		unlock(&logMutex);
		lock(&READMUTEX);
		--READCOUNTER;
		if (!READCOUNTER) // no one wants to read the value, so we can unlock the write lock
			unlock(&WRITEMUTEX);
		sleep(1);
		unlock(&READMUTEX);
}


void Account::Transfer(int srcAccount, int receiverAccountIndex, Account& receiverAccount, string PassReceived, int ATM, int sum) {
		if (!IdenticalPassword(ATM, PassReceived))
		{
		return;
		}
		// lock the write lock of both accounts, ,sender and receiver, in particular order to avoid deadlock
		if (srcAccount >= receiverAccountIndex)
		{
			lock(&receiverAccount.WRITEMUTEX);
			lock(&WRITEMUTEX);
		}
		else
		{
			lock(&WRITEMUTEX);
			lock(&receiverAccount.WRITEMUTEX);
		}
		if (sum>=BALANCE)
		{
			BALANCE = BALANCE-sum;
			receiverAccount.BALANCE =receiverAccount.BALANCE+ sum;
			lock(&logMutex);
			fprintf(logtxt, "%d: Transfer %d from account %d to account %d new account balance is %d new target account balance is %d\n",ATM,sum,ID,receiverAccount.ID,BALANCE,receiverAccount.BALANCE);
			unlock(&logMutex);
		}
		else // we want to transfer more money than we have in the account
		{
			lock(&logMutex);
			fprintf(logtxt, "Error %d: Your transaction failed - account id %d balance is lower than %d\n",ATM,ID,sum);
			unlock(&logMutex);
		}

		sleep(1);
		// unlock the write lock of both accounts, sender and receiver
		if (srcAccount >= receiverAccountIndex)
		{
			unlock(&WRITEMUTEX);
			unlock(&receiverAccount.WRITEMUTEX);
		}
		else
		{
			unlock(&receiverAccount.WRITEMUTEX);
			unlock(&WRITEMUTEX);
		}
}


bool Account::IdenticalPassword(int ATM, string PassReceived)
{
	if (PassReceived != PASSWORD)
	{
	lock(&logMutex);
	fprintf(logtxt, "Error %d: Your transaction failed - password password for account id %d is incorrect\n",ATM,ID);
	unlock(&logMutex);
	return false;
	}
	return true;
}


int Account::deductCommission(double percentage) 
{
	lock(&WRITEMUTEX);
	BALANCE = BALANCE-(int)round(((double)BALANCE)*(percentage/100));//deducting the comission
	sleep(1);
	unlock(&WRITEMUTEX);
	return (int)round(((double)BALANCE)*(percentage/100));
}
