#include <iostream>
#include <ctime>
#include <stdint.h>
#include <cstring>
#include "yuv.h"
#include "rgb.h"
using namespace std;

#define WIDTH 1920  
#define HEIGHT 1080
#define SRC_YUV1 "dem1.yuv"
#define SRC_YUV2 "dem2.yuv"
#define DST_YUV "result.yuv"
clock_t totaltime = 0;
clock_t begintime = 0;

int main(int argc, char * argv[])
{
	if(argc < 2)
	{
		cout << "Parameter missing!" << endl;
		return 0;
	}
	else if(strcmp(argv[1], "1") && strcmp(argv[1], "2"))
	{
		cout << "Parameter wrong!" << endl;
		return 0;
	}
	#ifdef MMX
		MmxYUV src_yuv(WIDTH, HEIGHT);
		MmxYUV dst_yuv(WIDTH, HEIGHT);
		MmxRGB base_rgb(WIDTH, HEIGHT);
		MmxRGB dst_rgb(WIDTH, HEIGHT);
		MmxYUV src_yuv2(WIDTH, HEIGHT);
		MmxRGB base_rgb2(WIDTH, HEIGHT);
	#else
		#ifdef SSE
			SseYUV src_yuv(WIDTH, HEIGHT);
			SseYUV dst_yuv(WIDTH, HEIGHT);
			SseRGB base_rgb(WIDTH, HEIGHT);
			SseRGB dst_rgb(WIDTH, HEIGHT);
			SseYUV src_yuv2(WIDTH, HEIGHT);
			SseRGB base_rgb2(WIDTH, HEIGHT);
		#else
			#ifdef AVX
				AvxYUV src_yuv(WIDTH, HEIGHT);
				AvxYUV dst_yuv(WIDTH, HEIGHT);
				AvxRGB base_rgb(WIDTH, HEIGHT);
				AvxRGB dst_rgb(WIDTH, HEIGHT);
				AvxYUV src_yuv2(WIDTH, HEIGHT);
				AvxRGB base_rgb2(WIDTH, HEIGHT);
			#else
				NonSimdYUV src_yuv(WIDTH, HEIGHT);
				NonSimdYUV dst_yuv(WIDTH, HEIGHT);
				NonSimdRGB base_rgb(WIDTH, HEIGHT);
				NonSimdRGB dst_rgb(WIDTH, HEIGHT);
				NonSimdYUV src_yuv2(WIDTH, HEIGHT);
				NonSimdRGB base_rgb2(WIDTH, HEIGHT);
			#endif
		#endif
	#endif
	if(src_yuv.Read_YUV_File(SRC_YUV1) == -1)
		return 0;
	if(!strcmp(argv[1], "2")) 
	{
		if(src_yuv2.Read_YUV_File(SRC_YUV2) == -1)
			return 0;
	}
	begintime = clock();
	src_yuv.toRGB(&base_rgb);
	if(!strcmp(argv[1], "2")) 
		src_yuv2.toRGB(&base_rgb2);
	totaltime += clock() - begintime;
	for (int alpha = 1; alpha < 256; alpha += 3)
	{
		begintime = clock();
		if(!strcmp(argv[1], "1"))
			dst_rgb.alphaBlend((uint8_t)alpha, &base_rgb);
		else if(!strcmp(argv[1], "2"))
			dst_rgb.overlay((uint8_t)alpha, &base_rgb, &base_rgb2);
		dst_rgb.toYUV(&dst_yuv);
		totaltime += clock() - begintime;
		dst_yuv.Write_YUV_File(DST_YUV);
	}
	cout << "Critical Function time: " << totaltime << endl;
	return 0;
}