#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
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
  bool operator < (const struct Line_ &x)
  {
      return timep < x.timep;
  }
} Line;

typedef struct Set_{
  Line *line_p;
} Set;

class Cache: public Storage {
 public:
  Cache(CacheConfig cc, Storage *ll, Memory *mm, int lat)
  {
	  latency_.hit_latency = lat;
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
		  }
	  }
  }
  ~Cache() 
  {
	  for (int i = 0; i < config_.set_num; i++)
		  delete[] set_p[i].line_p;
	  delete[] set_p;
  }
  // Sets & Gets
  void SetConfig(CacheConfig cc) { config_ = cc; }
  void GetConfig(CacheConfig &cc) { cc = config_; }
  void SetLower(Storage *ll) { lower = ll; }
  // Main access process
  /*void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &cycle);**/
  void HandleRequest(uint64_t addr, int read);
 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm(uint64_t addr, uint64_t &tag_inst, int &set_inst);
  // Replacement
  int ReplaceDecision(uint64_t tag_inst, int set_inst, int &victim);
  void ReplaceAlgorithm();
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm();

  CacheConfig config_;
  Storage *lower;
  Memory *mem_p;
  Set *set_p;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 
