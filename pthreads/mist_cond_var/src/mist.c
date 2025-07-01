#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  unsigned int counter;
  unsigned int max;
  pthread_mutex_t mutex;
  pthread_cond_t cond_var;
} mist_t;

void mist_init(mist_t* mist, unsigned max) {
  mist->counter = 0;
  mist->max = max;
  pthread_mutex_init(&mist->mutex, NULL);
  pthread_cond_init(&mist->cond_var, NULL);
}

void mist_destroy(mist_t* mist) {
  mist->counter = 0;
  pthread_mutex_destroy(&mist->mutex);
  pthread_cond_destroy(&mist->cond_var);
}

void mistery(mist_t* mist) {
  pthread_mutex_lock(&mist->mutex);
  ++mist->counter;
  if (mist->counter < mist->max) {
    // Preferred: while ( pthread_cond_wait(...) != 0 ) /* empty */;
    pthread_cond_wait(&mist->cond_var, &mist->mutex);
  } else {
    mist->counter = 0;
    pthread_cond_broadcast(&mist->cond_var);
  }
  pthread_mutex_unlock(&mist->mutex);
}

static mist_t mist;

void* run(void* data) {
  fprintf(stderr, "%zu: before mist()\n", (size_t)data);
  sleep((unsigned)(size_t)data);
  mistery(&mist);
  fprintf(stderr, "%zu: after mist()\n", (size_t)data);
  return NULL;
}

int main() {
  mist_init(&mist, 3);

  pthread_t* workers = malloc(3 * sizeof(pthread_t));
  for (size_t index = 0; index < 3; ++index) {
    pthread_create(&workers[index], NULL, run, (void*)index);
  }

  for (size_t index = 0; index < 3; ++index) {
    pthread_join(workers[index], NULL);
  }

  mist_destroy(&mist);
  return 0;
}
