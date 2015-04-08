#ifndef EINA_BEZIER_H
#define EINA_BEZIER_H

/**
 * Floating point cubic bezier curve
 */
typedef struct _Eina_Bezier Eina_Bezier;

struct _Eina_Bezier
{
   double x1; // x coordinate of start point
   double y1; // y coordinate of start point

   double x2; // x coordinate of 1st control point
   double y2; // y coordinate of 1st control point

   double x3; // x coordinate of 2nd control point
   double y3; // y coordinate of 2nd control point

   double x4; // x coordinate of end point
   double y4; // y coordinate of end point
};

/**
 * @brief Set the values of the points of the given floating
 * point cubic bezier curve.
 *
 * @param b The floating point bezier.
 * @param start_x x coordinate of start point.
 * @param start_y y coordinate of start point.
 * @param ctrl_x1 x coordinate of 1st control point.
 * @param ctrl_y1 y coordinate of 1st control point.
 * @param ctrl_x2 x coordinate of 2nd control point.
 * @param ctrl_y2 y coordinate of 2nd control point.
 * @param end_x   x coordinate of end point.
 * @param end_y   y coordinate of end point.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI void eina_bezier_values_set(Eina_Bezier *b,
                            double start_x, double start_y,
                            double ctrl_x1, double ctrl_y1,
                            double ctrl_x2, double ctrl_y2,
                            double end_x, double end_y);

/**
 * @brief Returns the length of the given floating
 * point cubic bezier curve.
 *
 * @param b The floating point bezier.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI double eina_bezier_length_get(Eina_Bezier *b);

/**
 * @brief Returns the position of the given bezier
 * at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI double eina_bezier_t_at(Eina_Bezier *b, double len);

/**
 * @brief Gets the point on the bezier curve at 
 * position t.
 *
 * @param b The floating point bezier.
 * @param t The floating point position.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI void eina_bezier_point_at(Eina_Bezier *b, double t, double *px, double *py);

/**
 * @brief Returns the slope  of the  bezier
 * at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI double eina_bezier_angle_at(Eina_Bezier *b, double t);

/**
 * @brief split the bezier at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 *
 */
EAPI void eina_bezier_split_at_length(Eina_Bezier *b, double len, Eina_Bezier *left, Eina_Bezier *right);

#endif // EINA_BEZIER_H
