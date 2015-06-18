#include "evas_common_private.h"
#include "evas_private.h"

//#define LYDBG(f, args...) printf(f, ##args)
#define LYDBG(f, args...)

#define MY_CLASS EVAS_TEXTBLOCK_EXTENSION_CLASS

#define MY_CLASS_NAME "Evas_Textblock_Extension"

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock_extension";

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)


/* private struct for textblock extension object internal data */
/**
 * @internal
 * @typedef Evas_Textblock_Extension_Data
 * The actual textblock extension object.
 */
typedef struct _Evas_Object_Textblock_Extension    Evas_Textblock_Extension_Data;

/* private methods for textblock_extension extension objects */
static unsigned int evas_object_textblock_extension_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_textblock_extension_visual_id_get(Evas_Object *eo_obj);
// static void *evas_object_textblock_extension_engine_data_get(Evas_Object *eo_obj);
static void evas_object_textblock_extension_init(Evas_Object *eo_obj);
static void evas_object_textblock_extension_render(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data,
					 void *output, void *context, void *surface,
					 int x, int y, Eina_Bool do_async);
static void evas_object_textblock_extension_render_pre(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);
static void evas_object_textblock_extension_render_post(Evas_Object *eo_obj,
					      Evas_Object_Protected_Data *obj,
					      void *type_private_data);
static int evas_object_textblock_extension_is_opaque(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);
static int evas_object_textblock_extension_was_opaque(Evas_Object *eo_obj,
					    Evas_Object_Protected_Data *obj,
					    void *type_private_data);
static void evas_object_textblock_extension_scale_update(Evas_Object *eo_obj,
					       Evas_Object_Protected_Data *obj,
					       void *type_private_data);


static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
     evas_object_textblock_extension_render,
     evas_object_textblock_extension_render_pre,
     evas_object_textblock_extension_render_post,
     evas_object_textblock_extension_id_get,
     evas_object_textblock_extension_visual_id_get,
     NULL, /* evas_object_textblock_extension_engine_data_get */
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_textblock_extension_is_opaque,
     evas_object_textblock_extension_was_opaque,
     NULL,
     NULL,
     NULL, /* evas_object_textblock_extension_coords_recalc <- disable - not useful. */
     evas_object_textblock_extension_scale_update,
     NULL,
     NULL,
     NULL
};

/* Size of the index array */
#define TEXTBLOCK_PAR_INDEX_SIZE 10
struct _Evas_Object_Textblock_Extension
{
   void                               *engine_data;
   struct {
      int                              w, h, oneline_h;
      Eina_Bool                        valid : 1;
   } formatted, native;
   Eina_Bool                           redraw : 1;
   Eina_Bool                           changed : 1;
   Eina_Bool                           done : 1;
};

static void evas_object_textblock_extension_init(Evas_Object *eo_obj)
{
   // Left empty for now
}

EOLIAN static void
_evas_textblock_extension_eo_base_destructor(Eo *eo_obj, Evas_Textblock_Extension_Data *o EINA_UNUSED)
{
   // evas_object_textblock_extension_free(eo_obj);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

EOLIAN static void
_evas_textblock_extension_eo_base_dbg_info_get(Eo *eo_obj, Evas_Textblock_Extension_Data *o EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   if (!root) return;
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
#if 0
   Eo_Dbg_Info *node;
   const char *style;
   const char *text = NULL;
   char shorttext[48];
   const Evas_Textblock_Style *ts = NULL;

   eo_do(eo_obj, ts = evas_obj_textblock_style_get());
   style = evas_textblock_style_get(ts);
   eo_do(eo_obj, text = evas_obj_textblock_text_markup_get());
   strncpy(shorttext, text, 38);
   if (shorttext[37])
     strcpy(shorttext + 37, "\xe2\x80\xa6"); /* HORIZONTAL ELLIPSIS */

   EO_DBG_INFO_APPEND(group, "Style", EINA_VALUE_TYPE_STRING, style);
   EO_DBG_INFO_APPEND(group, "Text", EINA_VALUE_TYPE_STRING, shorttext);

     {
        int w, h;
        eo_do(eo_obj, evas_obj_textblock_size_formatted_get(&w, &h));
        node = EO_DBG_INFO_LIST_APPEND(group, "Formatted size");
        EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }

     {
        int w, h;
        eo_do(eo_obj, evas_obj_textblock_size_native_get(&w, &h));
        node = EO_DBG_INFO_LIST_APPEND(group, "Native size");
        EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }
#endif
}

static void
evas_object_textblock_extension_render_pre(Evas_Object *eo_obj,
				 Evas_Object_Protected_Data *obj,
				 void *type_private_data)
{
   Evas_Textblock_Extension_Data *o = type_private_data;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;

   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw textblocks */
#if 0
   evas_object_textblock_coords_recalc(eo_obj, obj, obj->private_data);
#endif
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          evas_object_clip_recalc(obj->cur->clipper);
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
                                            obj->cur->clipper,
                                            obj->cur->clipper->private_data);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj, obj);
   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes,
                                              eo_obj, is_v, was_v);
        goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes,
                                         eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   if (obj->cur->render_op != obj->prev->render_op)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes,
                                         eo_obj, is_v, was_v);
}

void
evas_object_textblock_extension_render_master(
      Evas_Object *eo_obj EINA_UNUSED,
      Evas_Object_Protected_Data *obj,
      void *type_private_data,
      void *output, void *context, void *surface,
      int x, int y, Eina_Bool do_async);

static void
evas_object_textblock_extension_render(Evas_Object *eo_obj EINA_UNUSED,
			     Evas_Object_Protected_Data *obj,
			     void *type_private_data,
			     void *output, void *context, void *surface,
			     int x, int y, Eina_Bool do_async)
{
   evas_object_textblock_extension_render_master(eo_obj, obj, type_private_data,
         output, context, surface, x, y, do_async);
}

static void
evas_object_textblock_extension_render_post(Evas_Object *eo_obj,
                                  Evas_Object_Protected_Data *obj EINA_UNUSED,
                                  void *type_private_data EINA_UNUSED)
{
   evas_object_clip_changes_clean(eo_obj);
   evas_object_cur_prev(eo_obj);
}

static unsigned int evas_object_textblock_extension_id_get(Evas_Object *eo_obj)
{
   Evas_Textblock_Extension_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_TEXTBLOCK;
}

static unsigned int evas_object_textblock_extension_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Textblock_Extension_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_CUSTOM;
}

static int
evas_object_textblock_extension_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                Evas_Object_Protected_Data *obj EINA_UNUSED,
                                void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_textblock_extension_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                 Evas_Object_Protected_Data *obj EINA_UNUSED,
                                 void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static void
evas_object_textblock_extension_scale_update(Evas_Object *eo_obj EINA_UNUSED,
                                   Evas_Object_Protected_Data *obj EINA_UNUSED,
                                   void *type_private_data)
{
#if 0
   Evas_Textblock_Data *o = type_private_data;
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, eo_obj);
   o->last_w = -1;
   o->last_h = -1;
#endif
}

EOLIAN static Eo *
_evas_textblock_extension_eo_base_constructor(Eo *eo_obj, Evas_Textblock_Extension_Data *class_data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   eo_obj = eo_do_super_ret(eo_obj, MY_CLASS, eo_obj, eo_constructor());

   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   evas_object_textblock_extension_init(eo_obj);

   return eo_obj;
}


#include "canvas/evas_textblock_extension.eo.c"
