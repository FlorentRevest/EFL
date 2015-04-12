#include "eina_private.h"
#include "eina_bezier.h"
#include <math.h>


static void
_eina_bezier_1st_derivative(Eina_Bezier *bz, double t, double *px, double *py)
{
   // p'(t) = 3 * (-(1-2t+t^2) * p0 + (1 - 4 * t + 3 * t^2) * p1 + (2 * t - 3 * t^2) * p2 + t^2 * p3)

   double m_t = 1. - t;

   double d = t * t;
   double a = -m_t * m_t;
   double b = 1 - 4 * t + 3 * d;
   double c = 2 * t - 3 * d;

   *px = 3 * ( a * bz->x1 + b * bz->x2 + c * bz->x3 + d * bz->x4);
   *py = 3 * ( a * bz->y1 + b * bz->y2 + c * bz->y3 + d * bz->y4);
}

// approximate sqrt(x*x + y*y) using alpha max plus beta min algorithm.
// With alpha = 1, beta = 3/8, giving results with a largest error less 
// than 7% compared to the exact value.
static
double _line_length(double x1, double y1, double x2, double y2)
{
   double x = x2 - x1;
   double y = y2 - y1;
   x = x < 0 ? -x : x;
   y = y < 0 ? -y : y;
   return (x > y ? x + 0.375 * y : y + 0.375 * x);
}

static void
_eina_bezier_split(Eina_Bezier *b, Eina_Bezier *firstHalf, Eina_Bezier *secondHalf)
{
   double c = (b->x2 + b->x3)*.5;
   firstHalf->x2 = (b->x1 + b->x2)*.5;
   secondHalf->x3 = (b->x3 + b->x4)*.5;
   firstHalf->x1 = b->x1;
   secondHalf->x4 = b->x4;
   firstHalf->x3 = (firstHalf->x2 + c)*.5;
   secondHalf->x2 = (secondHalf->x3 + c)*.5;
   firstHalf->x4 = secondHalf->x1 = (firstHalf->x3 + secondHalf->x2)*.5;

   c = (b->y2 + b->y3)/2;
   firstHalf->y2 = (b->y1 + b->y2)*.5;
   secondHalf->y3 = (b->y3 + b->y4)*.5;
   firstHalf->y1 = b->y1;
   secondHalf->y4 = b->y4;
   firstHalf->y3 = (firstHalf->y2 + c)*.5;
   secondHalf->y2 = (secondHalf->y3 + c)*.5;
   firstHalf->y4 = secondHalf->y1 = (firstHalf->y3 + secondHalf->y2)*.5;
}

static void
_eina_bezier_length_helper(Eina_Bezier *b,double *length, double error)
{
   Eina_Bezier left, right;     /* bez poly splits */

   double len = 0.0;  /* arc length */
   double chord;             /* chord length */

   len = len + _line_length(b->x1, b->y1, b->x2, b->y2);
   len = len + _line_length(b->x2, b->y2, b->x3, b->y3);
   len = len + _line_length(b->x3, b->y3, b->x4, b->y4);

   chord = _line_length(b->x1, b->y1, b->x4, b->y4);

   if((len-chord) > error) {
     _eina_bezier_split(b, &left, &right);                 /* split in two */
     _eina_bezier_length_helper(&left, length, error);       /* try left side */
     _eina_bezier_length_helper(&right, length, error);      /* try right side */
     return;
   }

   *length = *length + len;

   return;
}

EAPI void
eina_bezier_values_set(Eina_Bezier *b,
                       double start_x, double start_y,
                       double ctrl_x1, double ctrl_y1,
                       double ctrl_x2, double ctrl_y2,
                       double end_x, double end_y)
{
   b->x1 = start_x;
   b->y1 = start_y;
   b->x2 = ctrl_x1;
   b->y2 = ctrl_y1;
   b->x3 = ctrl_x2;
   b->y3 = ctrl_y2;
   b->x4 = end_x;
   b->y4 = end_y;
}


EAPI void
eina_bezier_point_at(Eina_Bezier *bz, double t, double *px, double *py)
{
   double m_t = 1. - t;
   {
      double a = bz->x1*m_t + bz->x2*t;
      double b = bz->x2*m_t + bz->x3*t;
      double c = bz->x3*m_t + bz->x4*t;
      a = a*m_t + b*t;
      b = b*m_t + c*t;
      *px = a*m_t + b*t;
   }
   {
      double a = bz->y1*m_t + bz->y2*t;
      double b = bz->y2*m_t + bz->y3*t;
      double c = bz->y3*m_t + bz->y4*t;
      a = a*m_t + b*t;
      b = b*m_t + c*t;
      *py = a*m_t + b*t;
   }
}

#ifndef M_2PI
#define M_2PI 6.283185
#endif

EAPI double eina_bezier_angle_at(Eina_Bezier *b, double t)
{
   double x, y;
   _eina_bezier_1st_derivative(b, t, &x, &y);

   double theta = atan2(y, x) * 360.0 / M_2PI;

   double theta_normalized = theta < 0 ? theta + 360 : theta;

   return theta_normalized;
}

EAPI double
eina_bezier_length_get(Eina_Bezier *b)
{
   double length = 0.0;

   _eina_bezier_length_helper(b, &length, 0.01);

   return length;
}

static void
_eina_bezier_split_left(Eina_Bezier *b, double t, Eina_Bezier *left)
{
   left->x1 = b->x1;
   left->y1 = b->y1;

   left->x2 = b->x1 + t * ( b->x2 - b->x1 );
   left->y2 = b->y1 + t * ( b->y2 - b->y1 );

   left->x3 = b->x2 + t * ( b->x3 - b->x2 ); // temporary holding spot
   left->y3 = b->y2 + t * ( b->y3 - b->y2 ); // temporary holding spot

   b->x3 = b->x3 + t * ( b->x4 - b->x3 );
   b->y3 = b->y3 + t * ( b->y4 - b->y3 );

   b->x2 = left->x3 + t * ( b->x3 - left->x3);
   b->y2 = left->y3 + t * ( b->y3 - left->y3);

   left->x3 = left->x2 + t * ( left->x3 - left->x2 );
   left->y3 = left->y2 + t * ( left->y3 - left->y2 );

   left->x4 = b->x1 = left->x3 + t * (b->x2 - left->x3);
   left->y4 = b->y1 = left->y3 + t * (b->y2 - left->y3);
}

EAPI double
eina_bezier_t_at(Eina_Bezier *b, double l)
{
   double len = eina_bezier_length_get(b);
   double t   = 1.0;
   const double error = 0.01;
   if (l > len || ((len - l) < 0.0001))
     return t;

   t *= 0.5;

   double lastBigger = 1.0;
   while (1)
     {
        Eina_Bezier right = *b;
        Eina_Bezier left;
        _eina_bezier_split_left(&right, t, &left);
        double lLen = eina_bezier_length_get(&left);
        if (fabs(lLen - l) < error)
          break;

        if (lLen < l)
          {
             t += (lastBigger - t) * 0.5;
          }
        else
          {
             lastBigger = t;
             t -= t * 0.5;
          }
     }
   return t;
}

EAPI void
eina_bezier_split_at_length(Eina_Bezier *b, double len, Eina_Bezier *left, Eina_Bezier *right)
{
   double t =  eina_bezier_t_at(b, len);
   *right = *b;
   _eina_bezier_split_left(right, t, left);
}
