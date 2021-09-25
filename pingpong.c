#include <pthread.h>
#include <stdio.h>

void *ping_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {
    printf("ping\n");
  }
  return NULL;
}

void *pong_func(void *ptr) {
  for (int i = 0; i < 10000; i++) {
    printf("pong\n");
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