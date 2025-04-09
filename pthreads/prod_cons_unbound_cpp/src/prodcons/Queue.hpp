// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <mutex>
#include <queue>

#include "common.hpp"
#include "Semaphore.hpp"

/**
 * @brief A thread-safe generic queue for consumer-producer pattern.
 *
 * @remark None of the methods of this class can be const because all
 * methods require lock the mutex to avoid race-conditions
 */
template <typename DataType>
class Queue {
  DISABLE_COPY(Queue);

 protected:
  /// mutex para hacer que no se den condiciones de carrera
  /// al evitar que se lean valores del queue con un hilo y se agreguen
  // , modifiquen valores del queue con otro hilo
  std::mutex mutex;
  /// Indicates if there are consumable elements in the queue
  Semaphore canConsume;
  /// Contains the actual data shared between producer and consumer
  std::queue<DataType> queue;

 public:
  /// Constructor
  Queue()
    : canConsume(0) {
  }

  /// Destructor
  ~Queue() {
    // TODO(jhc): clear()?
  }

  /// funcion para encolar productos producitos por los productores
  // avisando que existen productos para ser consumidos por los
  // consumidores
  void enqueue(const DataType& data) {
    this->mutex.lock();
    this->queue.push(data);
    this->mutex.unlock();
    this->canConsume.signal();
  }

  // funcion que borra x elemento de la cola al ser consumido
  // dicha cola se "detiene" en caso de encontrarse con cero
  // elementos para consumir
  DataType dequeue() {
    this->canConsume.wait();
    this->mutex.lock();
    DataType result = this->queue.front();
    this->queue.pop();
    this->mutex.unlock();
    return result;
  }
};

#endif  // QUEUE_HPP
