//Compile with:
// gcc -o emotion_signals_example emotion_signals_example.c `pkg-config --libs --cflags emotion evas ecore ecore-evas eo`

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Emotion.h>
#include <stdio.h>

#define WIDTH  (320)
#define HEIGHT (240)

static void
_display_info(Evas_Object *o)
{
   int w, h;
   printf("playing: %d\n", emotion_object_play_get(o));
   printf("meta title: %s\n",
	  emotion_object_meta_info_get(o, EMOTION_META_INFO_TRACK_TITLE));
   printf("seek position: %0.3f\n",
	  emotion_object_position_get(o));
   printf("play length: %0.3f\n",
	  emotion_object_play_length_get(o));
   printf("is seekable: %d\n",
	  emotion_object_seekable_get(o));
   emotion_object_size_get(o, &w, &h);
   printf("video geometry: %dx%d\n", w, h);
   printf("video width / height ratio: %0.3f\n",
	  emotion_object_ratio_get(o));
   printf("\n");
}

static Eina_Bool
_playback_started_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object started playback.\n");
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_playback_finished_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object finished playback.\n");
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_open_done_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object open done.\n");
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_position_update_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object first position update.\n");
   eo_do(o, eo_event_callback_del(EMOTION_OBJECT_EVENT_POSITION_UPDATE,
             _position_update_cb, NULL));
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_frame_decode_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object first frame decode.\n");
   eo_do(o, eo_event_callback_del(EMOTION_OBJECT_EVENT_FRAME_DECODE,
            _frame_decode_cb, NULL));
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_decode_stop_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object decode stop.\n");
   _display_info(o);

   return EINA_TRUE;
}

static Eina_Bool
_frame_resize_cb(void *data EINA_UNUSED,
            Eo *o, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf(">>> Emotion object frame resize.\n");
   _display_info(o);

   return EINA_TRUE;
}

static const Eo_Callback_Array_Item emotion_object_example_callbacks[] = {
       { EMOTION_OBJECT_EVENT_PLAYBACK_STARTED, _playback_started_cb },
       { EMOTION_OBJECT_EVENT_PLAYBACK_FINISHED, _playback_finished_cb },
       { EMOTION_OBJECT_EVENT_OPEN_DONE, _open_done_cb },
       { EMOTION_OBJECT_EVENT_POSITION_UPDATE, _position_update_cb },
       { EMOTION_OBJECT_EVENT_FRAME_DECODE, _frame_decode_cb },
       { EMOTION_OBJECT_EVENT_DECODE_STOP, _decode_stop_cb },
       { EMOTION_OBJECT_EVENT_FRAME_RESIZE, _frame_resize_cb },
       { NULL, NULL }
};

static void
_setup_emotion_callbacks(Evas_Object *o)
{
   eo_do(o, eo_event_callback_add
     (EMOTION_OBJECT_EVENT_PLAYBACK_STARTED, _playback_started_cb, NULL));
}

int
main(int argc, const char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *bg, *em;
   const char *filename = NULL;
   const char *module = NULL;

   if (argc < 2)
     {
	printf("At least one argument is necessary. Usage:\n");
	printf("\t%s <filename> [module_name]\n", argv[0]);
	goto error;
     }

   filename = argv[1];

   if (argc >= 3)
     module = argv[2];

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto error;

   ecore_evas_show(ee);

   /* the canvas pointer, de facto */
   e = ecore_evas_get(ee);

   /* adding a background to this example */
   bg = evas_object_rectangle_add(e);
   evas_object_name_set(bg, "our dear rectangle");
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   /* Creating the emotion object */
   em = emotion_object_add(e);

   /* Try to load the specified module - NULL for auto-discover */
   if (!emotion_object_init(em, module))
     fprintf(stderr, "Emotion: \"%s\" module could not be initialized.\n", module);

   _display_info(em);
   _setup_emotion_callbacks(em);

   if (!emotion_object_file_set(em, filename))
     fprintf(stderr, "Emotion: Could not load the file \"%s\"\n", filename);

   evas_object_move(em, 0, 0);
   evas_object_resize(em, WIDTH, HEIGHT);
   evas_object_show(em);

   emotion_object_play_set(em, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return 0;

   ecore_evas_free(ee);

error:
   ecore_evas_shutdown();
   return -1;
}
