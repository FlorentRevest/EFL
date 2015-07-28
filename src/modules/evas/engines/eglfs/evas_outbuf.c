#include "evas_engine.h"

/* local variables */
static Outbuf *_evas_eglfs_window = NULL;
static EGLContext context = EGL_NO_CONTEXT;
static int win_count = 0;

static Eina_Bool
_evas_outbuf_make_current(void *data, void *doit)
{
   Outbuf *ob;

   if (!(ob = data)) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                            ob->egl.surface[0], ob->egl.context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE,
                            EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_evas_outbuf_egl_setup(Outbuf *ob)
{
   int ctx_attr[3];
   int cfg_attr[40];
   int maj = 0, min = 0, n = 0, i = 0;
   EGLint ncfg;
   EGLConfig *cfgs;
   const GLubyte *vendor, *renderer, *version, *glslversion;
   Eina_Bool blacklist = EINA_FALSE;

   /* setup egl surface */
   ctx_attr[0] = EGL_CONTEXT_CLIENT_VERSION;
   ctx_attr[1] = 2;
   ctx_attr[2] = EGL_NONE;

   cfg_attr[n++] = EGL_BUFFER_SIZE;
   cfg_attr[n++] = 32;
   cfg_attr[n++] = EGL_DEPTH_SIZE;
   cfg_attr[n++] = EGL_DONT_CARE;
   cfg_attr[n++] = EGL_STENCIL_SIZE;
   cfg_attr[n++] = EGL_DONT_CARE;
   cfg_attr[n++] = EGL_RENDERABLE_TYPE;
   cfg_attr[n++] = EGL_OPENGL_ES2_BIT;
   cfg_attr[n++] = EGL_SURFACE_TYPE;
   cfg_attr[n++] = EGL_WINDOW_BIT;

   cfg_attr[n++] = EGL_ALPHA_SIZE;
   if (ob->destination_alpha) cfg_attr[n++] = 1;
   else cfg_attr[n++] = 0;
   cfg_attr[n++] = EGL_NONE;

   ob->egl.disp = eglGetDisplay(NULL);
   if (ob->egl.disp  == EGL_NO_DISPLAY)
     {
        ERR("eglGetDisplay() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (!eglInitialize(ob->egl.disp, &maj, &min))
     {
        ERR("eglInitialize() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS)
     {
        ERR("eglBindAPI() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (!eglGetConfigs(ob->egl.disp, NULL, 0, &ncfg) || (ncfg == 0))
     {
        ERR("eglGetConfigs() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   cfgs = malloc(ncfg * sizeof(EGLConfig));
   if (!cfgs)
     {
        ERR("Failed to malloc space for egl configs");
        return EINA_FALSE;
     }

   if (!eglChooseConfig(ob->egl.disp, cfg_attr, cfgs,
                        ncfg, &ncfg) || (ncfg == 0))
     {
        ERR("eglChooseConfig() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   // First is always best...
   ob->egl.config = cfgs[0];

   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)NULL, NULL);
   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            NULL, eglGetError());
        return EINA_FALSE;
     }

   ob->egl.context[0] =
     eglCreateContext(ob->egl.disp, ob->egl.config, EGL_NO_CONTEXT, ctx_attr);
   if (ob->egl.context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (context == EGL_NO_CONTEXT) context = ob->egl.context[0];

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                      ob->egl.surface[0], ob->egl.context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
   if (!vendor)   vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version)  version  = (unsigned char *)"-UNKNOWN-";
   if (!glslversion) glslversion = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor  : %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version : %s\n", version);
        fprintf(stderr, "glsl ver: %s\n", glslversion);
     }

   if (strstr((const char *)vendor, "Mesa Project"))
     {
        if (strstr((const char *)renderer, "Software Rasterizer"))
          blacklist = EINA_TRUE;
     }
   if (strstr((const char *)renderer, "softpipe"))
     blacklist = EINA_TRUE;
   if (strstr((const char *)renderer, "llvmpipe"))
     blacklist = EINA_TRUE;

   if ((blacklist) && (!getenv("EVAS_GL_NO_BLACKLIST")))
     {
        ERR("OpenGL Driver blacklisted:");
        ERR("Vendor: %s", (const char *)vendor);
        ERR("Renderer: %s", (const char *)renderer);
        ERR("Version: %s", (const char *)version);
        return EINA_FALSE;
     }

   ob->gl_context = glsym_evas_gl_common_context_new();
   if (!ob->gl_context) return EINA_FALSE;

#ifdef GL_GLES
   ob->gl_context->egldisp = ob->egl.disp;
   ob->gl_context->eglctxt = ob->egl.context[0];
#endif

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h, ob->rotation);

   ob->surf = EINA_TRUE;

   return EINA_TRUE;
}

Outbuf *
evas_outbuf_new(Evas_Engine_Info_Eglfs *info, int w, int h, Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *ob;
   char *num;

   if (!info) return NULL;

   /* try to allocate space for outbuf */
   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   win_count++;

   ob->w = w;
   ob->h = h;
   ob->info = info;
   ob->depth = info->info.depth;
   ob->rotation = info->info.rotation;
   ob->destination_alpha = info->info.destination_alpha;
   ob->swap_mode = swap_mode;
   ob->priv.num = 2;

   if ((num = getenv("EVAS_EGLFS_BUFFERS")))
     {
        ob->priv.num = atoi(num);
        if (ob->priv.num <= 0) ob->priv.num = 1;
        else if (ob->priv.num > 4) ob->priv.num = 4;
     }

   if ((num = getenv("EVAS_EGLFS_VSYNC")))
     ob->vsync = atoi(num);

   if (!_evas_outbuf_egl_setup(ob))
     {
        evas_outbuf_free(ob);
        return NULL;
     }

   return ob;
}

void
evas_outbuf_free(Outbuf *ob)
{
   int ref = 0;

   win_count--;
   evas_outbuf_use(ob);

   if (ob == _evas_eglfs_window) _evas_eglfs_window = NULL;

   if (ob->gl_context)
     {
        ref = ob->gl_context->references - 1;
        glsym_evas_gl_common_context_free(ob->gl_context);
     }

   eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (ob->egl.context[0] != context)
     eglDestroyContext(ob->egl.disp, ob->egl.context[0]);

   if (ob->egl.surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);

   if (ref == 0)
     {
        if (context) eglDestroyContext(ob->egl.disp, context);
        eglTerminate(ob->egl.disp);
        eglReleaseThread();
        context = EGL_NO_CONTEXT;
     }

   free(ob);
}

void
evas_outbuf_use(Outbuf *ob)
{
   Eina_Bool force = EINA_FALSE;

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);

   if (_evas_eglfs_window)
     {
        if (eglGetCurrentContext() != _evas_eglfs_window->egl.context[0])
          force = EINA_TRUE;
     }

   if ((_evas_eglfs_window != ob) || (force))
     {
        if (_evas_eglfs_window)
          {
             glsym_evas_gl_common_context_use(_evas_eglfs_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_eglfs_window->gl_context);
          }

        _evas_eglfs_window = ob;

        if (ob)
          {
             if (ob->egl.surface[0] != EGL_NO_SURFACE)
               {
                  if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                                     ob->egl.surface[0],
                                     ob->egl.context[0]) == EGL_FALSE)
                    ERR("eglMakeCurrent() failed!");
               }
          }
     }

   if (ob) glsym_evas_gl_common_context_use(ob->gl_context);
}

void
evas_outbuf_resurf(Outbuf *ob)
{
   if (ob->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", ob);

   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)NULL, NULL);

   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            NULL, eglGetError());
        return;
     }

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0], ob->egl.surface[0],
                      ob->egl.context[0]) == EGL_FALSE)
     ERR("eglMakeCurrent() failed!");

   ob->surf = EINA_TRUE;
}

void
evas_outbuf_unsurf(Outbuf *ob)
{
   if (!ob->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", ob);

   if (_evas_eglfs_window)
      glsym_evas_gl_common_context_flush(_evas_eglfs_window->gl_context);
   if (_evas_eglfs_window == ob)
     {
        eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (ob->egl.surface[0] != EGL_NO_SURFACE)
           eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);
        ob->egl.surface[0] = EGL_NO_SURFACE;

        _evas_eglfs_window = NULL;
     }

   ob->surf = EINA_FALSE;
}

void
evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth)
{
   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;

   ob->w = w;
   ob->h = h;
   ob->depth = depth;
   ob->rotation = rot;

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

Render_Engine_Swap_Mode
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   return MODE_FULL;
   // Forces re-rendering all the screen, that is bad for performance. However
   // partial rendering makes black area. We should try to find a better solution.
}

int
evas_outbuf_rot_get(Outbuf *ob)
{
   return ob->rotation;
}

Eina_Bool
evas_outbuf_update_region_first_rect(Outbuf *ob)
{
   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);
   evas_outbuf_use(ob);

   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);

   return EINA_FALSE;
}

void *
evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx EINA_UNUSED, int *cy EINA_UNUSED, int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if ((w == ob->w) && (h == ob->h))
     ob->gl_context->master_clip.enabled = EINA_FALSE;
   else
     {
        ob->gl_context->master_clip.enabled = EINA_TRUE;
        ob->gl_context->master_clip.x = x;
        ob->gl_context->master_clip.y = y;
        ob->gl_context->master_clip.w = w;
        ob->gl_context->master_clip.h = h;
     }

   return ob->gl_context->def_surface;
}

void
evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   /* Is it really necessary to flush per region ? Shouldn't we be able to
      still do that for the full canvas when doing partial update */
   if (!_re_wincheck(ob)) return;
   ob->drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   /* Nothing to do here as we don't really create an image per area */
}

void
evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects EINA_UNUSED, Evas_Render_Mode render_mode)
{
   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(ob)) goto end;
   if (!ob->drew) goto end;

   ob->drew = EINA_FALSE;
   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   if (!ob->vsync)
     {
        if (ob->info->info.vsync) eglSwapInterval(ob->egl.disp, 1);
        else eglSwapInterval(ob->egl.disp, 0);
        ob->vsync = 1;
     }

   if (ob->info->callback.pre_swap)
     ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);

   eglSwapBuffers(ob->egl.disp, ob->egl.surface[0]);

   if (ob->info->callback.post_swap)
     ob->info->callback.post_swap(ob->info->callback.data, ob->evas);

   ob->priv.frame_cnt++;

end:
   glsym_evas_gl_preload_render_unlock(_evas_outbuf_make_current, ob);
}

Evas_Engine_GL_Context *
evas_outbuf_gl_context_get(Outbuf *ob)
{
   return ob->gl_context;
}

void *
evas_outbuf_egl_display_get(Outbuf *ob)
{
   return ob->egl.disp;
}

Context_3D *
evas_outbuf_gl_context_new(Outbuf *ob)
{
   Context_3D *ctx;
   int context_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   if (!ob) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

   ctx->context = eglCreateContext(ob->egl.disp, ob->egl.config,
                                   ob->egl.context[0], context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = ob->egl.disp;
   ctx->surface = ob->egl.surface[0];

   return ctx;

error:
   free(ctx);
   return NULL;
}

void
evas_outbuf_gl_context_free(Context_3D *ctx)
{
   eglDestroyContext(ctx->display, ctx->context);
   free(ctx);
}

void
evas_outbuf_gl_context_use(Context_3D *ctx)
{
   if (eglMakeCurrent(ctx->display, ctx->surface,
                      ctx->surface, ctx->context) == EGL_FALSE)
     ERR("eglMakeCurrent() failed.");
}
