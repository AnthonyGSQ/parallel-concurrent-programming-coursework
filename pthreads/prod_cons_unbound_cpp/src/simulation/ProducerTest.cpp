// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#include "ProducerTest.hpp"
#include "Log.hpp"
#include "Util.hpp"
// funcion constructora de producertest, inicializa todas las variables
// necesarias
ProducerTest::ProducerTest(size_t packageCount, int productorDelay
  , size_t consumerCount)
  : packageCount(packageCount)
  , productorDelay(productorDelay)
  , consumerCount(consumerCount) {
}
// funcion encargada de correr el llamado a la produccion de productos
int ProducerTest::run() {
  // Produce each asked message
  for (size_t index = 0; index < this->packageCount; ++index) {
    this->produce(this->createMessage(index));
  }

  // Produce an empty message to communicate we finished
  this->produce(NetworkMessage());

  // Reporte de la produccion 
  Log::append(Log::INFO, "Producer", std::to_string(this->packageCount)
    + " messages sent");
  return EXIT_SUCCESS;
}
// funcion encargada de asignar los parametros de cada producto
// es decir, la fuente de donde viene, el destino, bien sea consumidor
// uno, dos, tres ... etc. 
NetworkMessage ProducerTest::createMessage(size_t index) const {
  // Source is always 1: this producer
  const uint16_t source = 1;
  // Target is selected by random
  const uint16_t target = 1 + Util::random(0
    , static_cast<int>(this->consumerCount));
  // IMPORTANT: This simulation uses sleep() to mimics the process of
  // producing a message. However, you must NEVER use sleep() for real projects
  Util::sleepFor(this->productorDelay);
  // Create and return a copy of the network message
  return NetworkMessage(target, source, index);
}
