#ifndef ECTOR_DRAWHELPER_PRIVATE_H
#define ECTOR_DRAWHELPER_PRIVATE_H

#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef uint
typedef unsigned int uint;
#endif


#define ECTOR_ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

#define ECTOR_MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define BYTE_MUL(c, a) \
 ( (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + \
   (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff) )


static inline void
_ector_memfill(uint *dest, int length, uint value)
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

static inline uint 
INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b)
{
   uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
   t >>= 8;
   t &= 0xff00ff;
   x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
   x &= 0xff00ff00;
   x |= t;
   return x;
}

typedef void (*RGBA_Comp_Func)(uint *dest, const uint *src, int length, uint mul_col, uint const_alpha);
typedef void (*RGBA_Comp_Func_Solid)(uint *dest, int length, uint color, uint const_alpha);

RGBA_Comp_Func_Solid ector_comp_func_solid_span_get(Ector_Rop op, uint color);
RGBA_Comp_Func ector_comp_func_span_get(Ector_Rop op, uint color);

#endif