#ifndef RGB_H
#define RGB_H

#include <stdint.h>
#include "yuv.h"

static const int16_t RGB_Y[3] = { int16_t(0.256788 * (1 << 16)),  int16_t(0.004129 * (1 << 16)),  int16_t(0.097906 * (1 << 16)) }; 
static const int16_t RGB_U[3] = { int16_t(-0.148223 * (1 << 16)), int16_t(-0.290993 * (1 << 16)), int16_t(0.439216 * (1 << 16)) }; 
static const int16_t RGB_V[3] = { int16_t(0.439216 * (1 << 16)), int16_t(-0.367788 * (1 << 16)), int16_t(-0.071427 * (1 << 16)) }; 
class YUV;
class RGB
{
public:
	uint8_t *R8;
	uint8_t *G8;
	uint8_t *B8;
	int16_t *R16;
	int16_t *G16;
	int16_t *B16;
	int length;
	int width;
	int height;
	//char *filename;
	RGB(int w, int h) 
	{
		length = w * h;
		width = w;
		height = h;
		R8 = new uint8_t[length];
		G8 = new uint8_t[length];
		B8 = new uint8_t[length];
		R16 = new int16_t[length];
		G16 = new int16_t[length];
		B16 = new int16_t[length];
	}
	~RGB()
	{
		if(R8)
			delete []R8;
		if(G8)
			delete []G8;
		if(B8)
			delete []B8;
		if(R16)
			delete []R16;
		if(G16)
			delete []G16;
		if(B16)
			delete []B16;
	}
	void BoundCheck();
	void s16_to_u8();
	virtual void toYUV(YUV* dst_yuv) = 0;
	virtual	void alphaBlend(const uint8_t alpha, const RGB* src_rgb) = 0;
	virtual void overlay(const uint8_t alpha, const RGB* src_rgb1, const RGB* src_rgb2) = 0;
};


class NonSimdRGB: public RGB
{
public:
	NonSimdRGB(int w, int h): RGB(w, h) {}
	~NonSimdRGB() {}
	void toYUV(YUV* dst_yuv);
	void alphaBlend(const uint8_t alpha, const RGB* src_rgb);
	void overlay(const uint8_t alpha, const RGB* src_rgb1, const RGB* src_rgb2);
};

class MmxRGB: public RGB
{
public:
	MmxRGB(int w, int h): RGB(w, h) {}
	~MmxRGB() {}
	void toYUV(YUV* dst_yuv);
	void alphaBlend(const uint8_t alpha, const RGB* src_rgb);
	void overlay(const uint8_t alpha, const RGB* src_rgb1, const RGB* src_rgb2);
};

class SseRGB: public RGB //TODO
{
public:
	SseRGB(int w, int h): RGB(w, h) {}
	~SseRGB() {}
	void toYUV(YUV* dst_yuv);
	void alphaBlend(const uint8_t alpha, const RGB* src_rgb);
	void overlay(const uint8_t alpha, const RGB* src_rgb1, const RGB* src_rgb2);
};

class AvxRGB: public RGB //TODO
{
public:
	AvxRGB(int w, int h): RGB(w, h) {}
	~AvxRGB() {}
	void toYUV(YUV* dst_yuv);
	void alphaBlend(const uint8_t alpha, const RGB* src_rgb);
	void overlay(const uint8_t alpha, const RGB* src_rgb1, const RGB* src_rgb2);
};
//void overlay_NonSimd(const RGB* dst_rgb, const RGB* src_rgb_1, const RGB* src_rgb_2, const uint8_t alpha);
//void overlay_MMX(const RGB* dst_rgb, const RGB* src_rgb_1, const RGB* src_rgb_2, const uint8_t alpha);
//void overlay_SSE(const RGB* dst_rgb, const RGB* src_rgb_1, const RGB* src_rgb_2, const uint8_t alpha);
//void overlay_AVX(const RGB* dst_rgb, const RGB* src_rgb_1, const RGB* src_rgb_2, const uint8_t alpha);
#endif
