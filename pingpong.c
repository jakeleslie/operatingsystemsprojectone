#include <pthread.h>
#include <stdio.h>

//int pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *cattr);
//int	pthread_cond_wait(pthread_cond_t *cv,pthread_mutex_t *mutex);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int check = 0;
pthread_cond_t cv;
pthread_condattr_t cattr;

//three operatings: wait, broadcast, and signal
// int ret;

// ret = pthread_cond_init(&cv, NULL);

// ret = pthread_cond_init(&cv, &cattr);

//wait on a condition variable, probably what i want to print the stuff i need
//ret = pthread_cond_wait(&cv, &mp);

void *ping_func(void *ptr) {
  for (int i = 0; i < 10000; i++) { //go through the loop
    pthread_mutex_lock(&mutex); //should keep locking
    //if(check % 2 == 0){ //this condition should go through and print
    printf("ping\n");
    //}
    pthread_mutex_unlock(&mutex);
    //++check;
    pthread_cond_signal(&cv);
    }
    return NULL;
  }


void *pong_func(void *ptr) {
  pthread_mutex_lock(&mutex); //start this locked since i want ping to go first, or put inside loop?
  for (int i = 0; i < 10000; i++) {
  
   // if(check % 2 == 1){ //this condition should go through and then it should unlock the thread print its output and relock again
    printf("pong\n");
    pthread_cond_wait(&cv, &mutex); //in order to trigger we need a signal from something else
    //}
    //++check;
    pthread_mutex_unlock(&mutex);

  }
  return NULL;
}

int main() {
  pthread_t thread1, thread2;

  pthread_create(&thread1, NULL, *ping_func, NULL); 
  pthread_create(&thread2, NULL, *pong_func, NULL); 

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}
