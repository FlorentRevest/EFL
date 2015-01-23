
#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <string.h>
#include <math.h>

#define MY_CLASS EVAS_VG_NODE_CLASS

static Eina_Bool
_evas_vg_node_property_changed(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Evas_VG_Node_Data *pd = data;
   Eo *parent;

   if (pd->changed) return EINA_TRUE;
   pd->changed = EINA_TRUE;

   parent = eo_do(obj, eo_parent_get());
   eo_do(parent, eo_event_callback_call(desc, event_info));
   return EINA_TRUE;
}

void
_evas_vg_node_transformation_set(Eo *obj,
                                 Evas_VG_Node_Data *pd,
                                 const Eina_Matrix3 *m)
{
   if (!pd->m)
     {
        pd->m = malloc(sizeof (Eina_Matrix3));
        if (!pd->m) return ;
     }
   memcpy(pd->m, m, sizeof (Eina_Matrix3));

   _evas_vg_node_changed(obj);
}

const Eina_Matrix3 *
_evas_vg_node_transformation_get(Eo *obj EINA_UNUSED, Evas_VG_Node_Data *pd)
{
   return pd->m;
}

void
_evas_vg_node_origin_set(Eo *obj,
                         Evas_VG_Node_Data *pd,
                         double x, double y)
{
   pd->x = x;
   pd->y = y;

   _evas_vg_node_changed(obj);
}

void
_evas_vg_node_origin_get(Eo *obj EINA_UNUSED,
                         Evas_VG_Node_Data *pd,
                         double *x, double *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

void
_evas_vg_node_efl_gfx_base_position_set(Eo *obj EINA_UNUSED,
                                        Evas_VG_Node_Data *pd,
                                        int x, int y)
{
   pd->x = lrint(x);
   pd->y = lrint(y);

   _evas_vg_node_changed(obj);
}

void
_evas_vg_node_efl_gfx_base_position_get(Eo *obj EINA_UNUSED,
                                        Evas_VG_Node_Data *pd,
                                        int *x, int *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

void
_evas_vg_node_efl_gfx_base_visible_set(Eo *obj EINA_UNUSED,
                                       Evas_VG_Node_Data *pd, Eina_Bool v)
{
   pd->visibility = v;

   _evas_vg_node_changed(obj);
}


Eina_Bool
_evas_vg_node_efl_gfx_base_visible_get(Eo *obj EINA_UNUSED,
                                       Evas_VG_Node_Data *pd)
{
   return pd->visibility;
}

void
_evas_vg_node_efl_gfx_base_color_set(Eo *obj EINA_UNUSED,
                                     Evas_VG_Node_Data *pd,
                                     int r, int g, int b, int a)
{
   pd->r = r;
   pd->g = g;
   pd->b = b;
   pd->a = a;

   _evas_vg_node_changed(obj);
}

Eina_Bool
_evas_vg_node_efl_gfx_base_color_part_set(Eo *obj, Evas_VG_Node_Data *pd,
                                          const char *part,
                                          int r, int g, int b, int a)
{
   if (part) return EINA_FALSE;

   _evas_vg_node_efl_gfx_base_color_set(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

void
_evas_vg_node_efl_gfx_base_color_get(Eo *obj EINA_UNUSED,
                                     Evas_VG_Node_Data *pd,
                                     int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->r;
   if (g) *g = pd->g;
   if (b) *b = pd->b;
   if (a) *a = pd->a;
}

Eina_Bool
_evas_vg_node_efl_gfx_base_color_part_get(Eo *obj, Evas_VG_Node_Data *pd,
                                          const char *part,
                                          int *r, int *g, int *b, int *a)
{
   if (part) return EINA_FALSE;

   _evas_vg_node_efl_gfx_base_color_get(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

void
_evas_vg_node_mask_set(Eo *obj EINA_UNUSED,
                       Evas_VG_Node_Data *pd,
                       Evas_VG_Node *r)
{
   Evas_VG_Node *tmp = pd->mask;

   pd->mask = eo_ref(r);
   eo_unref(tmp);

   _evas_vg_node_changed(obj);
}

Evas_VG_Node*
_evas_vg_node_mask_get(Eo *obj EINA_UNUSED, Evas_VG_Node_Data *pd)
{
   return pd->mask;
}

void
_evas_vg_node_efl_gfx_base_size_get(Eo *obj,
                                    Evas_VG_Node_Data *pd EINA_UNUSED,
                                    int *w, int *h)
{
   Eina_Rectangle bound = { 0, 0, 0, 0 };

   eo_do(obj, evas_vg_node_bound_get(&bound));
   if (w) *w = bound.w;
   if (h) *h = bound.h;
}

// Parent should be a container otherwise dismissing the stacking operation
static Eina_Bool
_evas_vg_node_parent_checked_get(Eo *obj,
                                 Eo **parent,
                                 Evas_VG_Container_Data **cd)
{
   *cd = NULL;
   *parent = eo_do(obj, eo_parent_get());

   if (eo_isa(*parent, EVAS_VG_CONTAINER_CLASS))
     {
        *cd = eo_data_scope_get(*parent, EVAS_VG_CONTAINER_CLASS);
        if (!*cd)
          {
             ERR("Can't get EVAS_VG_CONTAINER_CLASS data.");
             goto on_error;
          }
     }
   return EINA_TRUE;

 on_error:
   *cd = NULL;
   return EINA_FALSE;
}

void
_evas_vg_node_eo_base_constructor(Eo *obj,
                                  Evas_VG_Node_Data *pd)
{
   Evas_VG_Container_Data *cd = NULL;
   Eo *parent;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   if (!_evas_vg_node_parent_checked_get(obj, &parent, &cd))
     eo_error_set(obj);

   eo_do(obj, eo_event_callback_add(EFL_GFX_CHANGED, _evas_vg_node_property_changed, pd));
   pd->changed = EINA_TRUE;
}

void
_evas_vg_node_eo_base_parent_set(Eo *obj,
                                 Evas_VG_Node_Data *pd EINA_UNUSED,
                                 Eo *parent)
{
   Evas_VG_Container_Data *cd = NULL;
   Evas_VG_Container_Data *old_cd = NULL;
   Eo *old_parent;

   if (eo_isa(parent, EVAS_VG_CONTAINER_CLASS))
     {
        cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
        if (!cd)
          {
             ERR("Can't get EVAS_VG_CONTAINER_CLASS data from %p.", parent);
             goto on_error;
          }
     }
   else if (parent != NULL)
     {
        ERR("%p not even an EVAS_VG_CLASS.", parent);
        goto on_error;
     }

   if (!_evas_vg_node_parent_checked_get(obj, &old_parent, &old_cd))
     goto on_error;

   // FIXME: this may become slow with to much object
   if (old_cd)
     old_cd->children = eina_list_remove(old_cd->children, obj);

   eo_do_super(obj, MY_CLASS, eo_parent_set(parent));
   if (cd)
     cd->children = eina_list_append(cd->children, obj);

   _evas_vg_node_changed(old_parent);
   _evas_vg_node_changed(obj);
   _evas_vg_node_changed(parent);

   return ;

 on_error:
   eo_error_set(obj);
   return ;
}

void
_evas_vg_node_efl_gfx_stack_raise(Eo *obj, Evas_VG_Node_Data *pd EINA_UNUSED)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *next;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   next = eina_list_next(lookup);
   if (!next) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, next);

   _evas_vg_node_changed(parent);
   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_efl_gfx_stack_stack_above(Eo *obj,
                                        Evas_VG_Node_Data *pd EINA_UNUSED,
                                        Efl_Gfx_Stack *above)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, above);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, ref);

   _evas_vg_node_changed(parent);
   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_efl_gfx_stack_stack_below(Eo *obj,
                                        Evas_VG_Node_Data *pd EINA_UNUSED,
                                        Efl_Gfx_Stack *below)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, below);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, ref);

   _evas_vg_node_changed(parent);
   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_efl_gfx_stack_lower(Eo *obj, Evas_VG_Node_Data *pd EINA_UNUSED)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *prev;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   prev = eina_list_prev(lookup);
   if (!prev) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, prev);

   _evas_vg_node_changed(parent);
   return ;

 on_error:
   eo_error_set(obj);
}

Efl_Gfx_Stack *
_evas_vg_node_efl_gfx_stack_below_get(Eo *obj, Evas_VG_Node_Data *pd)
{
   // FIXME: need to implement bound_get
   return NULL;
}

Efl_Gfx_Stack *
_evas_vg_node_efl_gfx_stack_above_get(Eo *obj, Evas_VG_Node_Data *pd)
{
   // FIXME: need to implement bound_get
   return NULL;
}

Eina_Bool
_evas_vg_node_original_bound_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Node_Data *pd EINA_UNUSED,
                                 Eina_Rectangle *r EINA_UNUSED)
{
   return EINA_FALSE;
}

#include "evas_vg_node.eo.c"