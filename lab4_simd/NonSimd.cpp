#include "yuv.h"
#include "rgb.h"


void NonSimdYUV::toRGB(RGB *dst_rgb)
{
	u8_to_s16();
	int i = 0;
	for (int j = 0; j < dst_rgb -> height; ++j)
	{
		for (int k = 0; k < dst_rgb -> width; ++k)
		{
			int loc = (j >> 1) * (dst_rgb -> width >> 1) + (k >> 1);
			dst_rgb -> R16[i] = int16_t(((int)(Y16[i] - 16) * (int)YUV_R[0] + (int)(U16[loc] - 128) * (int)YUV_R[1] + (int)(V16[loc] - 128) * (int)YUV_R[2]) >> 16);
			dst_rgb -> R16[i] += Y16[i] - 16 + ((V16[loc] - 128) >> 1) + V16[loc] - 128;
			dst_rgb -> G16[i] = int16_t(((int)(Y16[i] - 16) * (int)YUV_G[0] + (int)(U16[loc] - 128) * (int)YUV_G[1] + (int)(V16[loc] - 128) * (int)YUV_G[2]) >> 16);
			dst_rgb -> G16[i] += Y16[i] - 16 - ((V16[loc] - 128) >> 1);
			dst_rgb -> B16[i] = int16_t(((int)(Y16[i] - 16) * (int)YUV_B[0] + (int)(U16[loc] - 128) * (int)YUV_B[1] + (int)(V16[loc] - 128) * (int)YUV_B[2]) >> 16);
			dst_rgb -> B16[i] += Y16[i] - 16 + ((U16[loc] - 128) << 1);
			++i;
		}
	}
	dst_rgb -> BoundCheck();
	dst_rgb -> s16_to_u8();
}
/*void NonSimdYUV::toRGB(RGB *dst_rgb)
{
	u8_to_s16();
	int k = 0;
	for (int i = 0; i < dst_rgb -> height; ++i)
	{
		for (int j = 0; j < dst_rgb -> width; ++j)
		{
			int loc = (i >> 1) * (dst_rgb -> width >> 1) + (j >> 1);
			dst_rgb -> R16[k] = (int16_t)(Y16[k] - 16) * 1.164383 + (int16_t)(V16[loc] - 128) * 1.596027;
			dst_rgb -> B16[k] = (int16_t)(Y16[k] - 16) * 1.164383 + (int16_t)(U16[loc] - 128) * 2.017232;
			dst_rgb -> G16[k] = (int16_t)(Y16[k] - 16) * 1.164383 - (int16_t)(U16[loc] - 128) * 0.391762 - (V16[loc] - 128) * 0.812968;
			++k;
		}
	}
	dst_rgb -> BoundCheck();
	dst_rgb -> s16_to_u8();
}*/

void NonSimdRGB::toYUV(YUV* dst_yuv) 
{
	for (int i = 0; i < length; ++i)
	{
		dst_yuv -> Y8[i] = uint8_t(((int)R8[i] * (int)RGB_Y[0] + (int)G8[i] * (int)RGB_Y[1] + (int)B8[i] * (int)RGB_Y[2]) >> 16);
		dst_yuv -> Y8[i] += (G8[i] >> 1) + 16;
	}
	int k = 0;
	for (int i = 0; i < dst_yuv -> height; i += 2)
	{
		for (int j = 0; j < dst_yuv -> width; j += 2)
		{
			int loc = i * dst_yuv -> width + j;
			dst_yuv -> U8[k] = uint8_t(((int)R8[loc] * (int)RGB_U[0] + (int)G8[loc] * (int)RGB_U[1] + (int)B8[loc] * (int)RGB_U[2]) >> 16);
			dst_yuv -> U8[k] += 128;
			dst_yuv -> V8[k] = uint8_t(((int)R8[loc] * (int)RGB_V[0] + (int)G8[loc] * (int)RGB_V[1] + (int)B8[loc] * (int)RGB_V[2]) >> 16);
			dst_yuv -> V8[k] += 128;
			++k;
		}
	}
}

/*inline uint8_t format(int16_t input)
{
	if(input > 255)
		return (uint8_t)255;
	if(input < 0)
		return (uint8_t)0;
	return (uint8_t)input;
}
void NonSimdRGB::toYUV(YUV* dst_yuv)
{
	for (int i = 0; i < length; ++i)
	{
		dst_yuv -> Y8[i] = format(0.256788 * R8[i] + 0.504129 * G8[i] + 0.097906 * B8[i] + 16);
	}
	int k = 0;
	for (int i = 0; i < height; i += 2)
	{
		for (int j = 0; j < width; j += 2)
		{
			int loc = i * width + j;
			dst_yuv -> U8[k] = format(-0.148223 * R8[loc] - 0.290993 * G8[loc] + 0.439216 * B8[loc] + 128);
			dst_yuv -> V8[k] = format(0.439216 * R8[loc] - 0.367786 * G8[loc] - 0.071427 * B8[loc] + 128);
			++k;
		}
	}
}*/

void NonSimdRGB::alphaBlend(const uint8_t alpha, const RGB* src_rgb)
{
	for (int i = 0; i < length; ++i)
	{
		R8[i] = ((int16_t)src_rgb -> R8[i] * alpha) >> 8;
		G8[i] = ((int16_t)src_rgb -> G8[i] * alpha) >> 8;
		B8[i] = ((int16_t)src_rgb -> B8[i] * alpha) >> 8;
	}
}

void NonSimdRGB::overlay(const uint8_t alpha, const RGB *src_rgb_1, const RGB *src_rgb_2) {
	uint8_t _alpha = 256 - alpha;
	for (int i = 0; i < length; ++i) {
		R8[i] = ((int16_t)src_rgb_1->R8[i] * alpha + (int16_t)src_rgb_2->R8[i] * _alpha) >> 8;
		G8[i] = ((int16_t)src_rgb_1->G8[i] * alpha + (int16_t)src_rgb_2->G8[i] * _alpha) >> 8;
		B8[i] = ((int16_t)src_rgb_1->B8[i] * alpha + (int16_t)src_rgb_2->B8[i] * _alpha) >> 8;
	}
}
