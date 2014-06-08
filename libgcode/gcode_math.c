/*
*  gcode_math.c
*  Source code file for G-Code generation, simulation, and visualization
*  library. This software is Copyright (C) 2006 by Justin Shumaker
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "gcode_math.h"


int
gcode_math_angle_within_arc (gfloat_t start_angle, gfloat_t sweep_angle, gfloat_t test_angle)
{
  gfloat_t begin, end;

  if (sweep_angle < 0.0)
  {
    begin = start_angle + sweep_angle;
    end = start_angle;
  }
  else
  {
    begin = start_angle;
    end = start_angle + sweep_angle;
  }

  if (begin < 0.0)
  {
    begin += 360.0;
    end += 360.0;
  }

  if (test_angle >= begin-GCODE_ANGULAR_PRECISION && test_angle <= end+GCODE_ANGULAR_PRECISION)
  {
/*    printf ("begin: %f, test_angle: %f, end: %f\n", begin, test_angle, end); */
    return (0);
  }

  test_angle += 360.0;
  if (test_angle >= begin-GCODE_ANGULAR_PRECISION && test_angle <= end+GCODE_ANGULAR_PRECISION)
  {
/*    printf ("begin: %f, test_angle: %f, end: %f\n", begin, test_angle, end); */
    return (0);
  }

/*  printf ("begin: %f, test_angle: %f, end: %f\n", begin, test_angle, end); */
  return (1);
}


void
gcode_math_xy_to_angle (gcode_vec2d_t center, gfloat_t radius, gfloat_t x, gfloat_t y, gfloat_t *angle)
{
  gfloat_t tx, ty, radius_inv;

  if (radius < GCODE_PRECISION)
  {
    (*angle) = 0.0;
    return;
  }

  radius_inv = 1.0 / radius;
  tx = (x - center[0]) * radius_inv;
  ty = (y - center[1]) * radius_inv;

  /* On occaison tx and ty can be slightly larger than 1 and smaller than -1 due to floating precision */
  if (tx > 1.0)
    tx = 1.0;
  if (tx < -1.0)
    tx = -1.0;

  *angle = GCODE_RAD2DEG * (ty < 0.0 ? (GCODE_2PI) - acos (tx) : acos (tx));

  if (*angle+GCODE_PRECISION >= 360.0)
    *angle -= 360.0;

//printf ("  tx: %f, ty: %f, angle: %f\n", tx, ty, *angle);
}
