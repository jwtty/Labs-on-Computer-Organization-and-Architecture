#include "yuv.h"
#include "rgb.h"
#include <mmintrin.h> 

static const __m64 OFFSET_128 = _mm_set_pi16(128, 128, 128, 128);
static const __m64 OFFSET_16 = _mm_set_pi16(16, 16, 16, 16);

void MmxYUV::toRGB(RGB *dst_rgb)
{
	int16_t tmp_u[width * height];
	int16_t tmp_v[width * height];
	u8_to_s16();
	int i, j, k;
	__m64 tmp, tmp_data;
	for (i = 0, k = 0; i < dst_rgb -> height; ++i) 
	{
		for (j = 0; j < dst_rgb -> width; ++j, ++k) 
		{
			int loc = (i / 2) * dst_rgb->width / 2 + (j / 2);
			tmp_u[k] = U16[loc];
			tmp_v[k] = V16[loc];
		}
	}
	_mm_empty();

	// YUV to R Channel
	__m64 *dst = (__m64*)dst_rgb -> R16;
	__m64 *src_y = (__m64*)Y16;
	__m64 *src_u = (__m64*)tmp_u;
	__m64 *src_v = (__m64*)tmp_v;
	const __m64 Y_R = _mm_set_pi16(YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0]);
	const __m64 U_R = _mm_set_pi16(YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1]);
	const __m64 V_R = _mm_set_pi16(YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2]);
	for (i = 0; i < dst_rgb -> height * dst_rgb -> width / 4; i++) {
		// Y Channel to R Channel
		tmp_data = _m_psubw(*src_y, OFFSET_16); // (Y - 16)
		tmp = _m_pmulhw(tmp_data, Y_R);			// R = (Y - 16) * 0.164383
		*dst = _m_paddsw(tmp, tmp_data);		// R += Y - 16
		
		// V Channel to R Channel
		tmp_data = _m_psubw(*src_v, OFFSET_128);// (V - 128)
		tmp = _m_pmulhw(tmp_data, V_R);			// (V - 128) * 0.096027
		*dst = _m_paddsw(*dst, tmp);			// R += (V - 128) * 0.096027
		*dst = _m_paddsw(*dst, tmp_data);       // R += (V - 128)
		tmp = _m_psrawi(tmp_data, 1);
		*dst = _m_paddsw(*dst, tmp);			// R += (V - 128) >> 1;

		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to R Channel

	// YUV to G Channel
	dst = (__m64*) dst_rgb->G16;
	src_y = (__m64*)Y16;
	src_u = (__m64*)tmp_u;
	src_v = (__m64*)tmp_v;
	const __m64 Y_G = _mm_set_pi16(YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0]);
	const __m64 U_G = _mm_set_pi16(YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1]);
	const __m64 V_G = _mm_set_pi16(YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2]);
	for (i = 0; i < dst_rgb->height * dst_rgb->width / 4; i++) {
		// Y Channel to G Channel
		tmp_data = _m_psubw(*src_y, OFFSET_16); // (Y - 16)
		tmp = _m_pmulhw(tmp_data, Y_G);			// G = (Y - 16) * 0.164383
		*dst = _m_paddsw(tmp, tmp_data);		// G += Y - 16

		// U Channel to G Channel
		tmp_data = _m_psubw(*src_u, OFFSET_128);// (U - 128)
		tmp = _m_pmulhw(tmp_data, U_G);			// (U - 128) * (-0.391762)
		*dst = _m_paddsw(*dst, tmp);			// G += (U - 128) * (-0.391762)

		// V Channel to R Channel
		tmp_data = _m_psubw(*src_v, OFFSET_128);// (V - 128)
		tmp = _m_pmulhw(tmp_data, V_G);			// (V - 128) * (-0.312968)
		*dst = _m_paddsw(*dst, tmp);			// G += (V - 128) * (-0.312968)
		tmp = _m_psrawi(tmp_data, 1);
		*dst = _m_psubsw(*dst, tmp);			// G -= (V - 128) >> 1;

		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to G Channel

	// YUV to B Channel
	dst = (__m64*) dst_rgb->B16;
	src_y = (__m64*)Y16;
	src_u = (__m64*)tmp_u;
	src_v = (__m64*)tmp_v;
	const __m64 Y_B = _mm_set_pi16(YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0]);
	const __m64 U_B = _mm_set_pi16(YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1]);
	const __m64 V_B = _mm_set_pi16(YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2]);
	for (i = 0; i < dst_rgb->height * dst_rgb->width / 4; i++) {
		// Y Channel to B Channel
		tmp_data = _m_psubw(*src_y, OFFSET_16); // (Y - 16)
		tmp = _m_pmulhw(tmp_data, Y_B);			// B = (Y - 16) * 0.164383
		*dst = _m_paddsw(tmp, tmp_data);		// B += Y - 16

		// U Channel to B Channel
		tmp_data = _m_psubw(*src_u, OFFSET_128);// (U - 128)
		tmp = _m_pmulhw(tmp_data, V_B);			// (U - 128) * 0.017232
		*dst = _m_paddsw(*dst, tmp);			// B += (U - 128) * 0.017232
		tmp = _m_psllwi(tmp_data, 1);
		*dst = _m_paddsw(*dst, tmp);			// B += (U - 128) << 1;
		
		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to B Channel

	_mm_empty();

	dst_rgb -> BoundCheck();
	dst_rgb -> s16_to_u8();
}

void MmxRGB::toYUV(YUV* dst_yuv) 
{
	int16_t tmp_r[width * height / 4];
	int16_t tmp_g[width * height / 4];
	int16_t tmp_b[width * height / 4];
	int i, j, k;
	__m64 tmp;
	_mm_empty();
	// RGB to Y Channel
	__m64* dst = (__m64*) dst_yuv -> Y16;
	__m64* src_r = (__m64*)R16;
	__m64* src_g = (__m64*)G16;
	__m64* src_b = (__m64*)B16;
	const __m64 R_Y = _mm_set_pi16(RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0]);
	const __m64 G_Y = _mm_set_pi16(RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1]);
	const __m64 B_Y = _mm_set_pi16(RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2]);
	for (i = 0; i < dst_yuv->height * dst_yuv->width / 4; i++) {
		// R Channel to Y Channel
		*dst = _m_pmulhw(*src_r, R_Y); // Y = R * 0.256788
		
		// G Channel to Y Channel
		tmp = _m_pmulhw(*src_g, G_Y);
		*dst = _m_paddsw(tmp, *dst); // Y += G * 0.004129
		tmp = _m_psrlwi(*src_g, 1);
		*dst = _m_paddsw(tmp, *dst); // Y += G >> 1;

		// B Channel to Y Channel
		tmp = _m_pmulhw(*src_b, B_Y);
		*dst = _m_paddsw(tmp, *dst); // Y += B * 0.097906

		// Add offset
		*dst = _m_paddsw(*dst, OFFSET_16); // Y += 16

		// increase iterators
		dst++;
		src_r++;
		src_g++;
		src_b++;
	}
	// End of RGB to Y Channel
	


	for (i = 0, k = 0; i < dst_yuv->height; i += 2) {
		for (j = 0; j < dst_yuv->width; j += 2, ++k) {
			tmp_r[k] = R16[i * dst_yuv->width + j];
			tmp_g[k] = G16[i * dst_yuv->width + j];
			tmp_b[k] = B16[i * dst_yuv->width + j];
		}
	}

	// RGB to U Channel
	dst = (__m64*) dst_yuv -> U16;
	src_r = (__m64*) tmp_r;
	src_g = (__m64*) tmp_g;
	src_b = (__m64*) tmp_b;
	const __m64 R_U = _mm_set_pi16(RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0]);
	const __m64 G_U = _mm_set_pi16(RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1]);
	const __m64 B_U = _mm_set_pi16(RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2]);
	for (i = 0; i < width * height / 16; i++) {
		// R Channel to U Channel
		*dst = _m_pmulhw(*src_r, R_U); // U = R * (-0.148223)

		// G Channel to U Channel
		tmp = _m_pmulhw(*src_g, G_U);
		*dst = _m_paddsw(tmp, *dst); // U += G * (-0.290993)

		// B Channel to U Channel
		tmp = _m_pmulhw(*src_b, B_U);
		*dst = _m_paddsw(tmp, *dst); // U += B * (0.439216)

		// Add offset
		*dst = _m_paddsw(*dst, OFFSET_128); // U += 128

		// increase iterators
		dst++;
		src_r++;
		src_g++;
		src_b++;
	}
	// End of RGB to U Channel

	// RGB to V Channel
	dst = (__m64*) dst_yuv -> V16;
	src_r = (__m64*) tmp_r;
	src_g = (__m64*) tmp_g;
	src_b = (__m64*) tmp_b;
	const __m64 R_V = _mm_set_pi16(RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0]);
	const __m64 G_V = _mm_set_pi16(RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1]);
	const __m64 B_V = _mm_set_pi16(RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2]);
	for (i = 0; i < width * height / 16; i++) {
		// R Channel to V Channel
		*dst = _m_pmulhw(*src_r, R_V); // V = R * (0.439216)

		// G Channel to V Channel
		tmp = _m_pmulhw(*src_g, G_V);
		*dst = _m_paddsw(tmp, *dst); // V += G * (-0.367788)

		 // B Channel to V Channel
		tmp = _m_pmulhw(*src_b, B_V);
		*dst = _m_paddsw(tmp, *dst); // V += B * (-0.071427)

		// Add offset
		*dst = _m_paddsw(*dst, OFFSET_128); // V += 128

		// increase iterators
		dst++;
		src_r++;
		src_g++;
		src_b++;
	}
	// End of RGB to V Channel

	_mm_empty();
	dst_yuv -> s16_to_u8();
	// End of function
}

inline void alpha_blend_MMX(__m64* dst, __m64* src, const uint16_t _alpha, const unsigned int loop) 
{
	int i = 0;
	_mm_empty();
	__m64 alpha = _mm_set_pi16(_alpha, _alpha, _alpha, _alpha);
	__m64 tmp;
	for (i = 0; i < loop; i++) {
		tmp = _m_pmullw(*src, alpha);
		tmp = _m_psrlwi(tmp, 8);
		*dst = tmp;
		dst++;
		src++;
	}
	_mm_empty();
}

inline void blend_one_color_MMX(int16_t* dst16, uint16_t* src16, const uint8_t alpha, int width, int height) 
{
	alpha_blend_MMX((__m64*)dst16, (__m64*)src16, alpha, width * height / 4);
}

void MmxRGB::alphaBlend(const uint8_t alpha, const RGB* src_rgb) 
{
	// In this function, it only changes the RGB16. Thus, it can not match functions in Non_Simd
	blend_one_color_MMX(R16, (uint16_t*)src_rgb->R16, alpha, width, height);
	blend_one_color_MMX(G16, (uint16_t*)src_rgb->G16, alpha, width, height);
	blend_one_color_MMX(B16, (uint16_t*)src_rgb->B16, alpha, width, height);
}

inline void image_overlay_MMX(__m64* dst, __m64* src1, __m64* src2, const uint16_t __alpha, const unsigned int loop) 
{
	int i = 256 - __alpha;
	_mm_empty();
	__m64 alpha = _mm_set_pi16(__alpha, __alpha, __alpha, __alpha);
	__m64 _alpha = _mm_set_pi16(i, i, i, i);
	__m64 tmp1, tmp2;
	for (i = 0; i < loop; i++) {
		tmp1 = _m_pmullw(*src1, alpha);
		tmp2 = _m_pmullw(*src2, _alpha);
		*dst = _m_psrlwi(_m_paddusw(tmp1, tmp2), 8);
		dst++;
		src1++;
		src2++;
	}
	_mm_empty();
}

inline void overlay_one_color_MMX(int16_t* dst16, uint16_t* src16_1, uint16_t* src16_2, const uint8_t alpha, int width, int height) 
{
	image_overlay_MMX((__m64*)dst16, (__m64*)src16_1, (__m64*)src16_2, alpha, width * height / 4);
}

void MmxRGB::overlay(const uint8_t alpha, const RGB *src_rgb_1, const RGB *src_rgb_2) 
{
	// In this function, it only changes the RGB16. Thus, it can not match functions in Non_Simd
	overlay_one_color_MMX(R16, (uint16_t*)src_rgb_1->R16, (uint16_t*)src_rgb_2->R16, alpha, width, height);
	overlay_one_color_MMX(G16, (uint16_t*)src_rgb_1->G16, (uint16_t*)src_rgb_2->G16, alpha, width, height);
	overlay_one_color_MMX(B16, (uint16_t*)src_rgb_1->B16, (uint16_t*)src_rgb_2->B16, alpha, width, height);
}
