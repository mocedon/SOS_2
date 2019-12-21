#include "Account.h"

typedef pthread_mutex_init MUTEX_INIT;
typedef pthread_mutex_lock LOCK;
typedef pthread_mutex_unlock UNLOCK;

Account::Account(int id, char* password, int balance) : ID(id), PASSWORD(password), BALANCE(balance)
{
	//READCOUNTER=0; not neccesary.
	MUTEX_INIT(&WRITELOCK, NULL);
	MUTEX_INIT(&READLOCK, NULL);
}

Account::Account(const Account& account) : ID(account.getId()), PASSWORD(account.getPassword()), BALANCE(account.getBalance())
{
	READCOUNTER=0;
	MUTEX_INIT(&WRITELOCK, NULL);
	MUTEX_INIT(&READLOCK, NULL);
}

int Account::getReadCount() const
{
	return READCOUNTER;
}
int Account::getId() const
{
	return ID;
}

int Account::getBalance() const
{
	return BALANCE;
}

string Account::getPassword() const
{
	return PASSWORD;
}

void Account::deposit(int ATM, char* PassReceived, int sum) {
	if (IdenticalPassword(PassReceived,ATM))
	{
		LOCK(&WRITELOCK);
		BALANCE += sum;
		LOCK(&lock_log);
		logtxt << ATM << ": the account " << ID << " current balance is " << BALANCE << " following a deposit of " << sum << " $" << endl;
		UNLOCK(&lock_log);
		usleep(1000000);
		UNLOCK(&WRITELOCK);
	}
}


void Account::withdraw(int ATM, char* PassReceived, int sum) {
	if (IdenticalPassword(PassReceived,ATM))
	{
		LOCK(&WRITELOCK);

		if (sum <= BALANCE) //legal withdrawing
		{
			BALANCE -= sum;
			LOCK(&lock_log);
			logtxt << ATM << ": the account " << ID << " current balance is " << BALANCE << "following a withdraw of" << sum << " $w" << endl;
			UNLOCK(&lock_log);
		}
		else
		{
			// illegal attempt to withdraw amount bigger than the balance.
			LOCK(&lock_log);
			logtxt << "error " << ATM << ": the transaction has failed - account id " << ID << " the balance is insufficient, action requires balance of at least " << sum << endl;
			UNLOCK(&lock_log);
		}
		sleep(1);
		UNLOCK(&WRITELOCK);
	}
}

int Account::IdenticalPassword(char* PassReceived,int ATM)
{
	if (PassReceived==PASSWORD)
		return 1;
	LOCK(&lock_log);
	logtxt << "error " << ATM << ": the transaction has failed - password " << "password for account id " << ID << " is invalid" << endl;
	UNLOCK(&lock_log);
	return 0;
}

void Account::getBalance(int ATM, char* PassReceived) {
	if (IdenticalPassword(PassReceived,ATM))
	{
		LOCK(&READLOCK);
		READCOUNTER++;
		if (READCOUNTER == 1)
			LOCK(&WRITELOCK); //gets locked in attempt of reading the value.
		UNLOCK(&READLOCK);
		LOCK(&lock_log);
		logtxt << ATM << ": the account " << ID << " balance is: " << BALANCE << endl;
		UNLOCK(&lock_log);
		LOCK(&READLOCK);
		READCOUNTER--;
		if (!READCOUNTER)
			UNLOCK(&WRITELOCK); //not read by anyone and therefore gets unlocked.
		sleep(1);
		UNLOCK(&READLOCK);
	}
}

int Account::deductComission(double percentage) 
{
	LOCK(&WRITELOCK);
	int commission = (int)round(((double)BALANCE)*(percentage/100));
	BALANCE -= commission;
	sleep(1);
	UNLOCK(&WRITELOCK);
	return commission;
}


void Account::Transfer( Account& DestAccount, int SourceAccountIndex, int DestAccountIndex, char* PassReceived, int ATM, int sum) {
	if (IdenticalPassword(PassReceived,ATM))
	{
		//beginnning by locking both accounts.
		if (SourceAccountIndex < DestAccountIndex)
		{
			LOCK(&WRITELOCK);
			LOCK(&receiverAccount.WRITELOCK);
		}
		else
		{
			LOCK(&receiverAccount.WRITELOCK);
			LOCK(&WRITELOCK);
		}
		if (sum >=BALANCE) //legal transfer
		{
			BALANCE -= sum;
			receiverAccount.BALANCE += sum;
			LOCK(&lock_log);
			logtxt << ATM << ": transfer " << sum << " from account " << ID << " to destionation account " << receiverAccount.ID <<
				"following the transfer, new account balance is " << BALANCE << " new destination account balance is " << receiverAccount.BALANCE << endl;
			UNLOCK(&lock_log);
		}
		else // illegal transfer
		{
			LOCK(&lock_log);
			logtxt << "error " << ATM << ": the transaction has failed - account id " << ID << " the balance is insufficient, action requires balance of at least " << sum << endl;
			UNLOCK(&lock_log);
		}
		sleep(1);
		if (SourceAccountIndex < DestAccountIndex)
		{
			UNLOCK(&receiverAccount.WRITELOCK);
			UNLOCK(&WRITELOCK);
		}
		else
		{
			UNLOCK(&WRITELOCK);
			UNLOCK(&receiverAccount.WRITELOCK);
		}
		//finally we are unlocking both accounts.
	}
}

