#define NEED_SSE3 1

#include <Ector.h>
#include "ector_drawhelper_private.h"

//#ifdef BUILD_SSE3
#include <immintrin.h>

// NOTE
// a is already prepared for word multiplication, for performance
// reason
// ie
// __m128i v_al = _mm_unpacklo_epi16(v_ialpha, v_ialpha);
// __m128i v_ah = _mm_unpackhi_epi16(v_ialpha, v_ialpha);
// __m128i a = (__m128i) _mm_shuffle_ps( (__m128)v_al, (__m128)v_ah, 0x88);
//
inline static __m128i
v4_byte_mul_special_sse2(__m128i c, __m128i a)
{
   const __m128i ga_mask = _mm_set1_epi32(0x00FF00FF);
   const __m128i rb_mask = _mm_set1_epi32(0xFF00FF00);

   /* first half of calc */
   __m128i c0 = c;
   c0 = _mm_srli_epi32(c0, 8);
   c0 = _mm_and_si128(ga_mask, c0);
   c0 = _mm_mullo_epi16(a, c0);
   c0 = _mm_and_si128(rb_mask, c0);

   /* second half of calc */
   __m128i c1 = c;
   c1 = _mm_and_si128(ga_mask, c1);
   c1 = _mm_mullo_epi16(a, c1);
   c1 = _mm_srli_epi32(c1, 8);
   c1 = _mm_and_si128(ga_mask, c1);

   /* combine */
   return _mm_add_epi32(c0, c1);
}

inline static __m128i
v4_byte_mul_sse2(__m128i c, __m128i a)
{
   const __m128i ga_mask = _mm_set1_epi32(0x00FF00FF);
   const __m128i rb_mask = _mm_set1_epi32(0xFF00FF00);
   __m128i v_al = _mm_unpacklo_epi16(a, a);
   __m128i v_ah = _mm_unpackhi_epi16(a, a);
   __m128i v_a = (__m128i) _mm_shuffle_ps( (__m128)v_al, (__m128)v_ah, 0x88);

   /* first half of calc */
   __m128i c0 = c;
   c0 = _mm_srli_epi32(c0, 8);
   c0 = _mm_and_si128(ga_mask, c0);
   c0 = _mm_mullo_epi16(v_a, c0);
   c0 = _mm_and_si128(rb_mask, c0);

   /* second half of calc */
   __m128i c1 = c;
   c1 = _mm_and_si128(ga_mask, c1);
   c1 = _mm_mullo_epi16(v_a, c1);
   c1 = _mm_srli_epi32(c1, 8);
   c1 = _mm_and_si128(ga_mask, c1);

   /* combine */
   return _mm_add_epi32(c0, c1);
}

static inline __m128i
v4_interpolate_color_sse2(__m128i a, __m128i c0, __m128i c1)
{
   const __m128i rb_mask = _mm_set1_epi32(0xFF00FF00);
   const __m128i zero = _mm_setzero_si128();

   __m128i a_l = a;
   __m128i a_h = a;
   a_l = _mm_unpacklo_epi16(a_l, a_l);
   a_h = _mm_unpackhi_epi16(a_h, a_h);

   __m128i a_t = _mm_slli_epi64(a_l, 32);
   __m128i a_t0 = _mm_slli_epi64(a_h, 32);

   a_l = _mm_add_epi32(a_l, a_t);
   a_h = _mm_add_epi32(a_h, a_t0);

   __m128i c0_l = c0;
   __m128i c0_h = c0;

   c0_l = _mm_unpacklo_epi8(c0_l, zero);
   c0_h = _mm_unpackhi_epi8(c0_h, zero);

   __m128i c1_l = c1;
   __m128i c1_h = c1;

   c1_l = _mm_unpacklo_epi8(c1_l, zero);
   c1_h = _mm_unpackhi_epi8(c1_h, zero);

   __m128i cl_sub = _mm_sub_epi16(c0_l, c1_l);
   __m128i ch_sub = _mm_sub_epi16(c0_h, c1_h);

   cl_sub = _mm_mullo_epi16(cl_sub, a_l);
   ch_sub = _mm_mullo_epi16(ch_sub, a_h);

   __m128i c1ls = _mm_slli_epi16(c1_l, 8);
   __m128i c1hs = _mm_slli_epi16(c1_h, 8);

   cl_sub = _mm_add_epi16(cl_sub, c1ls);
   ch_sub = _mm_add_epi16(ch_sub, c1hs);

   cl_sub = _mm_and_si128(cl_sub, rb_mask);
   ch_sub = _mm_and_si128(ch_sub, rb_mask);

   cl_sub = _mm_srli_epi64(cl_sub, 8);
   ch_sub = _mm_srli_epi64(ch_sub, 8);

   cl_sub = _mm_packus_epi16(cl_sub, cl_sub);
   ch_sub = _mm_packus_epi16(ch_sub, ch_sub);

   return  (__m128i) _mm_shuffle_ps( (__m128)cl_sub, (__m128)ch_sub, 0x44);
}

static inline __m128i
v4_mul_color_sse2(__m128i x, __m128i y) {

   const __m128i zero = _mm_setzero_si128();
   const __m128i sym4_mask = _mm_set_epi32(0x00FF00FF, 0x000000FF, 0x00FF00FF, 0x000000FF);
   __m128i x_l = _mm_unpacklo_epi8(x, zero);
   __m128i x_h = _mm_unpackhi_epi8(x, zero);

   __m128i y_l = _mm_unpacklo_epi8(y, zero);
   __m128i y_h = _mm_unpackhi_epi8(y, zero);

   __m128i r_l = _mm_mullo_epi16(x_l, y_l);
   __m128i r_h = _mm_mullo_epi16(x_h, y_h);

   r_l = _mm_add_epi16(r_l, sym4_mask);
   r_h = _mm_add_epi16(r_h, sym4_mask);

   r_l = _mm_srli_epi16(r_l, 8);
   r_h = _mm_srli_epi16(r_h, 8);

   return  _mm_packus_epi16(r_l, r_h);
}

static inline __m128i
v4_ialpha_sse2(__m128i c)
{
   __m128i a = _mm_srli_epi32(c, 24);
   return _mm_sub_epi32(_mm_set1_epi32(0xff), a);
}
// dest = color + (dest * alpha)
inline static void
comp_func_helper_sse2 (uint *dest, int length, uint color, uint alpha)
{
   const __m128i v_color = _mm_set1_epi32(color);
   const __m128i v_ialpha = _mm_set1_epi32(alpha);

   /* prepare alpha for word multiplication */
   __m128i v_al = _mm_unpacklo_epi16(v_ialpha, v_ialpha);
   __m128i v_ah = _mm_unpackhi_epi16(v_ialpha, v_ialpha);
   __m128i v_a = (__m128i) _mm_shuffle_ps( (__m128)v_al, (__m128)v_ah, 0x88);

   LOOP_ALIGNED_U1_A4(dest, length,
      { /* UOP */

         *dest = color + BYTE_MUL(*dest, alpha);
         dest++; length--;
      },
      { /* A4OP */

         __m128i v_dest = _mm_load_si128((__m128i *)dest);

         v_dest = v4_byte_mul_special_sse2(v_dest, v_a);
         v_dest = _mm_add_epi32(v_dest, v_color);

         _mm_store_si128((__m128i *)dest, v_dest);

         dest += 4; length -= 4;
      })
}

void
comp_func_solid_source_sse2(uint *dest, int length, uint color, uint const_alpha)
{
   if (const_alpha == 255) _ector_memfill(dest, length, color);
   else
     {
        int ialpha;
        ialpha = 255 - const_alpha;
        color = BYTE_MUL(color, const_alpha);
        comp_func_helper_sse2(dest, length, color, ialpha);
     }
}

void
comp_func_solid_source_over_sse2(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha;
   if (const_alpha != 255)
     color = BYTE_MUL(color, const_alpha);
   ialpha = Alpha(~color);
   comp_func_helper_sse2(dest, length, color, ialpha);
}

static void
comp_func_source_sse2(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
          memcpy(dest, src, length * sizeof(uint));
        else
         {
            int ialpha = 255 - const_alpha;
            __m128i v_alpha = _mm_set1_epi32(const_alpha);
            LOOP_ALIGNED_U1_A4(dest, length,
              { /* UOP */

                 *dest = INTERPOLATE_PIXEL_256(*src, const_alpha, *dest, ialpha);
                 dest++; src++; length--;
              },
              { /* A4OP */
                const __m128i v_src = _mm_loadu_si128((__m128i *)src);
                __m128i v_dest = _mm_load_si128((__m128i *)dest);
                v_dest = v4_interpolate_color_sse2(v_alpha, v_src, v_dest);
                _mm_store_si128((__m128i *)dest, v_dest);
                 dest += 4; src +=4; length -= 4;
              })
         }
     }
   else
     {
        __m128i v_color = _mm_set1_epi32(color);
        if (const_alpha == 255)
          {
             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */

                  *dest = ECTOR_MUL4_SYM(*src, color);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  __m128i v_src = _mm_loadu_si128((__m128i *)src);
                  v_src = v4_mul_color_sse2(v_src, v_color);
                  _mm_store_si128((__m128i *)dest, v_src);
                  dest += 4; src +=4; length -= 4;
               })
          }
        else
          {
             int ialpha = 255 - const_alpha;
             __m128i v_alpha = _mm_set1_epi32(const_alpha);
             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  uint src_color = ECTOR_MUL4_SYM(*src, color);
                  *dest = INTERPOLATE_PIXEL_256(src_color, const_alpha, *dest, ialpha);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  // 1. load src and dest vector
                  __m128i v_src = _mm_loadu_si128((__m128i *)src);
                  __m128i v_dest = _mm_load_si128((__m128i *)dest);

                  // 2. multiply src color with const_alpha
                  v_src = v4_mul_color_sse2(v_src, v_color);

                  // 3. dest = s * ca + d * cia
                  v_dest = v4_interpolate_color_sse2(v_alpha, v_src, v_dest);

                  _mm_store_si128((__m128i *)dest, v_dest);
                  dest += 4; src +=4; length -= 4;
               })
          }
     }
}

static void
comp_func_source_over_sse2(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
         {
            LOOP_ALIGNED_U1_A4(dest, length,
              { /* UOP */
                 uint s = *src;
                 uint sia = Alpha(~s);
                 *dest = s + BYTE_MUL(*dest, sia);
                 dest++; src++; length--;
              },
              { /* A4OP */
                 
                 // 1. load src and dest vector
                 __m128i v_src = _mm_loadu_si128((__m128i *)src);
                 __m128i v_dest = _mm_load_si128((__m128i *)dest);
                 
                 // 2. dest = src + dest * sia
                 __m128i v_sia = v4_ialpha_sse2(v_src);;
                 v_dest = v4_byte_mul_sse2(v_dest, v_sia);
                 v_dest = _mm_add_epi32(v_dest, v_src);

                 _mm_store_si128((__m128i *)dest, v_dest);
                 dest += 4; src +=4; length -= 4;
              })
         }
        else
         {
            __m128i v_alpha = _mm_set1_epi32(const_alpha);
            LOOP_ALIGNED_U1_A4(dest, length,
              { /* UOP */
                 uint s = BYTE_MUL(*src, const_alpha);
                 uint sia = Alpha(~s);
                 *dest = s + BYTE_MUL(*dest, sia);
                 dest++; src++; length--;
              },
              { /* A4OP */
                 
                 // 1. load src and dest vector
                 __m128i v_src = _mm_loadu_si128((__m128i *)src);
                 __m128i v_dest = _mm_load_si128((__m128i *)dest);
                 
                 // 2. multiply src color with const_alpha
                 v_src = v4_byte_mul_sse2(v_src, v_alpha);

                 // 3. dest = src + dest * sia
                 __m128i v_sia = v4_ialpha_sse2(v_src);;
                 v_dest = v4_byte_mul_sse2(v_dest, v_sia);
                 v_dest = _mm_add_epi32(v_dest, v_src);

                 _mm_store_si128((__m128i *)dest, v_dest);
                 dest += 4; src +=4; length -= 4;
              })
         }
     }
   else
     {
        __m128i v_color = _mm_set1_epi32(color);
        if (const_alpha == 255)
          {
             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  uint s = ECTOR_MUL4_SYM(*src, color);
                  uint sia = Alpha(~s);
                  *dest = s + BYTE_MUL(*dest, sia);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  // 1. load src and dest vector
                  __m128i v_src = _mm_loadu_si128((__m128i *)src);
                  __m128i v_dest = _mm_load_si128((__m128i *)dest);

                  // 2. multiply src color with color multiplier
                  v_src = v4_mul_color_sse2(v_src, v_color);

                  // 3. dest = src + dest * sia
                  __m128i v_sia = v4_ialpha_sse2(v_src);
                  v_dest = v4_byte_mul_sse2(v_dest, v_sia);
                  v_dest = _mm_add_epi32(v_dest, v_src);

                  _mm_store_si128((__m128i *)dest, v_dest);
                  dest += 4; src +=4; length -= 4;
               })
          }
        else
          {
             __m128i v_alpha = _mm_set1_epi32(const_alpha);
             LOOP_ALIGNED_U1_A4(dest, length,
               { /* UOP */
                  uint s = ECTOR_MUL4_SYM(*src, color);
                  s = BYTE_MUL(s, const_alpha);
                  uint sia = Alpha(~s);
                  *dest = s + BYTE_MUL(*dest, sia);
                  dest++; src++; length--;
               },
               { /* A4OP */
                  // 1. load src and dest vector
                  __m128i v_src = _mm_loadu_si128((__m128i *)src);
                  __m128i v_dest = _mm_load_si128((__m128i *)dest);

                  // 2. multiply src color with color multiplier
                  v_src = v4_mul_color_sse2(v_src, v_color);

                  // 3. multiply src color with const_alpha
                  v_src = v4_byte_mul_sse2(v_src, v_alpha);

                  // 4. dest = src + dest * sia
                  __m128i v_sia = v4_ialpha_sse2(v_src);
                  v_dest = v4_byte_mul_sse2(v_dest, v_sia);
                  v_dest = _mm_add_epi32(v_dest, v_src);

                  _mm_store_si128((__m128i *)dest, v_dest);
                  dest += 4; src +=4; length -= 4;
               })
          }
     }
}


void
init_draw_helper_sse2()
{
   // update the comp_function table for solid color
   func_for_mode_solid[ECTOR_ROP_COPY] = comp_func_solid_source_sse2;
   func_for_mode_solid[ECTOR_ROP_BLEND] = comp_func_solid_source_over_sse2;
   
   // update the comp_function table for source data
   func_for_mode[ECTOR_ROP_COPY] = comp_func_source_sse2;
   func_for_mode[ECTOR_ROP_BLEND] = comp_func_source_over_sse2;
}





//#endif