#include "yuv.h"
#include "rgb.h"
#include <emmintrin.h>

static const __m128i OFFSET_128 = _mm_set_epi16(128, 128, 128, 128, 128, 128, 128, 128);
static const __m128i OFFSET_16 = _mm_set_epi16(16, 16, 16, 16, 16, 16, 16, 16);

void SseYUV::toRGB(RGB* dst_rgb) {
	u8_to_s16();
	int i, j, k;
	__m128i tmp, tmp_data;
	int16_t tmp_u[width * height];
	int16_t tmp_v[width * height];
	for (i = 0, k = 0; i < dst_rgb -> height; ++i) {
		for (j = 0; j < dst_rgb -> width; ++j, ++k) {
			int loc = (i / 2) * dst_rgb->width / 2 + (j / 2);
			tmp_u[k] = U16[loc];
			tmp_v[k] = V16[loc];
		}
	}

	_mm_empty();

	// YUV to R Channel
	__m128i* dst = (__m128i*) dst_rgb->R16;
	__m128i* src_y = (__m128i*)Y16;
	__m128i* src_u = (__m128i*)tmp_u;
	__m128i* src_v = (__m128i*)tmp_v;
	const __m128i Y_R = _mm_set_epi16(YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0], YUV_R[0]);
	const __m128i U_R = _mm_set_epi16(YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1], YUV_R[1]);
	const __m128i V_R = _mm_set_epi16(YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2], YUV_R[2]);
	for (i = 0; i < dst_rgb->height * dst_rgb->width / 8; i++) {
		// Y Channel to R Channel
		tmp_data = _mm_subs_epi16(*src_y, OFFSET_16);	// (Y - 16)
		tmp = _mm_mulhi_epi16(tmp_data, Y_R);			// R = (Y - 16) * 0.164383
		*dst = _mm_adds_epi16(tmp, tmp_data);			// R += Y - 16

		// V Channel to R Channel
		tmp_data = _mm_subs_epi16(*src_v, OFFSET_128);	// (V - 128)
		tmp = _mm_mulhi_epi16(tmp_data, V_R);			// (V - 128) * 0.096027
		*dst = _mm_adds_epi16(*dst, tmp);				// R += (V - 128) * 0.096027
		*dst = _mm_adds_epi16(*dst, tmp_data);           // R += (V - 128)
		tmp = _mm_srai_epi16(tmp_data, 1);
		*dst = _mm_adds_epi16(*dst, tmp);				// R += (V - 128) >> 1;

		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to R Channel

	// YUV to G Channel
	dst = (__m128i*) dst_rgb->G16;
	src_y = (__m128i*)Y16;
	src_u = (__m128i*)tmp_u;
	src_v = (__m128i*)tmp_v;
	const __m128i Y_G = _mm_set_epi16(YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0], YUV_G[0]);
	const __m128i U_G = _mm_set_epi16(YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1], YUV_G[1]);
	const __m128i V_G = _mm_set_epi16(YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2], YUV_G[2]);
	for (i = 0; i < dst_rgb->height * dst_rgb->width / 8; i++) {
		// Y Channel to G Channel
		tmp_data = _mm_subs_epi16(*src_y, OFFSET_16);	// (Y - 16)
		tmp = _mm_mulhi_epi16(tmp_data, Y_G);			// G = (Y - 16) * 0.164383
		*dst = _mm_adds_epi16(tmp, tmp_data);			// G += Y - 16

		// U Channel to G Channel
		tmp_data = _mm_subs_epi16(*src_u, OFFSET_128);	// (U - 128)
		tmp = _mm_mulhi_epi16(tmp_data, U_G);			// (U - 128) * (-0.391762)
		*dst = _mm_adds_epi16(*dst, tmp);				// G += (U - 128) * (-0.391762)

		// V Channel to R Channel
		tmp_data = _mm_subs_epi16(*src_v, OFFSET_128);	// (V - 128)
		tmp = _mm_mulhi_epi16(tmp_data, V_G);			// (V - 128) * (-0.312968)
		*dst = _mm_adds_epi16(*dst, tmp);				// G += (V - 128) * (-0.312968)
		tmp = _mm_srai_epi16(tmp_data, 1);
		*dst = _mm_subs_epi16(*dst, tmp);				// G -= (V - 128) >> 1;

		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to G Channel

	// YUV to B Channel
	dst = (__m128i*) dst_rgb->B16;
	src_y = (__m128i*)Y16;
	src_u = (__m128i*)tmp_u;
	src_v = (__m128i*)tmp_v;
	const __m128i Y_B = _mm_set_epi16(YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0], YUV_B[0]);
	const __m128i U_B = _mm_set_epi16(YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1], YUV_B[1]);
	const __m128i V_B = _mm_set_epi16(YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2], YUV_B[2]);
	for (i = 0; i < dst_rgb->height * dst_rgb->width / 8; i++) {
		// Y Channel to B Channel
		tmp_data = _mm_subs_epi16(*src_y, OFFSET_16);	// (Y - 16)
		tmp = _mm_mulhi_epi16(tmp_data, Y_B);			// B = (Y - 16) * 0.164383
		*dst = _mm_adds_epi16(tmp, tmp_data);			// B += Y - 16

		// U Channel to B Channel
		tmp_data = _mm_subs_epi16(*src_u, OFFSET_128);	// (U - 128)
		tmp = _mm_mulhi_epi16(tmp_data, U_B);			// (U - 128) * 0.017232
		*dst = _mm_adds_epi16(*dst, tmp);				// B += (U - 128) * 0.017232
		tmp = _mm_slli_epi16(tmp_data, 1);
		*dst = _mm_adds_epi16(*dst, tmp);				// G += (U - 128) << 1;

		// increase iterators
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}
	// End of YUV to B Channel

	_mm_empty();

	dst_rgb->BoundCheck();
	dst_rgb->s16_to_u8();
}

void SseRGB::toYUV(YUV* dst_yuv) {

	int i, j, k;
	__m128i tmp;
	int16_t tmp_r[width * height / 4];
	int16_t tmp_g[width * height / 4];
	int16_t tmp_b[width * height / 4];
	_mm_empty();

	// RGB to Y Channel
	__m128i* dst = (__m128i*) dst_yuv->Y16;
	__m128i* src_r = (__m128i*)R16;
	__m128i* src_g = (__m128i*)G16;
	__m128i* src_b = (__m128i*)B16;
	const __m128i R_Y = _mm_set_epi16(RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0], RGB_Y[0]);
	const __m128i G_Y = _mm_set_epi16(RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1], RGB_Y[1]);
	const __m128i B_Y = _mm_set_epi16(RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2], RGB_Y[2]);
	for (i = 0; i < dst_yuv->height * dst_yuv->width / 8; i++) {
		// R Channel to Y Channel
		*dst = _mm_mulhi_epi16(*src_r, R_Y); // Y = R * 0.256788

		// G Channel to Y Channel
		tmp = _mm_mulhi_epi16(*src_g, G_Y);
		*dst = _mm_adds_epi16(tmp, *dst); // Y += G * 0.004129
		tmp = _mm_srli_epi16(*src_g, 1);
		*dst = _mm_adds_epi16(tmp, *dst); // Y += G >> 1;

		// B Channel to Y Channel
		tmp = _mm_mulhi_epi16(*src_b, B_Y);
		*dst = _mm_adds_epi16(tmp, *dst); // Y += B * 0.097906

		// Add offset
		*dst = _mm_adds_epi16(*dst, OFFSET_16); // Y += 16

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
	dst = (__m128i*) dst_yuv->U16;
	src_r = (__m128i*) tmp_r;
	src_g = (__m128i*) tmp_g;
	src_b = (__m128i*) tmp_b;
	const __m128i R_U = _mm_set_epi16(RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0], RGB_U[0]);
	const __m128i G_U = _mm_set_epi16(RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1], RGB_U[1]);
	const __m128i B_U = _mm_set_epi16(RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2], RGB_U[2]);
	for (i = 0; i < width * height / 32; i++) {
		// R Channel to U Channel
		*dst = _mm_mulhi_epi16(*src_r, R_U); // U = R * （-0.148223）

		// G Channel to U Channel
		tmp = _mm_mulhi_epi16(*src_g, G_U);
		*dst = _mm_adds_epi16(tmp, *dst); // U += G * (-0.290993)

		// B Channel to U Channel
		tmp = _mm_mulhi_epi16(*src_b, B_U);
		*dst = _mm_adds_epi16(tmp, *dst); // U += B * (0.439216)

		// Add offset
		*dst = _mm_adds_epi16(*dst, OFFSET_128); // U += 128

		// increase iterators
		dst++;
		src_r++;
		src_g++;
		src_b++;
	}
	// End of RGB to U Channel

	// RGB to V Channel
	dst = (__m128i*) dst_yuv->V16;
	src_r = (__m128i*) tmp_r;
	src_g = (__m128i*) tmp_g;
	src_b = (__m128i*) tmp_b;
	const __m128i R_V = _mm_set_epi16(RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0], RGB_V[0]);
	const __m128i G_V = _mm_set_epi16(RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1], RGB_V[1]);
	const __m128i B_V = _mm_set_epi16(RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2], RGB_V[2]);
	for (i = 0; i < width * height / 32; i++) {
		// R Channel to V Channel
		*dst = _mm_mulhi_epi16(*src_r, R_V); // V = R * (0.439216)

		// G Channel to V Channel
		tmp = _mm_mulhi_epi16(*src_g, G_V);
		*dst = _mm_adds_epi16(tmp, *dst); // V += G * (-0.367788)

		// B Channel to V Channel
		tmp = _mm_mulhi_epi16(*src_b, B_V);
		*dst = _mm_adds_epi16(tmp, *dst); // V += B * (-0.071427)

		// Add offset
		*dst = _mm_adds_epi16(*dst, OFFSET_128); // V += 128

		// increase iterators
		dst++;
		src_r++;
		src_g++;
		src_b++;
	}
	// End of RGB to V Channel

	_mm_empty();
	dst_yuv->s16_to_u8();
	// End of function
}

inline void alpha_blend_SSE(__m128i* dst, __m128i* src, const uint16_t _alpha, const unsigned int loop) {
	int i = 0;
	_mm_empty();
	__m128i alpha = _mm_set_epi16(_alpha, _alpha, _alpha, _alpha, _alpha, _alpha, _alpha, _alpha);
	__m128i tmp;
	for (i = 0; i < loop; i++) {
		tmp = _mm_mullo_epi16(*src, alpha);
		tmp = _mm_srli_epi16(tmp, 8);
		*dst = tmp;
		dst++;
		src++;
	}
	_mm_empty();
}

inline void blend_one_color_SSE(int16_t* dst16, uint16_t* src16, const uint8_t alpha, int width, int height) {
	alpha_blend_SSE((__m128i*)dst16, (__m128i*)src16, alpha, width * height / 8);
}

void SseRGB::alphaBlend(const uint8_t alpha, const RGB* src_rgb) {
	// In this function, it only changes the RGB16. Thus, it can not match functions in Non_Simd
	blend_one_color_SSE(R16, (uint16_t*)src_rgb->R16, alpha, width, height);
	blend_one_color_SSE(G16, (uint16_t*)src_rgb->G16, alpha, width, height);
	blend_one_color_SSE(B16, (uint16_t*)src_rgb->B16, alpha, width, height);
}

inline void image_overlay_SSE(__m128i* dst, __m128i* src1, __m128i* src2, const uint16_t __alpha, const unsigned int loop) {
	int i = 256 - __alpha;
	_mm_empty();
	__m128i alpha = _mm_set_epi16(__alpha, __alpha, __alpha, __alpha, __alpha, __alpha, __alpha, __alpha);
	__m128i _alpha = _mm_set_epi16(i, i, i, i, i, i, i, i);
	__m128i tmp1, tmp2;
	for (i = 0; i < loop; i++) {
		tmp1 = _mm_mullo_epi16(*src1, alpha);
		tmp2 = _mm_mullo_epi16(*src2, _alpha);
		*dst = _mm_srli_epi16(_mm_add_epi16(tmp1, tmp2), 8);
		dst++;
		src1++;
		src2++;
	}
	_mm_empty();
}

inline void overlay_one_color_SSE(int16_t* dst16, uint16_t* src16_1, uint16_t* src16_2, const uint8_t alpha, int width, int height) {
	image_overlay_SSE((__m128i*)dst16, (__m128i*)src16_1, (__m128i*)src16_2, alpha, width * height / 8);
}

void SseRGB::overlay(const uint8_t alpha, const RGB *src_rgb_1, const RGB *src_rgb_2) 
{
	// In this function, it only changes the RGB16. Thus, it can not match functions in Non_Simd
	overlay_one_color_SSE(R16, (uint16_t*)src_rgb_1->R16, (uint16_t*)src_rgb_2->R16, alpha, width, height);
	overlay_one_color_SSE(G16, (uint16_t*)src_rgb_1->G16, (uint16_t*)src_rgb_2->G16, alpha, width, height);
	overlay_one_color_SSE(B16, (uint16_t*)src_rgb_1->B16, (uint16_t*)src_rgb_2->B16, alpha, width, height);
}
