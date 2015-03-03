#ifndef ECTOR_BLEND_PRIVATE_H
#define ECTOR_BLEND_PRIVATE_H

#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

#define ECTOR_ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

#define ECTOR_MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define ECTOR_MUL_256(c, a) \
 ( (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + \
   (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff) )


static inline void
_ector_memfill(DATA32 *dest, int length, uint value)
{
    if (!length)
        return;

    int n = (length + 7) / 8;
    switch (length & 0x07)
    {
    case 0: do { *dest++ = value;
    case 7:      *dest++ = value;
    case 6:      *dest++ = value;
    case 5:      *dest++ = value;
    case 4:      *dest++ = value;
    case 3:      *dest++ = value;
    case 2:      *dest++ = value;
    case 1:      *dest++ = value;
    } while (--n > 0);
    }
}

inline static void
_ector_comp_func_solid_source_copy(uint *dest, int length, uint color, uint const_alpha)
{
   if (const_alpha == 255) _ector_memfill(dest, length, color);
   else
     {
        int ialpha, i = 0;
        uint c = ECTOR_MUL_256(color, const_alpha);
        ialpha = 255 - const_alpha;
        for (; i < length; ++i)
          dest[i] = c + ECTOR_MUL_256(dest[i], ialpha);
     }
}

inline static void
_ector_comp_func_solid_source_blend(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha, i = 0;
   uint c = ECTOR_MUL_256(color, const_alpha);
   ialpha = (~c) >> 24;
   for (; i < length; ++i)
     dest[i] = c + ECTOR_MUL_256(dest[i], ialpha);
}

static inline void 
_ector_comp_func_source_over_c(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            DATA32 sc = ECTOR_MUL4_SYM(color, s);
            uint a = (~sc) >> 24;
            dest[i] = sc + ECTOR_MUL_256(dest[i], a);
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            DATA32 sc = ECTOR_MUL4_SYM(color, s);
            sc = ECTOR_MUL_256(sc, const_alpha);
            uint a = (~sc) >> 24;
            dest[i] = sc + ECTOR_MUL_256(dest[i], a);
        }
    }
}


static inline void 
_ector_comp_func_source_over(uint *dest, const uint *src, int length, uint color EINA_UNUSED, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            if (s >= 0xff000000)
                dest[i] = s;
            else if (s != 0) {
                uint a = (~s) >> 24;
                dest[i] = s + ECTOR_MUL_256(dest[i], a);
            }
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint s = ECTOR_MUL_256(src[i], const_alpha);
            uint a = (~s) >> 24;
            dest[i] = s + ECTOR_MUL_256(dest[i], a);
        }
    }
}

static inline uint 
INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}


#endif