#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int check = 0;

void *ping_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {
    pthread_mutex_lock(&mutex); //at beginning of for loop lock the thread. have to have a condition that happens where it causes it to unlock and prints its output. probably the pthread_cond_t

    if(check % 2 == 0){ //this condition should go through and then it should unlock the thread print its output and relock again
    pthread_mutex_unlock(&mutex);
    printf("ping\n");

    }
    ++check;
    pthread_mutex_unlock(&mutex);

    }
    return NULL;
  }


void *pong_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {
    pthread_mutex_lock(&mutex); //at beginning of for loop lock the thread. have to have a condition that happens where it causes it to unlock and prints its output. probably the pthread_cond_t
  
    if(check % 2 == 1){ //this condition should go through and then it should unlock the thread print its output and relock again
    pthread_mutex_unlock(&mutex);

    printf("pong\n");
    }
    ++check;
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

/* goal is to get our output to be
ping
pong
ping 
pong
...

need to use a cv and pthread_cond_t and a mutex to get this to work

*/

//what i could do is, each time one i added to the ping or pong lock it and unlock it