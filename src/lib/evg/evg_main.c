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
#include "evg_private.h"

typedef struct _Evg Evg;
struct _Evg
{
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

static void
_efl_vg_loader_start_set(Eo *obj, Evg *pd, const Efl_VG_Container *tree)
{
}

static const Efl_VG_Container *
_efl_vg_loader_start_get(Eo *obj, Evg *pd)
{
}

static void
_efl_vg_loader_end_set(Eo *obj, Evg *pd, const Efl_VG_Container *tree)
{
}

static const Efl_VG_Container *
_efl_vg_loader_end_get(Eo *obj, Evg *pd)
{
}

static void
_efl_vg_loader_interpolation_set(Eo *obj, Evg *pd, double pos)
{
}

static double
_efl_vg_loader_interpolation_get(Eo *obj, Evg *pd)
{
}

static Eo_Base *
_efl_vg_loader_eo_base_constructor(Eo *obj, Evg *pd)
{
}

static void
_efl_vg_loader_eo_base_destructor(Eo *obj, Evg *pd)
{
}

static Eina_Bool
_efl_vg_loader_efl_file_file_set(Eo *obj, Evg *pd, const char *file, const char *key)
{
}

static void
_efl_vg_loader_efl_file_file_get(Eo *obj, Evg *pd, const char **file, const char **key)
{
}

static Eina_Bool
_efl_vg_loader_efl_file_mmap_set(Eo *obj, Evg *pd, const Eina_File *f, const char *key)
{
}

static void
_efl_vg_loader_efl_file_mmap_get(Eo *obj, Evg *pd, const Eina_File **f, const char **key)
{
}

#include "efl_vg_loader.eo.c"
