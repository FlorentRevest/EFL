#ifndef _EVAS_ENGINE_EGLFS_H
# define _EVAS_ENGINE_EGLFS_H

typedef enum _Evas_Engine_Info_Eglfs_Swap_Mode
{
   EVAS_ENGINE_EGLFS_SWAP_MODE_AUTO = 0,
   EVAS_ENGINE_EGLFS_SWAP_MODE_FULL = 1,
   EVAS_ENGINE_EGLFS_SWAP_MODE_COPY = 2,
   EVAS_ENGINE_EGLFS_SWAP_MODE_DOUBLE = 3,
   EVAS_ENGINE_EGLFS_SWAP_MODE_TRIPLE = 4,
   EVAS_ENGINE_EGLFS_SWAP_MODE_QUADRUPLE = 5
} Evas_Engine_Info_Eglfs_Swap_Mode;

typedef struct _Evas_Engine_Info_Eglfs Evas_Engine_Info_Eglfs;

struct _Evas_Engine_Info_Eglfs
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct 
     {
        unsigned int rotation, depth;
        unsigned int crtc_id, conn_id, buffer_id;
        unsigned int format, flags;

        Eina_Bool destination_alpha : 1;
        Eina_Bool vsync : 1;
        Eina_Bool indirect : 1;
        unsigned char swap_mode : 4;
     } info;

   struct
     {
        void (*pre_swap)(void *data, Evas *evas);
        void (*post_swap)(void *data, Evas *evas);
        void *data;
     } callback;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
