# include "bank.h"

#define THREAD_SUCCESS(x) if (!x) exit(1)


void* threadAtm(void* bank){}
void* threadCut(void* bank){}
void* threadPnt(void* bank){} 



int main(int argc, const char* argv[]){
	
	// Init bank
	int atm_num = argc - 1;
	char* files[atm_num];
	for (int i = 2 ; i < argc; i++){
		files[i-2] = argv[i];
		if (!argv[i]) fprintf(stderr, "illegal arguments");
		return (1);
	}
	
	bank b(atm_num, files); // Maybe set as global?
	
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

bank::bank(int atm_num, char* atm_files[]){}

int bank::getAtmNum() {}

int bank::getAccNum() {}

pthread_t* bank::getATmThread(int i) {}
pthread_t* bank::getCutThread() {}
pthread_t* bank::getPntThread() {}

void* bank::startAtm(int i) {}

void* bank::takeCut() {}

void* bank::printStatus() {}