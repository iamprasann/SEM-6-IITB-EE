#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

#define NUM_ITER 10

//global condition variables we will need, along with sum
int sum = 0;
pthread_cond_t m_p;
pthread_cond_t p_t;
pthread_cond_t t_m;

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

// To create the agent, constructor
struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = PTHREAD_MUTEX_INITIALIZER;
  agent->paper   = PTHREAD_COND_INITIALIZER;
  agent->match   = PTHREAD_COND_INITIALIZER;
  agent->tobacco = PTHREAD_COND_INITIALIZER;
  agent->smoke   = PTHREAD_COND_INITIALIZER;
  return agent;
}

// I have taken this from available assignment codes

enum Resource {MATCH = 1, PAPER = 2, TOBACCO = 4}; 
char* resource_name[] = {"", "match", "paper", "", "tobacco"};

int signal_count[5];  // # of times resource signalled
int smoke_count [5];  // # of times smoker with resource smoked
//Here we need 5 so that TOBACCO=4 has an index to map to

/*
This is the agent procedure. We are not allowed to change this 
*/

void* agent (void* av) {
  struct Agent* a = av;

  //binary sums ((1, 2, 4) are completely binary) can be taken by ORing the summands
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO, PAPER, MATCH};
  
  pthread_mutex_lock(a->mutex);
    for(int i=0; i<NUM_ITERATIONS; i++){
      int r = random()%3;
      signal_count[matching_smoker[r]] ++;
      int c = choices [r];

      //Below we AND because of the binary logic described earlier
      if (c & MATCH) {
        printf("match available\n");
        pthread_cond_signal(a->match);
      }
      if (c & PAPER) {
        printf("paper available\n");
        pthread_cond_signal(a->paper);
      }
      if (c & TOBACCO) {
        printf("tobacco available\n");
        pthread_cond_signal(a->tobacco);
      }
      printf("agent is waiting for smoker to smoke\n");
      pthread_cond_wait(a->smoke, a->mutex);
    }
  pthread_mutex_unlock(a->mutex);
  return NULL;
}


void call_smoker(int s){
  switch(s){
    case MATCH+PAPER :
      printf("Call Tobacco smoker.\n");
      pthread_cond_signal(m_p);
      sum = 0;
      break;
    case PAPER+TOBACCO:
      printf("Call Match smoker.\n");
      pthread_cond_signal(p_t);
      sum = 0;
      break;
    case MATCH+TOBACCO:
      printf("Call Paper smoker.\n");
      pthread_cond_signal(t_m);
      sum = 0;
      break;
    default:
      //pass
      break;
  }
}

// Listens to agent for available ingredient news
void* tobacco_listener(void* av){
  struct Agent* a = av;
  // We need to lock to carry this part out to avoid sleep and no wakeup case
  pthread_mutex_lock(a->mutex); 
  while(1){
    pthread_cond_wait(a->tobacco, a->mutex);
    sum=sum+TOBACCO;
    call_smoker(sum);
  }
  pthread_mutex_unlock(a->mutex);
}
void* paper_listener(void* av){
  struct Agent* a = av;
  pthread_mutex_lock(a->mutex);
  while(1){
    pthread_cond_wait(a->paper, a->mutex);
    sum=sum+PAPER;
    call_smoker(sum);
  }
  pthread_mutex_unlock(a->mutex);
}
void* match_listener(void* av){
  struct Agent* a = av;
  pthread_mutex_lock(a->mutex);
  while(1){
    pthread_cond_wait(a->match, a->mutex);
    sum=sum+MATCH;
    call_smoker(sum);
  }
  pthread_mutex_unlock(a->mutex);
}

// Signals to correct smoker
void* tobacco_smoker (void* av){
  struct Agent* a = av;
  pthread_mutex_lock(a->mutex);
  while(1){
    pthread_cond_wait(m_p, a->mutex);
    printf("Tobacco smoker is smoking.\n");
    pthread_cond_signal(a->smoke);
    smoke_count [TOBACCO]++;
  }
  pthread_mutex_unlock(a->mutex);
}
void* match_smoker (void* av){
  struct Agent* a = av;
  pthread_mutex_lock(a->mutex);
  while(1){
    pthread_cond_wait(p_t, a->mutex);
    printf("Match smoker is smoking.\n");
    pthread_cond_signal(a->smoke);
    smoke_count [MATCH]++;
  }
  pthread_mutex_unlock(a->mutex);
}
void* paper_smoker (void* av){
  struct Agent* a = av;
  pthread_mutex_lock(a->mutex);
  while(1){
    pthread_cond_wait(t_m, a->mutex);
    printf("Paper smoker is smoking.\n");
    pthread_cond_signal(a->smoke);
    smoke_count [PAPER]++;
  }
  pthread_mutex_unlock(a->mutex);
}

//Main function to test everything
int main (int argc, char** argv) {
  // Initialize objects
  struct Agent*  a = createAgent();
  m_p = PTHREAD_COND_INITIALIZER;
  p_t = PTHREAD_COND_INITIALIZER;
  t_m = PTHREAD_COND_INITIALIZER;

  // Declare threads
  pthread_t mythreads[7];

  // Initialize threads
  pthread_create(&mythreads[1], NULL, tobacco_listener, a);
  pthread_create(&mythreads[2], NULL, paper_listener, a);
  pthread_create(&mythreads[3], NULL, match_listener, a);
  pthread_create(&mythreads[4], NULL, tobacco_smoker, a);
  pthread_create(&mythreads[5], NULL, match_smoker, a);
  pthread_create(&mythreads[6], NULL, paper_smoker, a);

  pthread_join(pthread_create(&mythreads[0], NULL, agent, a), NULL);

  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count[MATCH], smoke_count[PAPER], smoke_count[TOBACCO]);
  printf("Smoke signals: %d matches, %d paper, %d tobacco\n",
          signal_count[MATCH], signal_count[PAPER], signal_count[TOBACCO])
  printf("Sum of counts: %d", smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO])
  printf("Sum of signals: %d", signal_count[MATCH] + signal_count[PAPER] + signal_count[TOBACCO])
}