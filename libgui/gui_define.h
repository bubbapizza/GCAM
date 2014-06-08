/*
*  gui_define.h
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
#ifndef _GUI_DEFINE_H
#define _GUI_DEFINE_H

#define PROJECT_CLOSED			0x0
#define PROJECT_OPEN			0x1

#define WINDOW_W			720
#define WINDOW_H			540

#define PANEL_LEFT_W			280
#define PANEL_LEFT_H			(WINDOW_H - 80)		/* Subtract away the menu and status bar */
#define PANEL_LEFT_W_SW			(PANEL_LEFT_W - 20)	/* Left panel minus margins for scroll window */
#define PANEL_BOTTOM_H			260

#define TABLE_SPACING			3

#define	MAX_DIM_X			500.0	/* Inches */
#define	MAX_DIM_Y			500.0	/* Inches */
#define	MAX_DIM_Z			50.0	/* Inches */

#define MANTISSA			5	/* Length of Mantissa */

#define GUI_INSERT_AFTER		0
#define	GUI_INSERT_INTO			1
#define	GUI_INSERT_WITH_TANGENCY	2

#endif
