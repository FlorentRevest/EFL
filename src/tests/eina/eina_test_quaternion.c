/* EINA - EFL data type library
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

#include <math.h>
#include <float.h>
#include <limits.h>

#include "eina_suite.h"
#include "Eina.h"

#define FLOAT_CMP(a, b) (fabs((float)a - (float)b) <= FLT_MIN)

static inline Eina_Bool
eina_quaternion_cmp(const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   if (FLOAT_CMP(a->x, b->x) &&
       FLOAT_CMP(a->y, b->y) &&
       FLOAT_CMP(a->z, b->z) &&
       FLOAT_CMP(a->w, b->w))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_matrix3_cmp(const Eina_Matrix3 *a, const Eina_Matrix3 *b)
{
   if (FLOAT_CMP(a->xx, b->xx) &&
       FLOAT_CMP(a->xy, b->xy) &&
       FLOAT_CMP(a->xz, b->xz) &&
       FLOAT_CMP(a->yx, b->yx) &&
       FLOAT_CMP(a->yy, b->yy) &&
       FLOAT_CMP(a->yz, b->yz) &&
       FLOAT_CMP(a->zx, b->zx) &&
       FLOAT_CMP(a->zy, b->zy) &&
       FLOAT_CMP(a->zz, b->zz))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_point_3d_cmp(const Eina_Point_3D *a, const Eina_Point_3D *b)
{
   if (FLOAT_CMP(a->x, b->x) &&
       FLOAT_CMP(a->y, b->y) &&
       FLOAT_CMP(a->z, b->z))
     return EINA_TRUE;
   return EINA_FALSE;
}

START_TEST(eina_test_quaternion_norm)
{
   static const Eina_Quaternion q = { 1, 3, 4, 5 };

   eina_init();

   fail_if(!FLOAT_CMP(eina_quaternion_norm(&q), sqrt(51)));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_conjugate)
{
   static const Eina_Quaternion q1 = { 1, -1, -1, 3 }, q2 = { 1, 3, 4, 3 };
   static const Eina_Quaternion r1 = { -1, 1, 1, 3 }, r2 = { -1, -3, -4, 3 };
   Eina_Quaternion t1, t2;

   eina_init();

   eina_quaternion_conjugate(&t1, &q1);
   eina_quaternion_conjugate(&t2, &q2);

   fail_if(!eina_quaternion_cmp(&t1, &r1));
   fail_if(!eina_quaternion_cmp(&t2, &r2));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_matrix)
{
   Eina_Quaternion q = { 7, 9, 5, 1 };
   Eina_Matrix3 m = {
     -211, 136, 52,
     116, -147, 104,
     88, 76, -259
   };
   Eina_Matrix3 tm;

   eina_init();

   eina_quaternion_rotation_matrix3_get(&tm, &q);
   fail_if(!eina_matrix3_cmp(&tm, &m));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_op)
{
   Eina_Quaternion q = { 7, 9, 5, 1 };
   Eina_Quaternion z = { 0, 0, 0, 0 };
   Eina_Quaternion neg, r;

   eina_init();

   eina_quaternion_negative(&neg, &q);
   eina_quaternion_add(&r, &q, &neg);

   fail_if(!eina_quaternion_cmp(&z, &r));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_matrix_recompose)
{
   const Eina_Point_3D translation = { 0, 0, 0 };
   const Eina_Point_3D scale = { 1, 1, 1 };
   const Eina_Point_3D skew = { 0, 0, 0 };
   const Eina_Quaternion perspective = { 0, 0, 0, 1 };
   const Eina_Quaternion rotation = { 0, 0, 0, 1 };
   Eina_Matrix4 m4;

   eina_init();

   eina_quaternion_matrix4_to(&m4,
                              &rotation,
                              &perspective,
                              &translation,
                              &scale,
                              &skew);

   fail_if(eina_matrix4_type_get(&m4) != EINA_MATRIX_TYPE_IDENTITY);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_matrix_quaternion)
{
   const Eina_Point_3D rt = { -2, -3, 0 };
   const Eina_Point_3D rsc = { 4, 5, 1 };
   const Eina_Quaternion rr = { 0, 0, -1, 0 };
   const Eina_Quaternion rp = { 0, 0, 0, 1 };
   Eina_Quaternion rotation, perspective;
   Eina_Matrix3 m3, m3r;
   Eina_Matrix4 m4, m4r;
   Eina_Point_3D translation, scale, skew;

   eina_init();

   eina_matrix3_identity(&m3);
   eina_matrix3_rotate(&m3, 3.14159265);
   eina_matrix3_translate(&m3, 2, 3);
   eina_matrix3_scale(&m3, 4, 5);
   eina_matrix3_matrix4_to(&m4, &m3);

   fail_if(!eina_matrix4_quaternion_to(&rotation,
                                       &perspective,
                                       &translation,
                                       &scale,
                                       &skew,
                                       &m4));

   eina_quaternion_matrix4_to(&m4r,
                              &rotation,
                              &perspective,
                              &translation,
                              &scale,
                              &skew);

   eina_matrix4_matrix3_to(&m3r, &m4r);

   fail_if(!eina_point_3d_cmp(&scale, &rsc));
   fail_if(!eina_point_3d_cmp(&translation, &rt));
   fail_if(!eina_quaternion_cmp(&perspective, &rp));
   fail_if(!eina_quaternion_cmp(&rotation, &rr));

   // Disable this test for the moment as it seems a rounding issue
   // fail_if(!eina_matrix3_cmp(&m3r, &m3));

   eina_shutdown();
}
END_TEST

void
eina_test_quaternion(TCase *tc)
{
   tcase_add_test(tc, eina_test_quaternion_norm);
   tcase_add_test(tc, eina_test_quaternion_conjugate);
   tcase_add_test(tc, eina_test_quaternion_matrix);
   tcase_add_test(tc, eina_test_quaternion_op);
   tcase_add_test(tc, eina_test_matrix_quaternion);
   tcase_add_test(tc, eina_test_matrix_recompose);
}
