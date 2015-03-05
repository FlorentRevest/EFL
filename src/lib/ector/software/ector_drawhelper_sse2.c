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
byte_mul_sse3(__m128i c, __m128i a)
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

         v_dest = byte_mul_sse3(v_dest, v_a);
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


/*
 * Each 32bits components of alphaChannel must be in the form 0x00AA00AA
 * oneMinusAlphaChannel must be 255 - alpha for each 32 bits component
 * colorMask must have 0x00ff00ff on each 32 bits component
 * half must have the value 128 (0x80) for each 32 bits compnent
 */
// #define INTERPOLATE_PIXEL_255_SSE2(result, srcVector, dstVector, alphaChannel, oneMinusAlphaChannel, colorMask, half) { \
//     /* interpolate AG */\
//     __m128i srcVectorAG = _mm_srli_epi16(srcVector, 8); \
//     __m128i dstVectorAG = _mm_srli_epi16(dstVector, 8); \
//     __m128i srcVectorAGalpha = _mm_mullo_epi16(srcVectorAG, alphaChannel); \
//     __m128i dstVectorAGoneMinusAlphalpha = _mm_mullo_epi16(dstVectorAG, oneMinusAlphaChannel); \
//     __m128i finalAG = _mm_add_epi16(srcVectorAGalpha, dstVectorAGoneMinusAlphalpha); \
//     finalAG = _mm_add_epi16(finalAG, _mm_srli_epi16(finalAG, 8)); \
//     finalAG = _mm_add_epi16(finalAG, half); \
//     finalAG = _mm_andnot_si128(colorMask, finalAG); \
//  \
//     /* interpolate RB */\
//     __m128i srcVectorRB = _mm_and_si128(srcVector, colorMask); \
//     __m128i dstVectorRB = _mm_and_si128(dstVector, colorMask); \
//     __m128i srcVectorRBalpha = _mm_mullo_epi16(srcVectorRB, alphaChannel); \
//     __m128i dstVectorRBoneMinusAlphalpha = _mm_mullo_epi16(dstVectorRB, oneMinusAlphaChannel); \
//     __m128i finalRB = _mm_add_epi16(srcVectorRBalpha, dstVectorRBoneMinusAlphalpha); \
//     finalRB = _mm_add_epi16(finalRB, _mm_srli_epi16(finalRB, 8)); \
//     finalRB = _mm_add_epi16(finalRB, half); \
//     finalRB = _mm_srli_epi16(finalRB, 8); \
//  \
//     /* combine */\
//     result = _mm_or_si128(finalAG, finalRB); \
// }

// static void
// comp_func_source_sse2(uint *dest, const uint *src, int length, uint color, uint const_alpha)
// {
//    if (color == 0xffffffff) // No color multiplier
//      {
//         if (const_alpha == 255)
//           memcpy(dest, src, length * sizeof(uint));
//         else
//          {
//             int ialpha = 255 - const_alpha;
//             const __m128i half = _mm_set1_epi16(0x80);
//             const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
//             const __m128i constAlphaVector = _mm_set1_epi16(const_alpha);
//             const __m128i oneMinusConstAlpha =  _mm_set1_epi16(ialpha);
//             LOOP_ALIGNED_U1_A4(dest, length,
//               { /* UOP */

//                  *dest = INTERPOLATE_PIXEL_256(*src, const_alpha, *dest, ialpha);
//                  dest++; src++; length--;
//               },
//               { /* A4OP */
//                 const __m128i srcVector = _mm_loadu_si128((__m128i *)src);
//                 __m128i dstVector = _mm_load_si128((__m128i *)dest);
//                 INTERPOLATE_PIXEL_255_SSE2(dstVector, srcVector, dstVector, constAlphaVector, oneMinusConstAlpha, colorMask, half)
//                 _mm_store_si128((__m128i *)dest, dstVector);
                 
//                  dest += 4; length -= 4;
//               })
//          }
//      }
//    else
//      {

//      }
// }


void
init_draw_helper_sse2()
{
   func_for_mode_solid[ECTOR_ROP_COPY] = comp_func_solid_source_sse2;
   func_for_mode_solid[ECTOR_ROP_BLEND] = comp_func_solid_source_over_sse2;
   //func_for_mode[ECTOR_ROP_BLEND] = comp_func_source_sse2;
}





//#endif