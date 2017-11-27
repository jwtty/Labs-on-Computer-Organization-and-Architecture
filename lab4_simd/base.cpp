#include "yuv.h"
#include "rgb.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;

int YUV::Read_YUV_File(const char* infile)
{
	ifstream ifs(infile, ios::binary);
	if(!ifs)
	{
		cout << "Can not open the infile!" << endl;
		return -1;
	}
	int t = ifs.read((char*)Y8, ylength);
	t = ifs.read((char*)U8, ylength >> 2);
	t = ifs.read((char*)V8, ylength >> 2);
	ifs.close();
	return 0;
}

int YUV::Write_YUV_File(const char* outfile)
{
	ofstream ofs(outfile, ios::binary|ios::app);
	if(!ofs)
	{
		cout << "Can not open the outfile!" << endl;
		return -1;
	}
	ofs.seekp(0, ios::end);
	ofs.write((char*)Y8, ylength);
	ofs.write((char*)U8, ylength >> 2);
	ofs.write((char*)V8, ylength >> 2);
	ofs.close();
	/*struct stat buf;
	stat(outfile, &buf);
	cout << buf.st_size << endl;*/
	return 0;
}

inline uint8_t Chk(int16_t a)
{
	if(a > 255)
		return (uint8_t)255;
	if(a < 0)
		return (uint8_t)0;
	return (uint8_t)a;
}

void YUV::u8_to_s16()
{
	for (int i = 0; i < ylength; ++i)
	{
		Y16[i] = (int16_t)Y8[i];
	}
	for (int i = 0; i < (ylength >> 2); ++i)
	{
		U16[i] = (int16_t)U8[i];
		V16[i] = (int16_t)V8[i];
	}
}

void RGB::BoundCheck()
{
	for (int i = 0; i < length; ++i)
	{
		R16[i] = Chk(R16[i]);
		G16[i] = Chk(G16[i]);
		B16[i] = Chk(B16[i]);
	}
}

void YUV::s16_to_u8()
{
	for (int i = 0; i < ylength; ++i)
	{
		Y8[i] = Chk(Y16[i]);
	}
	for (int i = 0; i < (ylength >> 2); ++i)
	{
		U8[i] = Chk(U16[i]);
		V8[i] = Chk(V16[i]);
	}
}

void RGB::s16_to_u8()
{
	for (int i = 0; i < length; ++i)
	{
		R8[i] = Chk(R16[i]);
		G8[i] = Chk(G16[i]);
		B8[i] = Chk(B16[i]);
	}
}