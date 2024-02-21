# cigarette-smoker-problem

cigarette smoker synchronisation problem

Author: Laltu Sardar 

thereb are two code one named cs1420-prog1.c and another is cs1420-prog2.c 

****************************************
____________________________________________________________________________________
cs1420-prog1.c  :

This program is implemented considering each agent as a  process using fork() system call.

to compile give the following command---

gcc -pthread -Wall cs1420-prog1.c -o prog1.o

and run by giving the following command
./prog1.o

___________________________________________________________________________________
cs1420-prog2.c  :

This program is implemented considering each agent as a  thread.

to compile give the following command---

gcc -pthread -Wall cs1420-prog2.c -o prog2.o

and run by giving the following command
./prog2.o

give input 1 threee times whenever it want no of smoker.(cause explained bellow)

____________________________________________________________________________________________

____________________________________________________________________________________________|

according to the problem each should be in infinite loop, but for checking purpose prog1 is in finite loop initially.


there are three 4 kind of processes: agent, smoker with tobacco(smoker0), smoker with paper
(smoker1),smoker with match(smoker2)  

smoker with tobacco: has infinite supply of 

smoker with paper: has infinite supply paper

smoker with match: has infinite supply match

agent has infinite supply of tobacco, paper and match;

agent process keep 2 ingradients randomly and signal the smokers. 


for solution we neet three extra pusher processes to control that the apropriate smoker can get it.

------------------------------------------------------------------

prog2 in which each agent is  implemented as a thread is quite different. in this program, I have allowed no of each type of smoker process may be more than 1, so it takes no of smoker of each type. But nos of remain processes are same. 

NOTE: smoker i with tobacco -> indicates ith smoker with tobacco
------------------------------------------------------------------
