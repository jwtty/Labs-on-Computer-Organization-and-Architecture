#include "cache.h"
#include "memory.h"
#include<iostream>
#include<fstream>
using namespace std;
Cache* L_list[100];
CacheConfig cc[100];
int lat_cyc[100];
Memory* Mem_p;
inline int get_lat(int sz)
{
	switch(sz)
	{
		case 0: return 4;
		case 3: return 5;
		case 8: return 11;
		default: return 0;
	}
	return -1;
}
int main(int argc, char* argv[]) 
{
	ios::sync_with_stdio(false);
	ifstream fin;
	fin.open(argv[1],ios::in);
	fin.seekg(fin.beg);
	int level=1;
	cout<<"Cache Simulator 0.1\nPlease input number of cache levels:\n";
	cin>>level;
	cout<<"Please input specs of each level in separate lines:\n";
	for(int i=1; i<=level; i++)
	{
		int bsize=0;
		cin>>bsize>>cc[i].associativity>>cc[i].block_size>>cc[i].write_through;
		lat_cyc[i]=get_lat(bsize);
		cc[i].size=1<<(15+bsize);
		cc[i].set_num=cc[i].size/(cc[i].associativity * cc[i].block_size);
		cc[i].write_allocate = 1 - cc[i].write_through;
		cc[i].block_bit=0; cc[i].set_bit=0;
		bsize=cc[i].block_size;
		while(bsize>>=1) ++cc[i].block_bit;
		bsize=cc[i].set_num;
		while(bsize>>=1) ++cc[i].set_bit;
	}
	Mem_p = new Memory;
	L_list[level]=new Cache(cc[level],Mem_p,Mem_p,lat_cyc[level]);
	for(int i=level-1; i>=1;i--)
		L_list[i]=new Cache(cc[i],L_list[i+1],Mem_p,lat_cyc[i]);
	for(;;)
	{
		char t=0; uint64_t addr=0;
		fin>>t>>addr;
		if(t==0)break;
		L_list[1]->HandleRequest(addr, t=='w'?0:1);
	}
	int tot=0;
	for(int i=1;i<=level;i++)
	{
		cout<<"L"<<i<<" Cache\n";
		tot+=L_list[i]->print_res();
		cout<<endl;
	}
	cout<<"Memory\n";
	tot+=Mem_p->print_res();
	cout<<"\nTotal cycles:\t"<<tot<<endl;
	fin.close();
	return 0;
}
