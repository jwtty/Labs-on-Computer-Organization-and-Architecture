#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <memory.h>
#include "storage.h"
#include "memory.h"

#define mask 0xffffffffffffffff

typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc

  int block_size;
  int block_bit;
  int set_bit;
} CacheConfig;

typedef struct Line_ {
  int dirty;
  int valid;
  uint64_t tag;
  int timep;
  int pro;
  int prefetchtag;
  bool operator < (const struct Line_ &x)
  {
	if(x.pro == pro)
		return timep < x.timep;
	else
		return pro == 0;
  }
} Line;

typedef struct Set_{
  Line *line_p;
} Set;

class Cache: public Storage {
 public:
  Cache(CacheConfig cc, Storage *ll, Memory *mm, int lat, int blat, int bpdv, double bpth)
  {
	  latency_.hit_latency = lat;
	  latency_.bus_latency = blat;
	  config_ = cc;	lower = ll; mem_p = mm;
	  set_p=new Set[cc.set_num];
	  for (int i = 0; i < cc.set_num; i++)
	  {
		  set_p[i].line_p = new Line[cc.associativity];
		  for (int j = 0; j < cc.associativity; j++)
		  {
			  set_p[i].line_p[j].dirty = 0;
			  set_p[i].line_p[j].tag = 0;
			  set_p[i].line_p[j].timep = 0;
			  set_p[i].line_p[j].valid = 0;
			  set_p[i].line_p[j].pro = 0;
			  set_p[i].line_p[j].prefetchtag = 0;
		  }
	  }
	  bp_div=bpdv;
	  if(bpdv<0)
		bp_miss=bp_tot=NULL;
	  else
	  {
		bp_miss=new int[1<<bpdv];
		memset(bp_miss, 0, sizeof(bp_miss));
		bp_tot=new int[1<<bpdv];
    	// Generate some information for the data partition.
		memset(bp_tot, 0, sizeof(bp_tot));
		bp_th=bpth;
	  }
  }
  ~Cache() 
  {
	  for (int i = 0; i < config_.set_num; i++)
		  delete[] set_p[i].line_p;
	  delete[] set_p;
	  delete[] bp_miss;
	  delete[] bp_tot;
  }
  // Sets & Gets
  void SetConfig(CacheConfig cc) { config_ = cc; }
  void GetConfig(CacheConfig &cc) { cc = config_; }
  void SetLower(Storage *ll) { lower = ll; }
  // Main access process
  /*void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &cycle);**/
  void HandleRequest(uint64_t addr, int read, bool prefetchflag);
 private:
  // Bypassing
  int BypassDecision(uint64_t addr);
  void BypassStat(uint64_t addr);
  // Partitioning
  void PartitionAlgorithm(uint64_t addr, uint64_t &tag_inst, int &set_inst);
  // Replacement
  int ReplaceDecision(uint64_t tag_inst, int set_inst, int &victim);
  void ReplaceAlgorithm();
  // Prefetching
  int PrefetchDecision(bool prefetchflag, int tag);
  void PrefetchAlgorithm(uint64_t tag, int set);

  CacheConfig config_;
  Storage *lower;
  Memory *mem_p;
  Set *set_p;
  DISALLOW_COPY_AND_ASSIGN(Cache);
  int bp_div;
  int *bp_miss;
  int *bp_tot;
  double bp_th;
};

#endif //CACHE_CACHE_H_ 
