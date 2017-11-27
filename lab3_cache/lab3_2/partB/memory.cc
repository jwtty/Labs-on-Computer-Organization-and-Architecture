#include "memory.h"

/*void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &cycle) {
}*/
void Memory::HandleRequest(uint64_t addr, int read, bool prefetchflag) {
	++stats_.access_counter;
	if(!prefetchflag)
		stats_.access_cycle += latency_.hit_latency;
}

