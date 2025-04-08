// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <assert.h>
#include <stdlib.h>

#include "queue.h"

void queue_remove_first_unsafe(queue_t* queue);
bool queue_is_empty_unsafe(queue_t* queue);

int queue_init(queue_t* queue) {
  assert(queue);
  int error = pthread_mutex_init(&queue->can_access_queue, /*attr*/ NULL);
  queue->head = NULL;
  queue->tail = NULL;
  return error;
}

int queue_destroy(queue_t* queue) {
  queue_clear(queue);
  return pthread_mutex_destroy(&queue->can_access_queue);
}
// este queue_is_empty si es thread safe pues tiene un mutex
bool queue_is_empty(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  bool result = queue->head == NULL;
  pthread_mutex_unlock(&queue->can_access_queue);
  return result;
}
// este is empty no es thread safe ya que es una lectura que puede
// darse al mismo tiempo que se asigna un valo con otro thread
// para evitar condiciones de carrera debe de llamarse dentro de un mutex
bool queue_is_empty_unsafe(queue_t* queue) {
  assert(queue);
  return queue->head == NULL;
}

int queue_enqueue(queue_t* queue, const size_t data) {
  assert(queue);
  int error = EXIT_SUCCESS;

  queue_node_t* new_node = (queue_node_t*) calloc(1, sizeof(queue_node_t));
  if (new_node) {
    new_node->data = data;
    // bloqueamos para evitar que multiples hilos intenten insertar
    // nodos
    pthread_mutex_lock(&queue->can_access_queue);
    // si la cola no es NULL, es porque la cola tiene elementos y en
    // consecuencia el nuevo nodo agregado sera la nueva cola
    if (queue->tail) {
      queue->tail = queue->tail->next = new_node;
    // caso contrario, la cola esta vacia y por ende el nuevo nodo
    // creado sera la cabeza y cola.
    } else {
      queue->head = queue->tail = new_node;
    }
    pthread_mutex_unlock(&queue->can_access_queue);
  } else {
    error = EXIT_FAILURE;
  }

  return error;
}

int queue_dequeue(queue_t* queue, size_t* data) {
  assert(queue);
  int error = 0;
  // bloqueamos para evitar borrar colas ya vacias o borrar el mismo
  // elemento mas de una vez
  pthread_mutex_lock(&queue->can_access_queue);
  // si la cola no esta vacia, retorna el elemento y luego lo elimina
  if (!queue_is_empty_unsafe(queue)) {
    if (data) {
      *data = queue->head->data;
    }
    // borrar el head
    queue_remove_first_unsafe(queue);
  } else {
    error = EXIT_FAILURE;
  }
  pthread_mutex_unlock(&queue->can_access_queue);

  return error;
}
// funcion que borra el primer elemento de la cola (head)
// si dicha cola no esta vacia
void queue_remove_first_unsafe(queue_t* queue) {
  assert(queue);
  assert(!queue_is_empty_unsafe(queue));
  queue_node_t* node = queue->head;
  queue->head = queue->head->next;
  free(node);
  if (queue->head == NULL) {
    queue->tail = NULL;
  }
}
// funcion encargada de liberar todos los nodos del queue
void queue_clear(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  while (!queue_is_empty_unsafe(queue)) {
    queue_remove_first_unsafe(queue);
  }
  pthread_mutex_unlock(&queue->can_access_queue);
}
