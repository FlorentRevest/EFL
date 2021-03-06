
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

#include "callback.eo.h"

struct _Callback_Data
{
  int callbacks;
};
typedef struct _Callback_Data Callback_Data;

#define MY_CLASS CALLBACK_CLASS

static Eina_Bool _callback_callback_added(void* data EINA_UNUSED, Eo* obj EINA_UNUSED
                                          , Eo_Event_Description const* e EINA_UNUSED
                                          , void* event_info EINA_UNUSED)
{
  Callback_Data* pd = event_info;
  ++pd->callbacks;
  eo_event_callback_call(CALLBACK_EVENT_CALL_ON_ADD, &pd->callbacks);
  return EINA_TRUE;
}

static Eo *_callback_eo_base_constructor(Eo *obj, Callback_Data *pd EINA_UNUSED)
{
  pd->callbacks = 0;
  obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

  eo_event_callback_priority_add(EO_BASE_EVENT_CALLBACK_ADD, EO_CALLBACK_PRIORITY_DEFAULT
                                 , &_callback_callback_added, pd);

  return obj;
}

static void _callback_onecallback(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data)
{
  cb(data);
}

static void _callback_twocallback(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data
                                  , Ecore_Cb  cb2 EINA_UNUSED)
{
  cb(data);
}

static void _callback_test_global_callbacks(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED
                                            , Ecore_Cb cb, void *data)
{
  cb(data);
}

#include "callback.eo.c"

