
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
* and one agent process. Each smoker continuously rolls a cigarette	       *
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


#include <semaphore.h> 	/*sem_t, sem_wait, sem_post*/
#include <stdio.h>		/*input output*/
#include <unistd.h>		//usleep
#include <stdlib.h>		//sizeof, NULL, 
#include <sys/wait.h>	//wait(pid_t)
#include <sys/mman.h>	//mmap
#include <time.h>		//for srand function
#include <errno.h>		// error handling

void makeCigarette(int );
void smoke(int );
void randomWait();
void* smoker(void* );
void* pusher(void* );
void* agent();

#define N 50

struct Shared{
	sem_t agentSem;				// The agent semaphore 
	sem_t smokerSem[3];			// Each smoker semaphore represents when a smoker has the items they need
	sem_t pusherSem[3];			// Each pusher pushes a certian type item, manage these with this semaphore
	sem_t pusher_lock;			// This semaphore gives the pusher exclusive access to the items on the table
	int itemsOnTable[3]; 		// This list represents item types that are on the table.
	int resource[3];	//changed
} * g_shm ;				//global shared Memory


char* smokerType[3] = {"Matches and Paper" , "Tobacco and Paper", "Matches and Tobacco"};
					// This array of strings describing what each smoker type NEEDS
char* smokerItem[3]={"Tobacco","Matches","  Paper"};
					// This array of strings describing what each smoker type does HAVE

void* smoker(void* arg){

	int i,type   = (*(int*) arg) % 3;		//get the smoker type, 0, 1 and 2 for smoker with tobacco, paper and match respectively

	for ( i = 0; i < N; ++i){	
//	while(1){
		printf("* Smoker with %s --> Waiting for %s\n", smokerItem[type ], smokerType[type]);
					
 		sem_wait(&g_shm->smokerSem[type]);	//waits for required semaphore
		makeCigarette(type );				//makes the Cigarette					
		sem_post(&g_shm->agentSem);			//smoker gives signal to the agent that it has taken the ingradient 
		smoke(type );						//smoker smokes
			
	}
	return NULL;
}


void* pusher(void* arg){
	int i,pusherType = *(int*) arg;

	for ( i = 0; i < N; ++i){	
//	while(1){						
		sem_wait(&g_shm->pusherSem[pusherType]);				// Wait for this pusher to be needed
							
		sem_wait(&g_shm->pusher_lock);							//lock the other pushers semaphore using bakery algorithm

		if (g_shm->itemsOnTable[(pusherType + 1) % 3]){			// Check if the next item we need is on the table
			g_shm->itemsOnTable[(pusherType + 1) % 3] = 0;		//if yes it takes the item 	
			sem_post(&g_shm->smokerSem[(pusherType + 2) % 3]);	//and signals the corresponding semaphore

		} 
		else if (g_shm->itemsOnTable[(pusherType + 2) % 3]){	//else it Check if the next to the nex item we need is on the table
			g_shm->itemsOnTable[(pusherType + 2) % 3] = 0;		//if yes it takes the item 	
			sem_post(&g_shm->smokerSem[(pusherType + 1) % 3]);	//and signals the corresponding semaphore
		}
		else{
			g_shm->itemsOnTable[pusherType] = 1;				// The other item's aren't on the table yet
		}								
		sem_post(&g_shm->pusher_lock);							//unlock the other pushers to enter in their critical section
	}
	return NULL;
}


void* agent(){
	int k1,k2;
	int i;

	srand (time(NULL));

	for ( i = 0; i < N; ++i){	
//	while(1){						//generally agent runs infinitely many times, but for checking purpose it is in finite loop

		k2=k1=rand()%3;				//this to generate random numbers between 0 to 2 to choose random items
		while(k2==k1){
			k2= rand()%3 ;
		}		
		sem_wait(&g_shm->agentSem);// Wait for a lock on the agent semaphore
		g_shm->resource[k1]=1;
		g_shm->resource[k2]=1;


		printf("\n*********************************************************\n");
		printf("\033[0;35m-----> \033[0;33mAgent has given %s\033[0;35m<-----\033[0;0m\n", smokerType[3-k1-k2]);

		sem_post(&g_shm->pusherSem[k1]);// give Signal to 1st item	//changed
		sem_post(&g_shm->pusherSem[k2]);// give Signal to 2nd item


	}
	return NULL;
}

void makeCigarette(int type) {
	g_shm->resource[(type+1)%3]=0;	//changed
	g_shm->resource[(type+2)%3]=0;	//changed
	printf("\033[0;34m@ Smoker with %s --> started  making  cigarette.\033[0;0m\n", smokerItem[type]);
	randomWait();
	printf("\033[0;34m@ Smoker with %s --> finished making  cigarette.\033[0;0m\n", smokerItem[type]);
}

void smoke(int type ){
	printf("\033[0;32m# Smoker with %s --> started  smoking cigarette.\033[0;0m\n",smokerItem[ type]);
	randomWait();
	printf("\033[0;32m# Smoker with %s --> finished smoking cigarette.\033[0;0m\n", smokerItem[type]);
}

void randomWait() {
	usleep(rand()%800000 + 1000);  //changed

}


int main(int argc, char* arvg[])
{
	int i;					//index variables, used for various indexing 
	pid_t pid;

	g_shm = mmap( NULL , sizeof( struct Shared ) , PROT_READ | PROT_WRITE , MAP_SHARED | MAP_ANONYMOUS , -1 , 0 );

	//initialization of semaphore variables
	for(i=0;i<3;i++){
		sem_init( & g_shm->smokerSem[i] , 1 , 0 );
		sem_init( & g_shm->pusherSem[i] , 1 , 0 );
		g_shm->itemsOnTable[i]=0;
		g_shm->itemsOnTable[i]=0;				//changed
	}
    sem_init( & g_shm->agentSem , 1 , 1 );
    sem_init( & g_shm->pusher_lock , 1 , 1 );

	printf("\n*********************** Process Started ***************************\n");
	srand(time(NULL));

	//	fork the requirde processes
	for(i=0;i<7;i++){
		pid=fork(); 
				
		if(pid==0){
			int x=i;
			i=7;
			if(x<3)
				pusher(&x);
			else if(x<6){
				x=x%3;
				smoker(&x);
			}else
				agent();
			exit(printf("exit value %d \n",x));
		}
	}
	return 0;
}

