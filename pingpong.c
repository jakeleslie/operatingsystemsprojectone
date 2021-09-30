#include <pthread.h>
#include <stdio.h>


pthread_mutex_t mutex;
pthread_cond_t cv;  

volatile int check = 0; 

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
    }
    return NULL;
  }

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


