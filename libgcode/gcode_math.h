/*
*  gcode_math.h
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
#ifndef _GCODE_MATH_H
#define _GCODE_MATH_H

#include <math.h>

#define GCODE_PI		3.141592653589793
#define GCODE_HPI		1.570796326794896
#define GCODE_2PI		6.283185307179586
#define GCODE_MM2INCH		0.039370078740158
#define GCODE_INCH2MM		25.4
#define GCODE_PRECISION		0.00001		/* Cartesian */
#define	GCODE_ANGULAR_PRECISION	0.001		/* Angular Degrees */
#define GCODE_RAD2DEG		57.29577951308232
#define GCODE_DEG2RAD		0.017453292519943295

#define gfloat_t double

typedef gfloat_t gcode_vec2d_t[2];
typedef gfloat_t gcode_vec3d_t[3];

int gcode_math_angle_within_arc (gfloat_t start_angle, gfloat_t sweep_angle, gfloat_t test_angle);
void gcode_math_xy_to_angle (gcode_vec2d_t center, gfloat_t radius, gfloat_t x, gfloat_t y, gfloat_t *angle);


#define GCODE_MATH_AREA(_d) (GCODE_PI*0.25*_d*_d)

#define GCODE_MATH_CHORD_HALF_LENGTH(_radius, _angle) (_radius * sin (0.5 * _angle)) /* Angles in radians */

#define GCODE_MATH_VEC2D_MAG(_m, _v) { \
	_m = sqrt (_v[0]*_v[0] + _v[1]*_v[1]); }

#define GCODE_MATH_VEC2D_UNITIZE(_v) { \
	gfloat_t m; \
	GCODE_MATH_VEC2D_MAG (m, _v); \
	m = 1.0 / m; \
	_v[0] *= m; \
	_v[1] *= m; }

#define GCODE_MATH_VEC2D_ADD(_v, _va, _vb) { \
	_v[0] = _va[0] + _vb[0]; \
	_v[1] = _va[1] + _vb[1]; }

#define GCODE_MATH_VEC2D_SUB(_v, _va, _vb) { \
	_v[0] = _va[0] - _vb[0]; \
	_v[1] = _va[1] - _vb[1]; }

#define GCODE_MATH_VEC2D_MUL_SCALAR(_v, _va, _s) { \
	_v[0] = _va[0] * _s; \
	_v[1] = _va[1] * _s; }

#define GCODE_MATH_VEC2D_DOT(_d, _va, _vb) { \
	_d = _va[0]*_vb[0] + _va[1]*_vb[1]; }

#define GCODE_MATH_VEC2D_SCALE(_v, _s) { \
	_v[0] *= _s; \
	_v[1] *= _s; }

#define GCODE_MATH_ROTATE(_xform, _pt, _rotate) { \
	gfloat_t _angle, _dist = sqrt (_pt[0]*_pt[0] + _pt[1]*_pt[1]); \
	if (_dist < GCODE_PRECISION) \
	{ \
	  _xform[0] = _pt[0]; \
	  _xform[1] = _pt[1]; \
	} \
	else \
	{ \
	  _angle = asin (_pt[1] / _dist); \
	  if (_pt[0] < 0.0) \
	    _angle += 2.0 * (GCODE_HPI - _angle); \
	  if (_angle < 0.0) \
	    _angle += GCODE_2PI; \
	  _angle += _rotate * GCODE_DEG2RAD; \
	  _xform[0] = _dist * cos (_angle); \
	  _xform[1] = _dist * sin (_angle); \
	} }

#define GCODE_MATH_VEC3D_SET(_v, _x, _y, _z) { \
	_v[0] = _x; \
	_v[1] = _y; \
	_v[2] = _z; }

#define GCODE_MATH_VEC3D_COPY(_v, _vc) { \
	_v[0] = _vc[0]; \
	_v[1] = _vc[1]; \
	_v[2] = _vc[2]; }

#define GCODE_MATH_VEC3D_ADD(_v, _va, _vb) { \
	_v[0] = _va[0] + _vb[0]; \
	_v[1] = _va[1] + _vb[1]; \
	_v[2] = _va[2] + _vb[2]; }

#define GCODE_MATH_VEC3D_SUB(_v, _va, _vb) { \
	_v[0] = _va[0] - _vb[0]; \
	_v[1] = _va[1] - _vb[1]; \
	_v[2] = _va[2] - _vb[2]; }

#define GCODE_MATH_VEC3D_MUL_SCALAR(_v, _va, _s) { \
	_v[0] = _va[0] * _s; \
	_v[1] = _va[1] * _s; \
	_v[2] = _va[2] * _s; }

#define GCODE_MATH_VEC3D_DIST(_d, _va, _vb) { \
	_d = sqrt ((_va[0]-_vb[0])*(_va[0]-_vb[0])+ \
	           (_va[1]-_vb[1])*(_va[1]-_vb[1])+ \
	           (_va[2]-_vb[2])*(_va[2]-_vb[2])); }

#define GCODE_MATH_VEC3D_MAG(_m, _v) { \
	_m = sqrt (_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]); }

#define GCODE_MATH_VEC3D_UNITIZE(_v) { \
	gfloat_t m; \
	GCODE_MATH_VEC3D_MAG (m, _v); \
	m = 1.0 / m; \
	_v[0] *= m; \
	_v[1] *= m; \
	_v[2] *= m; }

#define GCODE_MATH_VEC3D_CROSS(_v, _v0, _v1) {\
        _v[0] = _v0[1]*_v1[2] - _v0[2]*_v1[1]; \
        _v[1] = _v0[2]*_v1[0] - _v0[0]*_v1[2]; \
        _v[2] = _v0[0]*_v1[1] - _v0[1]*_v1[0]; }

#define GCODE_MATH_VEC3D_ANGLE(_a, _x, _y) { \
	_a = _y < 0.0 ? 2.0*GCODE_PI - acos (_x) : acos (_x); }


#endif
