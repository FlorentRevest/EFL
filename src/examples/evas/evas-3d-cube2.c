/**
 * Simple Evas-3D example illustrating usage of normal mapping and animation created
 * by interpolation between frames.
 *
 * @verbatim
 * gcc -o evas-3d-cube2 evas-3d-cube2.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

#define  WIDTH          400
#define  HEIGHT         400

static const char *normal_map_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/normal_lego.png";

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node;

   Eo *camera;
   Eo *light;
   Eo *cube;
   Eo *mesh;
   Eo *material0;
   Eo *material1;

   Eo *texture0;
   Eo *texture1;
   Eo *texture_normal;
} Scene_Data;

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;

static const unsigned int pixels0[] =
{
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
};

static const unsigned int pixels1[] =
{
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
};

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(background, efl_gfx_size_set(w, h));
   eo_do(image, efl_gfx_size_set(w, h));
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   static int   frame = 0;
   static int   inc   = 1;
   Scene_Data  *scene = (Scene_Data *)data;

   angle += 0.2;

   eo_do(scene->mesh_node,
         evas_canvas3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   frame += inc;

   if (frame >= 20) inc = -1;
   else if (frame <= 0) inc = 1;

   eo_do(scene->mesh_node, evas_canvas3d_node_mesh_frame_set(scene->mesh, frame));

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   eo_do(data->camera,
         evas_canvas3d_camera_projection_perspective_set(60.0, 1.0, 2.0, 50.0));

   data->camera_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_canvas3d_node_camera_set(data->camera),
         evas_canvas3d_node_position_set(0.0, 0.0, 3.0),
         evas_canvas3d_node_look_at_set(EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node,
         evas_canvas3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_canvas3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_canvas3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_canvas3d_node_light_set(data->light),
         evas_canvas3d_node_position_set(0.0, 0.0, 10.0),
         evas_canvas3d_node_look_at_set(EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_canvas3d_node_member_add(data->light_node));
}

static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material. */
   data->material0 = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   data->material1 = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   eo_do(data->material0,
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_NORMAL, EINA_TRUE),

         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_material_shininess_set(100.0));

   eo_do(data->material1,
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_NORMAL, EINA_TRUE),

         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_material_shininess_set(100.0));

   data->texture0 = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   data->texture1 = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   data->texture_normal = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);

   eo_do(data->texture0,
         evas_canvas3d_texture_data_set(EVAS_COLORSPACE_ARGB8888, 4, 4, &pixels0[0]));
   eo_do(data->texture1,
         evas_canvas3d_texture_data_set(EVAS_COLORSPACE_ARGB8888, 4, 4, &pixels1[0]));
   eo_do(data->texture_normal,
         evas_canvas3d_texture_file_set(normal_map_path, NULL));

   eo_do(data->material0,
         evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, data->texture0));
   eo_do(data->material1,
         evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, data->texture1));
   eo_do(data->material1,
         evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_NORMAL, data->texture_normal));

   /* Set data of primitive */
   data->cube = eo_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   eo_do(data->cube,
         evas_canvas3d_primitive_form_set(EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE));

   /* Setup mesh. */
   data->mesh = eo_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   eo_do(data->mesh,
         evas_canvas3d_mesh_from_primitive_set(0, data->cube),
         evas_canvas3d_mesh_frame_material_set(0, data->material0),
         evas_canvas3d_mesh_frame_add(20),
         evas_canvas3d_mesh_frame_material_set(20, data->material1),
         evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_NORMAL_MAP));

   data->mesh_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_MESH));
   eo_do(data->root_node, evas_canvas3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_canvas3d_node_mesh_add(data->mesh));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   eo_do(data->scene,
         evas_canvas3d_scene_size_set(WIDTH, HEIGHT),
         evas_canvas3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0));

   data->root_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   eo_do(data->scene,
         evas_canvas3d_scene_root_node_set(data->root_node),
         evas_canvas3d_scene_camera_node_set(data->camera_node));
}

int
main(void)
{
   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   /* Add a background rectangle objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_do(background,
         efl_gfx_color_set(0, 0, 0, 255),
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   eo_do(image,
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   /* Add animation timer callback. */
   ecore_timer_add(0.01, _animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
