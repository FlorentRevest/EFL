/* @file evas_image_load_dds.c
 * @author Jean-Philippe ANDRE <jpeg@videolan.org>
 *
 * Load Microsoft DirectDraw Surface files.
 * Decode S3TC image format.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Evas_Loader.h"
#include "s3tc.h"

#ifdef _WIN32
# include <ddraw.h>
#endif

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;

   DxtFormat format;
   size_t data_size;

   struct {
      uint32_t flags;
      uint32_t fourcc;
      uint32_t rgb_bitcount;
      uint32_t r_mask;
      uint32_t g_mask;
      uint32_t b_mask;
      uint32_t a_mask;

      Eina_Bool has_alpha : 1;
      // TODO: check mipmaps to load faster a small image :)
   } pf; // pixel format
};

#undef FOURCC
#ifndef WORDS_BIGENDIAN
# define FOURCC(a,b,c,d) ((d << 24) | (c << 16) | (b << 8) | a)
#else
# define FOURCC(a,b,c,d) ((a << 24) | (b << 16) | (c << 8) | d)
#endif

#ifndef DIRECTDRAW_VERSION
// DIRECTDRAW_VERSION is defined in ddraw.h
// These definitions are from the MSDN reference.

enum DDSFlags {
   DDSD_CAPS = 0x1,
   DDSD_HEIGHT = 0x2,
   DDSD_WIDTH = 0x4,
   DDSD_PITCH = 0x8,
   DDSD_PIXELFORMAT = 0x1000,
   DDSD_MIPMAPCOUNT = 0x20000,
   DDSD_LINEARSIZE = 0x80000,
   DDSD_DEPTH = 0x800000
};

enum DDSPixelFormatFlags {
   DDPF_ALPHAPIXELS = 0x1,
   DDPF_ALPHA = 0x2,
   DDPF_FOURCC = 0x4,
   DDPF_RGB = 0x40,
   DDPF_YUV = 0x200,
   DDPF_LUMINANCE = 0x20000
};

enum DDSCaps {
   DDSCAPS_COMPLEX = 0x8,
   DDSCAPS_MIPMAP = 0x400000,
   DDSCAPS_TEXTURE = 0x1000
};

#endif

static const Evas_Colorspace cspaces_s3tc_dxt1_rgb[] = {
   //EVAS_COLORSPACE_RGB_S3TC_DXT1,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt1_rgba[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT1,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt2[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT2, // Not in OpenGL
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt3[] = {
   //EVAS_COLORSPACE_RGB_S3TC_DXT3,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt4[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT4, // Not in OpenGL
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt5[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT5,
   EVAS_COLORSPACE_ARGB8888
};

static void *
evas_image_load_file_open_dds(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   if (eina_file_size_get(f) <= 128)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }


   loader->f = eina_file_dup(f);
   if (!loader->f)
     {
        free(loader);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   return loader;
}

static void
evas_image_load_file_close_dds(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eina_file_close(loader->f);
   free(loader);
}

static inline uint32_t
_dword_read(const char **m)
{
   uint32_t val = *((uint32_t *) *m);
   *m += 4;
   return val;
}

#define FAIL() do { fprintf(stderr, "DDS: ERROR at %s:%d", __FUNCTION__, __LINE__); goto on_error; } while (0)

static Eina_Bool
evas_image_load_file_head_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   static const uint32_t base_flags = /* 0x1007 */
         DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

   Evas_Loader_Internal *loader = loader_data;
   uint32_t flags, height, width, pitchOrLinearSize, block_size = 16, stride,
         caps, caps2;
   Eina_Bool has_linearsize, has_mipmapcount;
   const char *m;

   m = eina_file_map_all(loader->f, EINA_FILE_SEQUENTIAL);
   if (!m)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (strncmp(m, "DDS ", 4) != 0)
     // TODO: Add support for DX10
     FAIL();
   m += 4;

   // Read DDS_HEADER
   if (_dword_read(&m) != 124)
     FAIL();

   flags = _dword_read(&m);
   if ((flags & base_flags) != (base_flags))
     FAIL();

   if ((flags & ~(DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE)) != base_flags)
     {
        // TODO: A lot of modes are not supported.
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }

   has_linearsize = !!(flags & DDSD_LINEARSIZE);
   if (!has_linearsize)
     FAIL();

   has_mipmapcount = !!(flags & DDSD_MIPMAPCOUNT);
   (void) has_mipmapcount; // We don't really care about it.

   height = _dword_read(&m);
   width = _dword_read(&m);
   pitchOrLinearSize = _dword_read(&m);
   if (!width || !height || (width & 0x3) || (height & 0x3))
     FAIL();

   // Skip depth & mipmap count + reserved[11]
   m += 13 * sizeof(uint32_t);
   // Entering DDS_PIXELFORMAT ddspf
   if (_dword_read(&m) != 32)
     FAIL();
   loader->pf.flags = _dword_read(&m);
   if (!(loader->pf.flags & DDPF_FOURCC))
     FAIL(); // Unsupported (uncompressed formats may not have a FOURCC)
   loader->pf.fourcc = _dword_read(&m);
   loader->pf.has_alpha = EINA_TRUE;
   switch (loader->pf.fourcc)
     {
      case FOURCC('D', 'X', 'T', '1'):
        loader->format = DXT1;
        if ((loader->pf.flags & DDPF_ALPHAPIXELS) == 0)
          {
             prop->alpha = EINA_FALSE;
             prop->cspaces = cspaces_s3tc_dxt1_rgb;
             loader->pf.has_alpha = EINA_FALSE;
          }
        else
          {
             prop->alpha = EINA_TRUE;
             prop->cspaces = cspaces_s3tc_dxt1_rgba;
          }
        block_size = 8;
        break;
#if 0
      case FOURCC('D', 'X', 'T', '2'):
        loader->format = DXT2;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt2;
        break;
      case FOURCC('D', 'X', 'T', '3'):
        loader->format = DXT3;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt5;
        break;
      case FOURCC('D', 'X', 'T', '4'):
        loader->format = DXT4;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt4;
        break;
      case FOURCC('D', 'X', 'T', '5'):
        loader->format = DXT5;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt5;
        break;
      case FOURCC('D', 'X', '1', '0'):
        loader->format = DX10;
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
#endif
      default:
        // TODO: Implement decoding support for uncompressed formats
        FAIL();
     }
   loader->pf.rgb_bitcount = _dword_read(&m);
   loader->pf.r_mask = _dword_read(&m);
   loader->pf.g_mask = _dword_read(&m);
   loader->pf.b_mask = _dword_read(&m);
   loader->pf.a_mask = _dword_read(&m);
   caps = _dword_read(&m);
   if ((caps & DDSCAPS_TEXTURE) == 0)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }
   caps2 = _dword_read(&m);
   if (caps2 != 0)
     {
        // Cube maps not supported
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }
   // Since the rest is unused, just ignore it.

   stride = ((width + 3) >> 2) * block_size;
   loader->data_size = stride * ((height + 3) >> 2);
   if (loader->data_size != pitchOrLinearSize)
     FAIL(); // Invalid size!

   // Check file size
   if (eina_file_size_get(loader->f) < (128 + loader->data_size))
     FAIL();

   prop->h = height;
   prop->w = width;
   *error = EVAS_LOAD_ERROR_NONE;

on_error:
   eina_file_map_free(loader->f, (void *) m);
   return (*error == EVAS_LOAD_ERROR_NONE);
}

Eina_Bool
evas_image_load_file_data_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   const char *m;

   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   m = eina_file_map_all(loader->f, EINA_FILE_WILLNEED);
   if (!m) return EINA_FALSE;

   // TODO

   *error = EVAS_LOAD_ERROR_GENERIC;
   r = EINA_FALSE; // FIXME

on_error:
   eina_file_map_free(loader->f, m);
   return r;
}

Evas_Image_Load_Func evas_image_load_dds_func =
{
  evas_image_load_file_open_dds,
  evas_image_load_file_close_dds,
  evas_image_load_file_head_dds,
  evas_image_load_file_data_dds,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_dds_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "dds",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, dds);

#ifndef EVAS_STATIC_BUILD_DDS
EVAS_EINA_MODULE_DEFINE(image_loader, dds);
#endif
