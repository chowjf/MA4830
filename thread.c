#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

  pthread_t tid[10];

void *wave_generator() {
  while(1){
    printf("Hello1 %d---%d\n", tid[0], pthread_self());
    sleep(1);
  }
}

void *wave_generator1() {
  while(1){
    printf("Hello2 %d\n", pthread_self());
    sleep(1);
  }
}

void *wave_generator2() {
  while(1){
    printf("Hello3 %d\n", pthread_self());
sleep(1);
  }
}

void signal_handler() {
  printf("%d\n", pthread_self());
  for(int t=2;t>=0;t--)  {
    if(t!=tid[0])
    {
      pthread_cancel(tid[t]);
      printf("%d\n", t);
    }
  }
  pthread_exit(NULL);
}

void signal_handler2(){
   pthread_cancel(tid[2]);
   pthread_create(&tid[2], NULL, &wave_generator2, NULL);
}

int main(void) {
  int rc;

  printf("%d\n", getpid());
  printf("%d\n", pthread_self());

  signal(SIGQUIT, signal_handler);
  signal(SIGINT, signal_handler2);
   rc = pthread_create(&tid[0], NULL, &wave_generator, NULL);
 rc = pthread_create(&tid[1], NULL, &wave_generator1, NULL);
 rc = pthread_create(&tid[2], NULL, &wave_generator2, NULL);
 // rc = pthread_create(&tid[3], NULL, &wave_generator2, NULL);
 // rc = pthread_create(&tid[4], NULL, &wave_generator2, NULL);
 pthread_exit(NULL);
 while(1);
}
