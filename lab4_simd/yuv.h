#ifndef YUV_H
#define YUV_H

#include <stdint.h>
#include "rgb.h"

static const int16_t YUV_R[3] = {int16_t(0.164383 * (1 << 16)), 0, int16_t(0.096027 * (1 << 16))};
static const int16_t YUV_G[3] = {int16_t(0.164383 * (1 << 16)), int16_t(-0.391762 * (1 << 16)), int16_t(-0.312968 * (1 << 16))};
static const int16_t YUV_B[3] = {int16_t(0.164383 * (1 << 16)), int16_t(0.017232 * (1 << 16)), 0};

class YUV
{
public:
	uint8_t *Y8;
	uint8_t *U8;
	uint8_t *V8;
	int16_t *Y16;
	int16_t *U16;
	int16_t *V16;
	int ylength;
	int width;
	int height;
	YUV(int w, int h)
	{
		ylength = w * h;
		width = w;
		height = h;
		Y8 = new unsigned char[ylength];
		U8 = new unsigned char[ylength >> 2];
		V8 = new unsigned char[ylength >> 2];
		Y16 = new int16_t[ylength];
		U16 = new int16_t[ylength >> 2];
		V16 = new int16_t[ylength >> 2];
	}
	~YUV()
	{
		if(Y8)
			delete []Y8;
		if(U8)
			delete []U8;
		if(V8)
			delete []V8;
		if(Y16)
			delete []Y16;
		if(U16)
			delete []U16;
		if(V16)
			delete []V16;
	}
	int Read_YUV_File(const char* infile);
	int Write_YUV_File(const char* outfile);
	void u8_to_s16();
	void s16_to_u8();
	virtual void toRGB(RGB *dst_rgb) = 0;
};

class NonSimdYUV: public YUV
{
public:
	NonSimdYUV(int w, int h): YUV(w, h) {}
	~NonSimdYUV() {}
	void toRGB(RGB *dst_rgb);
};

class MmxYUV: public YUV 
{
public:
	MmxYUV(int w, int h): YUV(w, h) {}
	~MmxYUV() {}
	void toRGB(RGB *dst_rgb);
};

class SseYUV: public YUV 
{
public:
	SseYUV(int w, int h): YUV(w, h) {}
	~SseYUV() {}
	void toRGB(RGB *dst_rgb);
};

class AvxYUV: public YUV
{
public:
	AvxYUV(int w, int h): YUV(w, h) {}
	~AvxYUV() {}
	void toRGB(RGB *dst_rgb);
};

#endif
