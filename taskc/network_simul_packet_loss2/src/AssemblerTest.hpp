// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#pragma once

#include "Assembler.hpp"
#include "NetworkMessage.hpp"

/**
 * @brief A ConsumerTest class example
 */
class AssemblerTest : public Assembler<NetworkMessage, NetworkMessage> {
  DISABLE_COPY(AssemblerTest);

 protected:
  /// Delay of consumer to consume a package, negative for max random
  int consumerDelay = 0;
  /// Probability of loss a package
  const double packetLossProbability = 0.0;
  /// Number of consumer threads
  const size_t consumerCount = 0;
  /// Number of messages loss
  size_t lostMessages = 0;

 public:
  /// Constructor
  explicit AssemblerTest(const int consumerDelay,
    const double packetLossProbability, const size_t consumerCount);
  /// Consume the messages in its own execution thread
  int run() override;
  /// Override this method to process any data extracted from the queue
  void consume(NetworkMessage data) override;
};
