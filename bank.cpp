# include "bank.h"

#define PASSWORD "password"
#define RAND_MAX 2
#define THREAD_SUCCESS(x) if (!x) exit(1)


void* threadAtm(void* bank){
	Bank* b = (Bank*) bank;
	b->startAtm((b->atm_run)++);
	return NULL;
}
void* threadCut(void* bank){
	Bank* b = (Bank*) bank;
	b->takeCut();
	return NULL;
}
void* threadPnt(void* bank){
	Bank* b = (Bank*) bank;
	b->printStatus();
	return NULL;
} 


int main(int argc, const char* argv[]){
	
	// Init bank
	int atm_num = argc - 1;
	char* files[atm_num];
	for (int i = 2 ; i < argc; i++){
		files[i-2] = argv[i];
		if (!argv[i]) fprintf(stderr, "illegal arguments");
		return (1);
	}
	
	Bank b(atm_num, files); // Maybe set as global?
	
	// Weave threads
	int ts; // thread success
	
	for (int i = 0 ; i < b.getAtmNum() ; i++){
		ts = pthread_create(b.getAtmThread(i), NULL, &threadAtm, (void*) &b);
		THREAD_SUCCESS(ts);
	}
	
	ts = pthread_create(b.getCutThread(), NULL, &threadCut, (void*) &b);
	THREAD_SUCCESS(ts);
	
	ts = pthread_create(b.getPntThread(), NULL, &threadPnt, (void*) &b);
	THREAD_SUCCESS(ts);
	
	// Wait for ATM threads to finish
	for (int i = 0 ; i < b.getAtmNum() ; i++){
		pthread_join(*b.getAtmThread(i), NULL);
	}
	b.setDone();
	
	pthread_join(*b.getCutThread());
	pthread_join(*b.getPntThread());
	
	return 
}

Bank::Bank(int atm_num, char* atm_files[]) :
	accs_(),
	atms_(atm_num,(pthread_t*) malloc(sizeof(pthread))),
	bank_(0, PASSWORD, 0),
	atm_num_(atm_num),
	atm_run_(0),
	acc_num_(0),
	done(false),
	cut_thread((pthread_t*) malloc(sizeof(pthread))),
	pnt_thread((pthread_t*) malloc(sizeof(pthread))) 
	{
		for (int i=0; i<atm_num; i++){
			strcpy(files_[i], atm_files[i]);
		}
	}

Bank::~Bank(){
	for (auto const& t : atms_){
		free(*t);
	}
	free(cut_thread);
	free(pnt_thread);
}

int Bank::getAtmNum() {
	return atms_num_;
}
int Bank::getAccNum() {
	return accs_.size();
}

void Bank::setDone() {
	done = true;
}
bool Bank::getDone() {
	return done;
}

pthread_t* Bank::getATmThread(int i) {
	return atms_[i];
}
pthread_t* Bank::getCutThread() {
	return cut_thread;
}
pthread_t* Bank::getPntThread() {
	return pnt_thread;
}

void Bank::startAtm(int i) {
	std::ifstream input(files_[i]);
	char command;
	char* args[];
	int num_args;
	for std::string line; getline(input, line);){
		num_args = 0;
		// line parse
		switch (command) {
			case 'O':
				// open new account
				break;
			case 'D':
				// deposit
				break;
			case 'W':
				// withdraw
				break;
			case 'B':
				// Balance
				break;
			case 'T':
				// transfer
				break;
			default:
				// Error of some sort
		}
	}
}

void Bank::takeCut() {
	int p;
	int com;
	while(!done){
		p = 2 + rand();
		for (auto & acc : accs_){
			com = acc->deductComission(p);
			acc->Transfer(&bank_, acc->ID, 0, 0, com);
		}
		sleep(3);
	}
	return;
}

void Bank::printStatus() {
	while(!done){
		printf("\33[2J");
		printf("\33[1;1H");
		printf("Current Bank Status\n");
		for (auto const& acc : accs_){
			acc->printStatus();
		}
		usleep(500000);
	}
	return;
}