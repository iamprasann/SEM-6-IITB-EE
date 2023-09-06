#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

zem_t agentSem;
zem_t ingredients[3];

void *justprint(void *data)
{
  int thread_id = *((int *)data);
	
  for(int i=0; i < NUM_ITER; i++)
    { 
      zem_down(&zems[thread_id]);
      printf("This is thread %d\n", thread_id);
      zem_up(&zems[(thread_id+1)%3]);
    }
  return 0;
}

int main(int argc, char *argv[])
{
  

  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];

  
  for(int i =0; i < NUM_THREADS; i++)
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }
  
  for(int i =0; i < NUM_THREADS; i++)
    {
      pthread_join(mythreads[i], NULL);
    }
  
  return 0;
}

// Pusher for any time Tobacco is on the table of ingredients offered
pthread_cond_wait(a->tobacco, a->mutex);
pthread_mutex_lock(a->mutex);
  if(isPaper){
    isPaper=false; //reset the variable because we found a match
    pthread_cond_signal(a->match);
  }
  else if(isMatch){
    isMatch=false;
    pthread_cond_signal(a->paper);
  }
  else{
    isTobacco=true;
  }
pthread_mutex_unlock(a->mutex);

