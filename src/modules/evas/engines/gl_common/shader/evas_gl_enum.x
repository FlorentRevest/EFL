/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */
/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */

typedef enum {
   SHADER_RECT,
   SHADER_RECT_MASK,
   SHADER_FONT,
   SHADER_FONT_MASK,
   SHADER_IMG,
   SHADER_IMG_BGRA,
   SHADER_IMG_12,
   SHADER_IMG_21,
   SHADER_IMG_22,
   SHADER_IMG_12_BGRA,
   SHADER_IMG_21_BGRA,
   SHADER_IMG_22_BGRA,
   SHADER_IMG_MASK,
   SHADER_IMG_BGRA_MASK,
   SHADER_IMG_12_MASK,
   SHADER_IMG_21_MASK,
   SHADER_IMG_22_MASK,
   SHADER_IMG_12_BGRA_MASK,
   SHADER_IMG_21_BGRA_MASK,
   SHADER_IMG_22_BGRA_MASK,
   SHADER_IMG_NOMUL,
   SHADER_IMG_BGRA_NOMUL,
   SHADER_IMG_12_NOMUL,
   SHADER_IMG_21_NOMUL,
   SHADER_IMG_22_NOMUL,
   SHADER_IMG_12_BGRA_NOMUL,
   SHADER_IMG_21_BGRA_NOMUL,
   SHADER_IMG_22_BGRA_NOMUL,
   SHADER_IMG_MASK_NOMUL,
   SHADER_IMG_BGRA_MASK_NOMUL,
   SHADER_IMG_12_MASK_NOMUL,
   SHADER_IMG_21_MASK_NOMUL,
   SHADER_IMG_22_MASK_NOMUL,
   SHADER_IMG_12_BGRA_MASK_NOMUL,
   SHADER_IMG_21_BGRA_MASK_NOMUL,
   SHADER_IMG_22_BGRA_MASK_NOMUL,
   SHADER_IMG_AFILL,
   SHADER_IMG_BGRA_AFILL,
   SHADER_IMG_NOMUL_AFILL,
   SHADER_IMG_BGRA_NOMUL_AFILL,
   SHADER_IMG_12_AFILL,
   SHADER_IMG_21_AFILL,
   SHADER_IMG_22_AFILL,
   SHADER_IMG_12_BGRA_AFILL,
   SHADER_IMG_21_BGRA_AFILL,
   SHADER_IMG_22_BGRA_AFILL,
   SHADER_IMG_12_NOMUL_AFILL,
   SHADER_IMG_21_NOMUL_AFILL,
   SHADER_IMG_22_NOMUL_AFILL,
   SHADER_IMG_12_BGRA_NOMUL_AFILL,
   SHADER_IMG_21_BGRA_NOMUL_AFILL,
   SHADER_IMG_22_BGRA_NOMUL_AFILL,
   SHADER_RGB_A_PAIR,
   SHADER_RGB_A_PAIR_MASK,
   SHADER_RGB_A_PAIR_NOMUL,
   SHADER_RGB_A_PAIR_MASK_NOMUL,
   SHADER_TEX_EXTERNAL,
   SHADER_TEX_EXTERNAL_AFILL,
   SHADER_TEX_EXTERNAL_NOMUL,
   SHADER_TEX_EXTERNAL_NOMUL_AFILL,
   SHADER_TEX_EXTERNAL_MASK,
   SHADER_TEX_EXTERNAL_MASK_NOMUL,
   SHADER_YUV,
   SHADER_YUV_NOMUL,
   SHADER_YUV_MASK,
   SHADER_YUV_MASK_NOMUL,
   SHADER_YUY2,
   SHADER_YUY2_NOMUL,
   SHADER_YUY2_MASK,
   SHADER_YUY2_MASK_NOMUL,
   SHADER_NV12,
   SHADER_NV12_NOMUL,
   SHADER_NV12_MASK,
   SHADER_NV12_MASK_NOMUL,
   SHADER_YUV_709,
   SHADER_YUV_709_NOMUL,
   SHADER_YUV_709_MASK,
   SHADER_YUV_709_MASK_NOMUL,
   SHADER_YUY2_709,
   SHADER_YUY2_709_NOMUL,
   SHADER_YUY2_709_MASK,
   SHADER_YUY2_709_MASK_NOMUL,
   SHADER_NV12_709,
   SHADER_NV12_709_NOMUL,
   SHADER_NV12_709_MASK,
   SHADER_NV12_709_MASK_NOMUL,
   SHADER_LAST
} Evas_GL_Shader;

#ifdef _EVAS_GL_CONTEXT_C

static struct {
   Evas_GL_Shader id;
   const char *tname;
} _shaders_textures[] = {
   { SHADER_FONT_MASK, "tex" },
   { SHADER_FONT_MASK, "texm" },
   { SHADER_IMG_MASK, "tex" },
   { SHADER_IMG_MASK, "texm" },
   { SHADER_IMG_BGRA_MASK, "tex" },
   { SHADER_IMG_BGRA_MASK, "texm" },
   { SHADER_IMG_12_MASK, "tex" },
   { SHADER_IMG_12_MASK, "texm" },
   { SHADER_IMG_21_MASK, "tex" },
   { SHADER_IMG_21_MASK, "texm" },
   { SHADER_IMG_22_MASK, "tex" },
   { SHADER_IMG_22_MASK, "texm" },
   { SHADER_IMG_12_BGRA_MASK, "tex" },
   { SHADER_IMG_12_BGRA_MASK, "texm" },
   { SHADER_IMG_21_BGRA_MASK, "tex" },
   { SHADER_IMG_21_BGRA_MASK, "texm" },
   { SHADER_IMG_22_BGRA_MASK, "tex" },
   { SHADER_IMG_22_BGRA_MASK, "texm" },
   { SHADER_IMG_MASK_NOMUL, "tex" },
   { SHADER_IMG_MASK_NOMUL, "texm" },
   { SHADER_IMG_BGRA_MASK_NOMUL, "tex" },
   { SHADER_IMG_BGRA_MASK_NOMUL, "texm" },
   { SHADER_IMG_12_MASK_NOMUL, "tex" },
   { SHADER_IMG_12_MASK_NOMUL, "texm" },
   { SHADER_IMG_21_MASK_NOMUL, "tex" },
   { SHADER_IMG_21_MASK_NOMUL, "texm" },
   { SHADER_IMG_22_MASK_NOMUL, "tex" },
   { SHADER_IMG_22_MASK_NOMUL, "texm" },
   { SHADER_IMG_12_BGRA_MASK_NOMUL, "tex" },
   { SHADER_IMG_12_BGRA_MASK_NOMUL, "texm" },
   { SHADER_IMG_21_BGRA_MASK_NOMUL, "tex" },
   { SHADER_IMG_21_BGRA_MASK_NOMUL, "texm" },
   { SHADER_IMG_22_BGRA_MASK_NOMUL, "tex" },
   { SHADER_IMG_22_BGRA_MASK_NOMUL, "texm" },
   { SHADER_RGB_A_PAIR, "tex" },
   { SHADER_RGB_A_PAIR, "texa" },
   { SHADER_RGB_A_PAIR_MASK, "tex" },
   { SHADER_RGB_A_PAIR_MASK, "texa" },
   { SHADER_RGB_A_PAIR_MASK, "texm" },
   { SHADER_RGB_A_PAIR_NOMUL, "tex" },
   { SHADER_RGB_A_PAIR_NOMUL, "texa" },
   { SHADER_RGB_A_PAIR_MASK_NOMUL, "tex" },
   { SHADER_RGB_A_PAIR_MASK_NOMUL, "texa" },
   { SHADER_RGB_A_PAIR_MASK_NOMUL, "texm" },
   { SHADER_YUV, "tex" },
   { SHADER_YUV, "texu" },
   { SHADER_YUV, "texv" },
   { SHADER_YUV_NOMUL, "tex" },
   { SHADER_YUV_NOMUL, "texu" },
   { SHADER_YUV_NOMUL, "texv" },
   { SHADER_YUV_MASK, "tex" },
   { SHADER_YUV_MASK, "texu" },
   { SHADER_YUV_MASK, "texv" },
   { SHADER_YUV_MASK, "texm" },
   { SHADER_YUV_MASK_NOMUL, "tex" },
   { SHADER_YUV_MASK_NOMUL, "texu" },
   { SHADER_YUV_MASK_NOMUL, "texv" },
   { SHADER_YUV_MASK_NOMUL, "texm" },
   { SHADER_YUY2, "tex" },
   { SHADER_YUY2, "texuv" },
   { SHADER_YUY2_NOMUL, "tex" },
   { SHADER_YUY2_NOMUL, "texuv" },
   { SHADER_YUY2_MASK, "tex" },
   { SHADER_YUY2_MASK, "texuv" },
   { SHADER_YUY2_MASK, "texm" },
   { SHADER_YUY2_MASK_NOMUL, "tex" },
   { SHADER_YUY2_MASK_NOMUL, "texuv" },
   { SHADER_YUY2_MASK_NOMUL, "texm" },
   { SHADER_NV12, "tex" },
   { SHADER_NV12, "texuv" },
   { SHADER_NV12_NOMUL, "tex" },
   { SHADER_NV12_NOMUL, "texuv" },
   { SHADER_NV12_MASK, "tex" },
   { SHADER_NV12_MASK, "texuv" },
   { SHADER_NV12_MASK, "texm" },
   { SHADER_NV12_MASK_NOMUL, "tex" },
   { SHADER_NV12_MASK_NOMUL, "texuv" },
   { SHADER_NV12_MASK_NOMUL, "texm" },
   { SHADER_YUV_709, "tex" },
   { SHADER_YUV_709, "texu" },
   { SHADER_YUV_709, "texv" },
   { SHADER_YUV_709_NOMUL, "tex" },
   { SHADER_YUV_709_NOMUL, "texu" },
   { SHADER_YUV_709_NOMUL, "texv" },
   { SHADER_YUV_709_MASK, "tex" },
   { SHADER_YUV_709_MASK, "texu" },
   { SHADER_YUV_709_MASK, "texv" },
   { SHADER_YUV_709_MASK, "texm" },
   { SHADER_YUV_709_MASK_NOMUL, "tex" },
   { SHADER_YUV_709_MASK_NOMUL, "texu" },
   { SHADER_YUV_709_MASK_NOMUL, "texv" },
   { SHADER_YUV_709_MASK_NOMUL, "texm" },
   { SHADER_YUY2_709, "tex" },
   { SHADER_YUY2_709, "texuv" },
   { SHADER_YUY2_709_NOMUL, "tex" },
   { SHADER_YUY2_709_NOMUL, "texuv" },
   { SHADER_YUY2_709_MASK, "tex" },
   { SHADER_YUY2_709_MASK, "texuv" },
   { SHADER_YUY2_709_MASK, "texm" },
   { SHADER_YUY2_709_MASK_NOMUL, "tex" },
   { SHADER_YUY2_709_MASK_NOMUL, "texuv" },
   { SHADER_YUY2_709_MASK_NOMUL, "texm" },
   { SHADER_NV12_709, "tex" },
   { SHADER_NV12_709, "texuv" },
   { SHADER_NV12_709_NOMUL, "tex" },
   { SHADER_NV12_709_NOMUL, "texuv" },
   { SHADER_NV12_709_MASK, "tex" },
   { SHADER_NV12_709_MASK, "texuv" },
   { SHADER_NV12_709_MASK, "texm" },
   { SHADER_NV12_709_MASK_NOMUL, "tex" },
   { SHADER_NV12_709_MASK_NOMUL, "texuv" },
   { SHADER_NV12_709_MASK_NOMUL, "texm" },
   { SHADER_LAST, NULL }
};

#endif // _EVAS_GL_CONTEXT_C
