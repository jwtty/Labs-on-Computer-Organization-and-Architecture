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
void Cache::HandleRequest(uint64_t addr, int read, bool prefetchflag) {
	++stats_.access_counter;
	if(!prefetchflag)
		stats_.access_cycle += latency_.bus_latency;
	uint64_t tag_inst;
	int set_inst;
	int victim;
    // Decide on whether a bypass should take place.
    if (!BypassDecision(addr)) {
    	// Generate some information for the data partition.
    	PartitionAlgorithm(addr, tag_inst, set_inst);
    	if(!prefetchflag)
			stats_.access_cycle += latency_.hit_latency;
    	// Check whether the required data already exist.
    	if (ReplaceDecision(tag_inst, set_inst, victim)) {
    		set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    		if(read == 0 && config_.write_through == 0)
    			set_p[set_inst].line_p[victim].dirty = 1;
    		else if(read == 0 && config_.write_through == 1)
    			lower -> HandleRequest(addr, 0, prefetchflag);
			set_p[set_inst].line_p[victim].pro = 1;
      		if (PrefetchDecision(prefetchflag, set_p[set_inst].line_p[victim].prefetchtag)) {
    			PrefetchAlgorithm(tag_inst, set_inst);
    		}
    		return;
    	}
    	// Choose a victim for the current request.
    	//ReplaceAlgorithm();
    	++stats_.miss_num;
		BypassStat(addr);
		set_p[set_inst].line_p[victim].pro=0;
    	if(read == 1)
    	{
    		if(set_p[set_inst].line_p[victim].valid == 1) {
    			++stats_.replace_num;
    			if(set_p[set_inst].line_p[victim].dirty == 1) {
    				lower -> HandleRequest(set_p[set_inst].line_p[victim].tag | (set_inst << config_.block_bit), 0, prefetchflag);
    			}
    			set_p[set_inst].line_p[victim].dirty = 0;
    			set_p[set_inst].line_p[victim].tag = tag_inst;
    			set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    			set_p[set_inst].line_p[victim].prefetchtag = prefetchflag ? 1 : 0;
    		}
    		else {
    			set_p[set_inst].line_p[victim].valid = 1;
    			set_p[set_inst].line_p[victim].dirty = 0;
    			set_p[set_inst].line_p[victim].tag = tag_inst;
    			set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    			set_p[set_inst].line_p[victim].prefetchtag = prefetchflag ? 1 : 0;
    		}
    		lower -> HandleRequest(addr, 1, prefetchflag);
    		++stats_.fetch_num;
    	}
    	else if(read == 0)
    	{
    		if(config_.write_allocate == 0)
    			mem_p -> HandleRequest(addr, 0, prefetchflag);
    		else {
    			if(set_p[set_inst].line_p[victim].valid == 1) {
    				++stats_.replace_num;
    				if(set_p[set_inst].line_p[victim].dirty == 1) {
    					lower -> HandleRequest(set_p[set_inst].line_p[victim].tag | (set_inst << config_.block_bit), 0, prefetchflag);
    				}
    				set_p[set_inst].line_p[victim].dirty = 1;
    				set_p[set_inst].line_p[victim].tag = tag_inst;
    				set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    				set_p[set_inst].line_p[victim].prefetchtag = 0;
    			}
    			else {
    				set_p[set_inst].line_p[victim].valid = 1;
    				set_p[set_inst].line_p[victim].dirty = 1;
    				set_p[set_inst].line_p[victim].tag = tag_inst;
    				set_p[set_inst].line_p[victim].timep = stats_.access_counter;
    				set_p[set_inst].line_p[victim].prefetchtag = 0;
    			}
    			lower -> HandleRequest(addr, 0, prefetchflag);
    			++stats_.fetch_num;
    		}
    	}
  	}
	else
	{
		lower -> HandleRequest(addr, read, prefetchflag);
		++stats_.fetch_num;
	}
}

int Cache::BypassDecision(uint64_t addr)
{
	if(bp_div<0)
		return FALSE;
	unsigned int bp_gp=addr>>(48-bp_div);
	++bp_tot[bp_gp];
	if(bp_tot[bp_gp]<100)
		return FALSE;
	if((double)bp_miss[bp_gp]/(double)bp_tot[bp_gp]>bp_th)
		return TRUE;
	return FALSE;
}
void Cache::BypassStat(uint64_t addr)
{
	if(bp_div<0) return;
	unsigned int bp_gp=addr>>(48-bp_div);
	++bp_miss[bp_gp];
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
/*		if(set_p[set_inst].line_p[i].valid == 0)
			set_p[set_inst].line_p[i].timep = 0;	**/
		if(set_p[set_inst].line_p[i] < set_p[set_inst].line_p[victim])
			victim = i;
  	}
  	return FALSE;
}

void Cache::ReplaceAlgorithm(){
	
}

int Cache::PrefetchDecision(bool prefetchflag, int tag) {
	if(!prefetchflag && tag == 1)
  		return TRUE;
}

void Cache::PrefetchAlgorithm(uint64_t tag, int set) {
	for (int i = 1; i <= 3; ++i)
	{
		uint64_t addr = tag | ((set + i) << config_.block_bit);
		HandleRequest(addr, 1, 1);
	}
}

