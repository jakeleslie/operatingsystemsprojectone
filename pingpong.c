#include <pthread.h>
#include <stdio.h>

// int pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *cattr);
// int	pthread_cond_wait(pthread_cond_t *cv,pthread_mutex_t *mutex);

pthread_mutex_t mutex;
pthread_cond_t cv;  

volatile int check = 0; //cap this somewher maybe

//pthread_condattr_t cattr; -> not sure if required 

//three operatings: wait, broadcast, and signal -> not sure if required 
//int ret; -> not sure if required 

// ret = pthread_cond_init(&cv, NULL); -> not sure if required + does not work

// ret = pthread_cond_init(&cv, &cattr); -> not sure if required  + does not work

//wait on a condition variable, probably what i want to print the stuff i need
//ret = pthread_cond_wait(&cv, &mp); -> not sure if required 

void *ping_func(void *ptr) {
  
  for (int i = 0; i < 10000; i++) { 

    pthread_mutex_lock(&mutex); //lock each loop. this is good
   
    while(check % 2 == 1){ //great condition
      pthread_cond_wait(&cv, &mutex);    
    }
    printf("ping\n"); 
    check++; //add to check to go to the next
    pthread_cond_signal(&cv); //then signal 
    pthread_mutex_unlock(&mutex); //then unlock it. this is good
    //pthread_cond_wait(&cv, &mutex);
    }
    return NULL;
  }

//do not think pong is actually getting signaled if i start
//it at 1, it should start because it is odd 
void *pong_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {
  
  pthread_mutex_lock(&mutex); //good here
  
   while(check % 2 == 0){ //great condition
    pthread_cond_wait(&cv, &mutex); //in the right spot 
   }
    printf("pong\n");
    check++;
    pthread_cond_signal(&cv); //then signal 
    pthread_mutex_unlock(&mutex); //good here
  }
  return NULL;
}

int main() {
  pthread_t thread1, thread2;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cv, NULL);

  pthread_create(&thread1, NULL, *ping_func, NULL); 
  pthread_create(&thread2, NULL, *pong_func, NULL); 

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}


//to do re-read the documentation, and review the code. think i need to initialize the wait thingy.
//and move around my lock and unlocks and the if statements