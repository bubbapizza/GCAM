########################################################################
#  Makefile.am
#  Source code file for G-Code generation, simulation, and visualization
#  library. This software is Copyright (C) 2006 by Justin Shumaker
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
########################################################################
bin_PROGRAMS = gcam
gcam_SOURCES = gcam.c

AM_CFLAGS = \
	@GTKGLEXT_CFLAGS@ \
	-I${top_srcdir}/libgui \
	-I${top_srcdir}/libgcode

AM_LDFLAGS = \
	${top_builddir}/libgui/libgui.la \
	${top_builddir}/libgcode/libgcode.la \
	@GTK_LIBS@ @GTKGLEXT_LIBS@ @PNG_LIBS@ -lexpat

SUBDIRS = \
	libgui \
	libgcode \
	samples \
	share
