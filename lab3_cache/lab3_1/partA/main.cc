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
	if(argc<2)
	{
		cout<<"Oops! No trace file!\n";
		return 0;
	}
	ios::sync_with_stdio(false);
	ifstream fin;
	fin.open(argv[1],ios::in);
	fin.seekg(fin.beg);
	int level=1;
	cout<<"Cache Simulator 1.0\n";
	//cin>>level;
	cout<<"Please input specs:\n";
	for(int i=1; i<=level; i++)
	{
		cout<<"Cache Level L"<<i<<":\n";
		int bsize=0;
		cout<<"Please input c; Cache size is (2^(15+c))\n";
		cin>>bsize;
		cout<<"Please input associativity\n";
		cin>>cc[i].associativity;
		cout<<"Please input block size\n";
		cin>>cc[i].block_size;
		cout<<"Please input 0 for write-back or 1 for write-through\n";
		cin>>cc[i].write_through;
		lat_cyc[i]=get_lat(bsize);
		cc[i].size=1<<(15+bsize);
		cc[i].set_num=cc[i].size/(cc[i].associativity * cc[i].block_size);
		if(cc[i].set_num<1)
		{
			cout<<"Oops! Wrong specs!\n";
			return 0;
		}
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
	cout<<endl;
	for(int i=1;i<=level;i++)
	{
		cout<<"L"<<i<<" Cache\n";
		tot+=L_list[i]->print_res();
		cout<<endl;
	}
	//cout<<"Memory\n";
	//tot+=Mem_p->print_res();
	//cout<<"\nTotal cycles:\t"<<tot<<endl;
	fin.close();
	return 0;
}
