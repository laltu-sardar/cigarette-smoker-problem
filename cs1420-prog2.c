
/******************************************************************************\
*  --------------------------------------------------------------------------  *
*  LALTU SARDAR                                                                *
*  ROLL - 1420                                                                 *
*  Operating System                                                            *
*  Assignment on Synchronization Problems   								   *
*  Date of submission: 10.08.2015                                  			   *
*  --------------------------------------------------------------------------  *
*  PROBLEM:                                                                    *
*  Implement the Cigarette smokers synchronization problems. You need to       *
*  write two sets of programs:                                                 *
*                                                                              *
*  (a) Model each agent (e.g. a producer, consumer, reader, philosopher,       *
*      etc.) as a process, and each shared data structure using shared         *
*      memory. The execution of the various active entities should be          *
*      synchronized using semaphores.                                          *
*  (b) Model each agent as a thread. Do not use any synchronization related    *
*      system calls/library functions. Instead, synchronization should be      *
*      done using your own implementation of Algorithm 3 or Lamport's bakery   *
*      (whichever is appropriate).                                             *
*                                                                              *
*  General guidelines:                                                         *
*  # Design your code so that it is easy to demonstrate the race condition by  *
*    omitting the entry_section and exit_section (i.e. if you leave out the    *
*    entry_section and exit_section, your programme should exhibit the         *
*    inconsistencies that arise from race conditions).                         *
*  # The behaviour of each agent should have an element of randomness (use a   *
*    function like rand() along with sleep() in a suitable way for this).      *
*  # Include suitable printf functions in each agent so that the sequence of   *
*    operations performed by all the agents is clearly understandable.         *
*  # Special credit for starvation-free solutions where applicable.            *
*------------------------------------------------------------------------------*
* The Cigarette-Smokers Problem: Consider a system with three smoker processes *
* and one agent process. Each smoker continuously rolls a cigarette	           *
* and then smokes it. But to roll and smoke a cigarette, the smoker needs      *
* three ingredients: tobacco, paper, and matches. One of the smoker processes  *
* has paper, another has tobacco, and the third has matches. The               *
* agent has an infinite supply of all three materials. The agent places two of *
* the ingredients on the table. The smoker who has the remaining ingredient    *
* then makes and smokes a cigarette, signaling the agent on completion.        *
* The agent then puts out another two of the three ingredients, and the cycle  *
* repeats. Write a program to synchronize the agent and the smokers.           *
* -----------------------------------------------------------------------------*
\******************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#define N     50
#define TRUE    1
#define FALSE   0

void makeCigarette(int ,int);
void smoke(int, int );
void randomWait();
void* smoker(void* );
void* pusher(void* );
void* agent();

int choosing[4][N];			// declaration and initial values of global variables for implementing bakery lock unlock
int number[4][N];

int agentSem;				// The agent semaphore 
int smokerSem[3];			// Each smoker semaphore represents when a smoker has the items they need
int pusherSem[3];			// Each pusher pushes a certian type item, manage these with this semaphore
int pusher_lock;			// This semaphore gives the pusher exclusive access to the items on the table

int noOfSmoker[3];

char* smokerType[3] = {"Matches and Paper" , "Tobacco and Paper", "Matches and Tobacco"};
					// This array of strings describing what each smoker type NEEDS
char* smokerItem[3]={"Tobacco","Matches","  Paper"};
					// This array of strings describing what each smoker type does HAVE
int itemsOnTable[3] = { 0, 0, 0 };	// This list represents item types that are on the table.
int resource[3]={0,0,0};			//changed

struct ThreadParams{		//structure to send parameters to smoker processes
	int smokerType;			//smoker type indicates which type of smoker it is
	int smokerID;			//it gives unique id for each type of of process 
};

void semWait(int *S){		//semaphore wait function
	while (*S <= 0)			//busy waiting
		;
	*S -=1;					
	return;
}

void semSignal(int *S){		//semaphore signal function
	*S +=1;					
	return;
}

int max( int type) {		//finding maximum of an array
    int i = 0;
    int maximum = number[type][0];
    for (i = 0; i < N; i++) {
        if (maximum < number[type][i])
            maximum = number[type][i];
        }
    return maximum;
}
void lock(int i,int type) {	//lampor's bakery lock function
    int j = 0;
    choosing[type][i] = TRUE;
    number[type][i] = 1 + max(type);
    choosing[type][i] = FALSE;
    for (j = 0; j < N; j++) {
        while (choosing[type][j])
			;
        while (number[type][j] != 0 && (number[type][j] < number[type][i] || (number[type][i] == number[type][j]) && j < i)) {
		}
    }
}
void unlock(int i,int type) {	//lampor's bakery lock function
    number[type][i] = 0;
}



void* smoker(void* arg){

	struct ThreadParams smokerParam;				//thread parameter variable for smoker process
	smokerParam = *(struct ThreadParams*) arg;		//stored the thread parameter to this structure variable
	int smokerID = smokerParam.smokerID;			//get the smoker ID
	int type   = smokerParam.smokerType % 3;		//get the smoker type, 0, 1 and 2 for smoker with tobacco, paper and match respectively

	while(1){
		printf("* Smoker %d with %s --> Waiting for %s\n",smokerID, smokerItem[type ], smokerType[type]);
		
		lock(smokerID,type);						//lock the other smokers of same type using bakery algorithm
			
		semWait(&smokerSem[type]);					//waits for required semaphore
		makeCigarette(type ,smokerID);				//makes the Cigarette
		semSignal(&agentSem);						//smoker gives signal to the agent that it has taken the ingradient 
		smoke(type ,smokerID);						//smoker smokes
			
		unlock(smokerID,type);						//unlock the other smokers of same type using bakery algorithm
	}
	return NULL;
}


void* pusher(void* arg){
	int pusherID = *(int*) arg;

	while(1){
		semWait(&pusherSem[pusherID]);					// Wait for this pusher to be needed	
		lock(pusherID,3);								//lock the other pushers semaphore using bakery algorithm

		if (itemsOnTable[(pusherID + 1) % 3]){			// Check if the next item we need is on the table
			itemsOnTable[(pusherID + 1) % 3] = 0;		//if yes it takes the item 
			semSignal(&smokerSem[(pusherID + 2) % 3]);	//and signals the corresponding semaphore
		} 
		else if (itemsOnTable[(pusherID + 2) % 3]){		//else it Check if the next to the nex item we need is on the table
			itemsOnTable[(pusherID + 2) % 3] = 0;		//if yes it takes the item 
			semSignal(&smokerSem[(pusherID + 1) % 3]);	//and signals the corresponding semaphore
		}
		else{
			itemsOnTable[pusherID] = 1;					// The other item's aren't on the table yet
		}
		unlock(pusherID,3);								//unlock the other pushers to enter in their critical section
	}
	return NULL;
}


void* agent(){
	int k1,k2;

	srand (time(NULL));
	
	while(1){						//generally agent runs infinitely many times, but for checking purpose it is in finite loop

		k2=k1=rand()%3;				//this to generate random numbers between 0 to 2 to choose random items
		while(k2==k1){
			k2= rand()%3 ;
		}		

		semWait(&agentSem);			// Wait for a lock on the agent semaphore

		resource[k1]=resource[k2]=1; //changed

		printf("\n*********************************************************\n");
		printf("\033[0;35m-----> \033[0;33mAgent has given %s\033[0;35m<-----\033[0;0m\n", smokerType[3-k1-k2]);

		semSignal(&pusherSem[k1]);	// give Signal to 1st item
		semSignal(&pusherSem[k2]);	// give Signal to 2nd item

	}
	return NULL;
}

void makeCigarette(int type, int smokerID) {

	resource[(type+1)%3]=0;	//changed
	resource[(type+2)%3]=0;
	printf("\033[0;34m@ Smoker %d with %s --> started  making  cigarette.\033[0;0m\n",smokerID, smokerItem[type]);
	randomWait();
	printf("\033[0;34m@ Smoker %d with %s --> finished making  cigarette.\033[0;0m\n", smokerID,smokerItem[type]);
}

void smoke(int type, int smokerID){
	printf("\033[0;32m# Smoker %d with %s --> started  smoking cigarette.\033[0;0m\n",smokerID,smokerItem[ type]);
	randomWait();
	printf("\033[0;32m# Smoker %d with %s --> finished smoking cigarette.\033[0;0m\n",smokerID, smokerItem[type]);
}

void randomWait() {
	usleep(rand()%800000 +1000);  //wait for 10000-1000099 microsecond randomly 

}


int main(int argc, char* arvg[])
{
	int i,j, k;					//index variables, used for various indexing 
	int pusherID[3];			// this array is pusher ID's 
	int noOfThreadParams = 3*N;

	pthread_t smokerThreadID[3*N];//array of thread ids for smoker processes	
	pthread_t pusherThreadID[3];//array of thread ids for pusher processes
	pthread_t agentThreadID;	//thread id for agent processes

	struct ThreadParams smokerParam[noOfThreadParams];	//This array is for sending thread parameters


	for(i=0;i<4;i++)
		for(j=0;j<N;j++)
			number[i][j]=0;
	do{
		printf("\nGive the no of somker of each type\n");
		for ( i = 0; i < 3; ++i){							//Takes no of inputs for each smoker type 
			printf("Enter no of smoker with %s\t",smokerItem[i]);
			scanf("%d",&noOfSmoker[i]);
		//	noOfSmoker[i]=9;
		}
	}while((noOfSmoker[0]>20)||(noOfSmoker[1]>20)||(noOfSmoker[2]>20));

	printf("\n*********************** Process Started ***************************\n");

	agentSem=1;				//initialization of semaphore variables
	pusher_lock= 1;			
	for ( i = 0; i < 3; ++i){	
		smokerSem[i]= 0;
		pusherSem[i]= 0;
	}


	k=0;		//this is for indexing the thread parameters for different type of smoker process 
	for(j=0; j<3 ; j++){
		for ( i = 0; i < noOfSmoker[j]; ++i){	
			smokerParam[k].smokerType=j;		//initialization of smoker-type to thread parameter for smoker thread  
			smokerParam[k].smokerID=i;			//initialization of  smoker-ID to thread parameter for smoker thread

			if((pthread_create(&smokerThreadID[k], NULL, smoker, &smokerParam[k])==EAGAIN)	)	{	//create pthreads for different smoker processes
				perror("Smoker thread not created");				//gives error message if a smoker thread not created 
				return 0;
			}
			k++;
		}
	}


	for ( i = 0; i < 3; ++i){
		pusherID[i] = i;
		if (pthread_create(&pusherThreadID[i], NULL, pusher, &pusherID[i])==EAGAIN ){
												//create pthreads for different pusher threads
			perror("Pusher thread not created");//gives error message if a pusher thread not created 
			return 0;
		}
	}

	if (pthread_create(&agentThreadID, NULL, agent, NULL)==EAGAIN ){
												//create pthreads for agent thread
		perror(" agent thread not created");	//gives error message if agent thread not created 
		return 0;
	}

	for ( i = 0; i < k; ++i){
		if(pthread_join(smokerThreadID[i], NULL));		//joins the smoker threads
		perror(" smoker thread has not terminated succesfully");
	}
	
	return 0;
}



