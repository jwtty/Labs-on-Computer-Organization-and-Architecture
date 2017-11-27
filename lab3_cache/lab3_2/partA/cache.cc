#include "cache.h"
#include "def.h"

// Main access process
// [in]  addr: access address
// [in]  bytes: target number of bytes
// [in]  read: 0|1 for write|read
//             3|4 for write|read in prefetch
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
/*void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &cycle) {*/
void Cache::HandleRequest(uint64_t addr, int read) {
	++stats_.access_counter;
	stats_.access_cycle += latency_.bus_latency;
	uint64_t tag_inst;
	int set_inst;
	int victim;
  // Decide on whether a bypass should take place.
  if (!BypassDecision()) {

    // Generate some infomation for the data partition.
    PartitionAlgorithm(addr, tag_inst, set_inst);
	stats_.access_cycle += latency_.hit_latency;
    // Check whether the required data already exist.
    if (ReplaceDecision(tag_inst, set_inst, victim)) {
    	set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    //	stats_.access_cycle += latency_.hit_latency;
    	if(read == 0 && config_.write_through == 0)
    		set_p[set_inst].line_p[victim].dirty = 1;
    	else if(read == 0 && config_.write_through == 1)
    		lower -> HandleRequest(addr, 0);
      return;
    }
    // Choose a victim for the current request.
    //ReplaceAlgorithm();
    ++stats_.miss_num;
    if(read == 1)
    {
    	if(set_p[set_inst].line_p[victim].valid == 1) {
    		++stats_.replace_num;
    		if(set_p[set_inst].line_p[victim].dirty == 1) {
    			lower -> HandleRequest(set_p[set_inst].line_p[victim].tag | (set_inst << config_.block_bit), 0);
    		}
    		set_p[set_inst].line_p[victim].dirty = 0;
    		set_p[set_inst].line_p[victim].tag = tag_inst;
    		set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    	}
    	else {
    		set_p[set_inst].line_p[victim].valid = 1;
    		set_p[set_inst].line_p[victim].dirty = 0;
    		set_p[set_inst].line_p[victim].tag = tag_inst;
    		set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    	}
    	lower -> HandleRequest(addr, 1);
    	++stats_.fetch_num;
    }
    else if(read == 0)
    {
    	if(config_.write_allocate == 0)
    		mem_p -> HandleRequest(addr, 0);
    	else {
    		if(set_p[set_inst].line_p[victim].valid == 1) {
    			++stats_.replace_num;
    			if(set_p[set_inst].line_p[victim].dirty == 1) {
    				lower -> HandleRequest(set_p[set_inst].line_p[victim].tag | (set_inst << config_.block_bit), 0);
    			}
    			set_p[set_inst].line_p[victim].dirty = 1;
    			set_p[set_inst].line_p[victim].tag = tag_inst;
    			set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    		}
    		else {
    			set_p[set_inst].line_p[victim].valid = 1;
    			set_p[set_inst].line_p[victim].dirty = 1;
    			set_p[set_inst].line_p[victim].tag = tag_inst;
    			set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    		}
    		lower -> HandleRequest(addr, 0);
    		++stats_.fetch_num;
    	}
    }
    // Decide on whether a prefetch should take place.
    if (PrefetchDecision()) {
      // Fetch the other data via HandleRequest() recursively.
      // To distinguish from the normal requests,
      // the 2|3 is employed for prefetched write|read data
      // while the 0|1 for normal ones.
      PrefetchAlgorithm();
    }
  }
  // Fetch from the lower layer

  // If a victim is selected, replace it.
  // Something else should be done here
  // according to your replacement algorithm.
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm(uint64_t addr, uint64_t &tag_inst, int &set_inst) {
	tag_inst = addr & (mask << (config_.block_bit + config_.set_bit));
	set_inst = (addr & ~(mask << (config_.block_bit + config_.set_bit))) >> config_.block_bit;
}

int Cache::ReplaceDecision(uint64_t tag_inst, int set_inst, int &victim) {
	victim = 0;
	for (int i = 0; i < config_.associativity; ++i)
	{
		if(set_p[set_inst].line_p[i].valid == 1 && set_p[set_inst].line_p[i].tag == tag_inst) {
			victim = i;
  			return TRUE;
		}
		if(set_p[set_inst].line_p[i] < set_p[set_inst].line_p[victim])
			victim = i;
  	}
  	return FALSE;
}

void Cache::ReplaceAlgorithm(){
	
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

