#include "cache.h"
#include "memory.h"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;
Cache* L_list[5];

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
	CacheConfig c1,c2;
	c1.associativity=c2.associativity=8;
	c1.block_size=c2.block_size=64;
	c1.write_through=c2.write_through=0;
	c1.write_allocate=c2.write_allocate=1;
	c1.size=32768;	c2.size=262144;
	c1.set_num=64;	c2.set_num=512;
	c1.block_bit=c2.block_bit=6;
	c1.set_bit=6;	c2.set_bit=9;
	Mem_p = new Memory;
	/*
		5:	hit_cycle
		6:	bus_cycle
		10:	bypassing decision: log(number of parts)
		0.15: bypassing decision: cutoff
	**/
	L_list[2]=new Cache(c2,Mem_p,Mem_p,5,6,10,0.5);
	L_list[1]=new Cache(c1,L_list[2],Mem_p,4,0,-1,0.0);
	for(;;)
	{
		char t=0; uint64_t addr=0;
		//fin>>t>>addr;
		fin>>dec>>t>>hex>>addr;
		if(t==0)break;
		L_list[1]->HandleRequest(addr, t=='w'?0:1, 0);
	}
	int tot=0;
	for(int i=1;i<=2;i++)
	{
		//cout<<"L"<<i<<" Cache\n";
		tot+=L_list[i]->print_res();
	//	cout<<endl;
	}
	//cout<<"Memory\n";
	tot+=Mem_p->print_res();
	cout<<tot<<endl;
	fin.close();
	return 0;
}
