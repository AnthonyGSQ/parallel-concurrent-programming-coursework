// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "Consumer.hpp"
#include "Producer.hpp"

/**
 * @brief An assembler is a worker of a assembly line. It receives incomplete
 * products, does some pre-defined work of them, and the resulting product
 * continues the assembly line, where other workers will add more parts. See
 * https://en.wikipedia.org/wiki/Assembly_line for the term assembler.
 * 
 * An assembler is both, a consumer of incomplete products coming from a
 * consuming queue, and a producer that pushes results to a queue. The data type
 * of coming products may be different of the products that the Assembler
 * produces.
 * 
 * This class is abstract. You should inherit a class from Assembler and
 * override the consume() method, that is called each time a product is
 * consumed, then do some work with the product, and finally call produce()
 * to push the resulting product to the assembly line.
 */
template <typename ConsumingType, typename ProducingType>
class Assembler
  : public Consumer<ConsumingType>
  , public Producer<ProducingType> {
  /// Objects of this class cannot be copied
  DISABLE_COPY(Assembler);

 public:
  /// Constructor del assembler (no utilizado en este ejercicio)
  explicit Assembler(Queue<ConsumingType>* consumingQueue = nullptr
    , Queue<ProducingType>* producingQueue = nullptr
    , const ConsumingType& stopCondition = ConsumingType())
    : Consumer<ConsumingType>(consumingQueue, stopCondition)
    , Producer<ProducingType>(producingQueue) {
  }

  /// Destructor
  virtual ~Assembler() {
  }
};

#endif  // ASSEMBLER_HPP
