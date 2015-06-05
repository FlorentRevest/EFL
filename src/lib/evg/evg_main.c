/* EVG - EFL retained mode drawing library
 * Copyright (C) 2015 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Evg.h>
#include <Evas.h>

#include "evg_private.h"

typedef struct _Evg Evg;
struct _Evg
{
   Eina_File *f;

   Efl_VG_Container *start, *end;

   Efl_VG_Container *self;

   double pos;
};

int _evg_log_dom_global = 0;

static int _evg_main_count = 0;

EAPI int
evg_init(void)
{
   if (EINA_LIKELY(_evg_main_count > 0))
     return ++_evg_main_count;

   eina_init();
   eo_init();

   _evg_log_dom_global = eina_log_domain_register("evg", EVG_DEFAULT_LOG_COLOR);
   if (_evg_log_dom_global < 0)
     {
        EINA_LOG_ERR("Could not register log domain: evg");
        goto on_error;
     }

   _evg_main_count = 1;

   eina_log_timing(_evg_log_dom_global, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   return _evg_main_count;

 on_error:
   eo_shutdown();
   eina_shutdown();

   return 0;
}

EAPI int
evg_shutdown(void)
{
   if (_evg_main_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown of evg.");
        return 0;
     }

   _evg_main_count--;
   if (EINA_LIKELY(_evg_main_count > 0))
     return _evg_main_count;

   eina_log_timing(_evg_log_dom_global,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   eo_shutdown();

   eina_log_domain_unregister(_evg_log_dom_global);

   eina_shutdown();
   return _evg_main_count;
}

static Efl_VG_Container *
_efl_vg_loader_self_build(Efl_VG_Container *parent,
                          const Efl_VG_Container *start,
                          const Efl_VG_Container *end)
{
   Eina_Iterator *it;
   Eo *start_child, *r;

   if (!start || !end) return NULL;

   r = eo_add(EFL_VG_CONTAINER_CLASS, parent);

   it = eo_do_ret(start, it, efl_vg_container_children_get());

   EINA_ITERATOR_FOREACH(it, start_child)
     {
        const char *name;
        Eo *end_child;

        name = eo_do_ret(start_child, name, efl_vg_name_get());

        end_child = eo_do_ret(end, end_child, efl_vg_container_child_get(name));

        if (!end_child) continue ;

        if (eo_class_get(start_child) != eo_class_get(end_child)) continue;

        if (eo_isa(start_child, EFL_VG_CONTAINER_CLASS))
          _efl_vg_loader_self_build(r, start_child, end_child);
        else
          eo_add(eo_class_get(start_child), r);
     }

   return r;
}

static void
_efl_vg_loader_start_set(Eo *obj EINA_UNUSED, Evg *pd, const Efl_VG_Container *tree)
{
   _eo_ref_replace(&pd->start, tree);
}

static const Efl_VG_Container *
_efl_vg_loader_start_get(Eo *obj EINA_UNUSED, Evg *pd)
{
   return pd->start;
}

static void
_efl_vg_loader_end_set(Eo *obj EINA_UNUSED, Evg *pd, const Efl_VG_Container *tree)
{
   _eo_ref_replace(&pd->end, tree);
}

static const Efl_VG_Container *
_efl_vg_loader_end_get(Eo *obj EINA_UNUSED, Evg *pd)
{
   return pd->end;
}

static void
_efl_vg_loader_interpolation_set(Eo *obj EINA_UNUSED, Evg *pd, double pos)
{
   pd->pos = pos;

   if (!pd->self) pd->self = _efl_vg_loader_self_build(NULL, pd->start, pd->end);
   // Walk self and access start and end to do the interpolation
}

static double
_efl_vg_loader_interpolation_get(Eo *obj EINA_UNUSED, Evg *pd)
{
   return pd->pos;
}

static Eo_Base *
_efl_vg_loader_eo_base_constructor(Eo *obj, Evg *pd)
{
   return obj;
}

static void
_efl_vg_loader_eo_base_destructor(Eo *obj, Evg *pd)
{
   eo_unref(pd->start);
   eo_unref(pd->end);
   eo_unref(pd->self);

   eina_file_close(pd->f);
}

static Eina_Bool
_efl_vg_loader_efl_file_file_set(Eo *obj, Evg *pd, const char *file, const char *key)
{
   Eina_File *tmp;
   Eina_Bool r;

   tmp = eina_file_open(file, EINA_FALSE);
   if (!tmp) return EINA_FALSE;

   r = eo_do_ret(obj, r, efl_file_mmap_set(tmp, key));

   eina_file_close(tmp);
   return r;
}

static void
_efl_vg_loader_efl_file_file_get(Eo *obj, Evg *pd, const char **file, const char **key)
{
   if (!pd->f) return ;

   if (file) *file = eina_file_filename_get(pd->f);
   if (key) *key = NULL;
}

static Eina_Bool
_efl_vg_loader_efl_file_mmap_set(Eo *obj, Evg *pd, const Eina_File *f, const char *key)
{
   // Only handling SVG for now
}

static void
_efl_vg_loader_efl_file_mmap_get(Eo *obj, Evg *pd, const Eina_File **f, const char **key)
{
   if (f) *f = pd->f;
   if (key) *key = NULL;
}

#include "efl_vg_loader.eo.c"
