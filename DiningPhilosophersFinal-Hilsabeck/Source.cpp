// Win 32 app for the Dining Philosophers
// Shell by S. Renk  2/4/99

#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>

using namespace std;

// shared vars for the philos - you can add more
enum pState {GONE, PRESENT, THINKING, HUNGRY, EATING, TALKING};
int philoCount = 0;                    // # of active philosophers
int forks[5] ={1,1,1,1,1};             // forks on table  1=present 0=gone
int usedForkLast[5] ={-1,-1,-1,-1,-1}; //No one has used them last
pState philoState[5]={GONE};           // 1-present 2-thinking 3-eating
int eatCount[5]= {0};                  // holds the # of time a philo has eaten
clock_t startTime, endTime;			   // used for run time calculation

// locks to create mutual exclusion
HANDLE mutex=CreateMutex(NULL, FALSE, NULL);
HANDLE forkLock[5];

// ********** Create 5 philosophers & set them to eating ****************
void main()
{ // Set up 5 threads
  HANDLE pThread[5];                  // 5 philosopher tbreads
  DWORD pThreadID[5];                 // PID of thread
  DWORD WINAPI philosopher(LPVOID);   // code for the 5 philos

  cout << "Dining Philosophers - R. Hilsabeck\n\n";
  startTime=clock();				  //start the timer

  // start 5 philosopher threads
  for(int philoNbr = 0; philoNbr < 5; philoNbr++)
  {   pThread[philoNbr]=CreateThread(NULL,0,philosopher,NULL,0,&pThreadID[philoNbr]);
	  forkLock[philoNbr] = CreateMutex(NULL, FALSE, NULL);
  }

  WaitForMultipleObjects(5, pThread, true, INFINITE); // wait for philos to finish
  endTime=clock();						//stop the timer
  cout << "\n\nRun time = " << ((endTime-startTime)*1000)/CLOCKS_PER_SEC << "ms\n";
  cout << "press CR to end."; while (_getch()!='\r');
  return;
}

//This function will be the test for a philosopher to see if they can eat or not. It will have a mutex around it that will make it atomic.
int getReadyToEat(int me, int left, int right, int leftFork, int rightFork)
{
  if(forks[leftFork] == 0 || forks[rightFork] == 0)
	return 0;
  if(philoState[right] == HUNGRY && usedForkLast[rightFork] == me)
    return 0;
  if(philoState[left] == HUNGRY &&  usedForkLast[leftFork] == me)
	return 0;
  else
	return 1;
}


// *************** The Philosopher **************************
DWORD WINAPI philosopher(LPVOID)
{  int me;   // holds my philo #
   int left, right; //holds neighbors
   int leftFork, rightFork; //holds forks
   int ableToEat; // boolean to see if you can eat or not
   int firstLock, secondLock;

   // who am I?
   WaitForSingleObject(mutex, INFINITE);   // lock the lock
   me = philoCount++;
   ReleaseMutex(mutex);                    // unlock the lock
   philoState[me] = PRESENT;
   left = (me+4)%5; right= (me+1)%5;
   leftFork = me; rightFork = (me+1)%5;

   //The below will create variable on what fork is to be locked first based on if they are an even numbered philosopher or an odd
   //numbered philosopher
   if(me%2)
   {
     firstLock = leftFork;
	 secondLock = rightFork;
   }
   else
   {
	 firstLock = rightFork;
	 secondLock = leftFork;
   }

   // wait for everybody to show up to dinner
   while (philoCount < 5) Sleep(0);           // preempt self till everybody gets here

   // Is the print out jumbled?  - why?
   WaitForSingleObject(mutex,INFINITE);
   cout << "Philosopher # " << me << " ready to dine." << endl;
   ReleaseMutex(mutex);
   
   while(eatCount[me] < 100) // eat 100 times
   { // think for awhile
     philoState[me] = THINKING;
	 
     Sleep((DWORD)rand()%20);

	 philoState[me] = HUNGRY;


   // if (you can get forks) eat for awhile
	ableToEat = 0;  //boolean to keep track if you can eat or not 0-can't eat 1-can eat

   //this while loop will check for certain conditions where the philosopher can eat or not eat
   while(ableToEat == 0)
   {
	 WaitForSingleObject(mutex,INFINITE);
	 ableToEat = getReadyToEat(me, left,right,leftFork,rightFork);
	 ReleaseMutex(mutex);	 
   }

   // eating
   //Lock the forks you need to use for eating

   WaitForSingleObject(forkLock[firstLock], INFINITE);
   WaitForSingleObject(forkLock[secondLock], INFINITE);

   forks[rightFork]--; forks[leftFork]--;

   philoState[me] = EATING;

   if (philoState[left] == EATING || philoState[right]==EATING)
   {
     WaitForSingleObject(mutex, INFINITE);
     cout << "******** Eating Error ********" << '\7' <<endl;
     ReleaseMutex(mutex);
   }

   if (forks[rightFork] || forks[leftFork])
   {
     WaitForSingleObject(mutex, INFINITE);
	 cout << "******** Fork Error ********" << '\7' <<endl;
	 ReleaseMutex(mutex);
   }

   eatCount[me]++;
   Sleep((DWORD)rand()%20);
   // return forks
   forks[rightFork]++; forks[leftFork]++;
   usedForkLast[leftFork] = me;
   usedForkLast[rightFork] = me;
   philoState[me] = TALKING;
   ReleaseMutex(forkLock[firstLock]);
   ReleaseMutex(forkLock[secondLock]);	
	  
   Sleep(rand()%10);
   }

   WaitForSingleObject(mutex, INFINITE);   // lock the lock
   cout << "Philosopher # " << me << " is leaving now." << endl;
   for(int i=0;i<5;cout << eatCount[i++] << " "); cout << endl;
   ReleaseMutex(mutex);                    // unlock the lock
   philoState[me] = GONE;
   return 0;
}