#include "memory.h"

/*void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &cycle) {
}*/
void Memory::HandleRequest(uint64_t addr, int read) {
	++stats_.access_counter;
	stats_.access_cycle += latency_.hit_latency;
}

