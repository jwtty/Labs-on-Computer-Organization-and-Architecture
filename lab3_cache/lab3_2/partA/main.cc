#include "cache.h"
#include "memory.h"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;
Cache* L_list[5];
CacheConfig cc[5];
int hit_cyc[5];
int bus_cyc[5];
ifstream fref;
int get_lat(int s, int as)
{
	fref.seekg(fref.beg);
	for(;;)
	{
		int ts = -1, tas = 0;
		char co = 0;
		double tlat=0.0;
		fref>>ts;
		if(ts==-1) break;
		fref>>co>>tas>>co>>tlat;
		if(ts<s)continue;
		if(tas<as)continue;
		return ceil(tlat/0.5);
	}
	return -1;
}
Memory* Mem_p;
int main(int argc, char* argv[]) 
{
	if(argc<2)
	{
		cout<<"Oops! No trace file!\n";
		return 0;
	}
	ios::sync_with_stdio(false);
	ifstream fin;
	fin.open(argv[1],ios::in);
	fin.seekg(fin.beg);
	fref.open("out.csv",ios::in);
	fref.seekg(fref.beg);
	ofstream fout;
	fout.open("res.csv",ios::out|ios::app);
	int level= -1;
	//cout<<"Cache Simulator 1.0\n";
	cin>>level;
	//cout<<"Please input specs:\n";
	for(int i=1; i<=level; i++)
	{
		//cout<<"Cache Level L"<<i<<":\n";
		int bsize=0;
		//cout<<"Please input c; Cache size is (2^(15+c))\n";
		cin>>bsize;
		//cout<<"Please input associativity\n";
		cin>>cc[i].associativity;
		//cout<<"Please input block size\n";
		//cin>>cc[i].block_size;
		cc[i].block_size=64;
		//cout<<"Please input 0 for write-back or 1 for write-through\n";
		cc[i].write_through=0;
		//cin>>cc[i].write_through;
		//cout<<"Please input bus latency for the cache\n";
		bus_cyc[i]=3*(i-1);
		hit_cyc[i]=get_lat(bsize,cc[i].associativity);
		cc[i].size=1<<(15+bsize);
		cc[i].set_num=cc[i].size/(cc[i].associativity * cc[i].block_size);
		if(cc[i].set_num<1)
		{
			cout<<"Oops! Wrong specs!\n";
			exit(0);
		}
		cc[i].write_allocate = 1 - cc[i].write_through;
		cc[i].block_bit=0; cc[i].set_bit=0;
		bsize=cc[i].block_size;
		while(bsize>>=1) ++cc[i].block_bit;
		bsize=cc[i].set_num;
		while(bsize>>=1) ++cc[i].set_bit;
	}
	Mem_p = new Memory;
	L_list[level]=new Cache(cc[level],Mem_p,Mem_p,hit_cyc[level],bus_cyc[level]);
	for(int i=level-1; i>=1;i--)
		L_list[i]=new Cache(cc[i],L_list[i+1],Mem_p,hit_cyc[i],bus_cyc[i]);
	for(;;)
	{
		char t=0; uint64_t addr=0;
		fin>>dec>>t>>hex>>addr;
		if(t==0)break;
		L_list[1]->HandleRequest(addr, t=='w'?0:1);
	}
	int tot=0;
	//cout<<endl;
	for(int i=1;i<=level;i++)
	{
	//	cout<<"L"<<i<<" Cache\n";
		tot+=L_list[i]->print_res();
	//	cout<<endl;
	}
	//cout<<"Memory\n";
	tot+=Mem_p->print_res();
	cout<<tot<<endl;
	fin.close();
	fref.close();
	fout.close();
	return 0;
}
