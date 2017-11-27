#ifndef CACHE_STORAGE_H_
#define CACHE_STORAGE_H_

#include <stdint.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

// Storage access stats
typedef struct StorageStats_ {
  int access_counter;
  int miss_num;
  int access_cycle; // In nanoseconds
  int replace_num; // Evict old lines
  int fetch_num; // Fetch lower layer
  int prefetch_num; // Prefetch
} StorageStats;

// Storage basic config
typedef struct StorageLatency_ {
  int hit_latency; // In cycles
  int bus_latency; // Added to each request
} StorageLatency;

class Storage {
 public:
  Storage() {}
  ~Storage() {}

  // Sets & Gets
  void SetStats(StorageStats ss) { stats_ = ss; }
  void GetStats(StorageStats &ss) { ss = stats_; }
  void SetLatency(StorageLatency sl) { latency_ = sl; }
  void GetLatency(StorageLatency &sl) { sl = latency_; }
  int print_res()
  {
	//cout<<"hit_cycle:\t"<<latency_.hit_latency<<"\nbus_latency:\t"<<latency_.bus_latency<<endl;
	//cout<<"access_counter:\t"<<stats_.access_counter<<"\nmiss_num:\t"<<stats_.miss_num<<endl;
	//cout<<"miss_rate:\t"<<((double)stats_.miss_num/(double)stats_.access_counter)*100<<"%\n";
	//cout<<"access_cycle:\t"<<stats_.access_cycle<<"\nreplace_num:\t"<<stats_.replace_num<<"\nfetch_num:\t"<<stats_.fetch_num<<endl;
	return stats_.access_cycle;
  }

  // Main access process
  // [in]  addr: access address
  // [in]  bytes: target number of bytes
  // [in]  read: 0|1 for write|read
  //             3|4 for prefetch write|read
  // [i|o] content: in|out data
  // [out] hit: 0|1 for miss|hit
  // [out] cycle: total access cycle
 /* virtual void HandleRequest(uint64_t addr, int bytes, int read,
                             char *content, int &hit, int &cycle) = 0;**/

  virtual void HandleRequest(uint64_t addr, int read) = 0;

 protected:
  StorageStats stats_;
  StorageLatency latency_;
};
#endif //CACHE_STORAGE_H_ 
