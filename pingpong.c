// Jake Leslie Operating Systems 29 September 2021
#include <pthread.h>
#include <stdio.h>


pthread_mutex_t mutex; //declaring our pthread variables, need mutex and a conditonal variable so that we can have conditions for locking and unlocking.
pthread_cond_t cv;  

int check = 0; //this is the global variable that will check to see if a number is even or odd.

void *ping_func(void *ptr) {
  
  for (int i = 0; i < 10000; i++) { 

    pthread_mutex_lock(&mutex); //Lock at the beginning so that we can focus on this thread.
   
    while(check % 2 == 1){ //Checks to see if even or odd, if our result is what we are not looking for, we unlock because it does not fit what we need.
      pthread_cond_wait(&cv, &mutex);  //Waiting, and going to the other thread because we do not print under this condition.
    }
    printf("ping\n");  //if the condition does not go through, we print ping. 
    check++; //add to check so the next number will be even or odd
    pthread_cond_signal(&cv); //Signaling the other thread that it is its time to shine.
    pthread_mutex_unlock(&mutex); //Now unlocking, so that the other thread can run.
    }
    return NULL;
  }

void *pong_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {

  pthread_mutex_lock(&mutex); //Same as before, we lock at the beginning so this thread is being focused on.
  
   while(check % 2 == 0){ //If our remainder is 0, we wait 
    pthread_cond_wait(&cv, &mutex); //Waiting, we do not print out under this condition.
   }
    printf("pong\n"); 
    check++;
    pthread_cond_signal(&cv); //signaling other thread
    pthread_mutex_unlock(&mutex); //Unlocking because we are done with this thread.
  }
  return NULL;
}

int main() {
  pthread_t thread1, thread2;
  
  pthread_mutex_init(&mutex, NULL); //Initiailizing our mutex, and our coniditional variable.
  pthread_cond_init(&cv, NULL);

  pthread_create(&thread1, NULL, *ping_func, NULL); 
  pthread_create(&thread2, NULL, *pong_func, NULL); 

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}


