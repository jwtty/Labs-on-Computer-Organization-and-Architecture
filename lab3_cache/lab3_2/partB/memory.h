#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"

class Memory: public Storage {
 public:
  Memory() {latency_.hit_latency=100; latency_.bus_latency=0;}
  ~Memory() {}

  // Main access process
  void HandleRequest(uint64_t addr, int read, bool prefetchflag);

 private:
  // Memory implement

  DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 
