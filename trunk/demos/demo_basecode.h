/************************************************************************************
This source file is part of the Advanced Text Rendering System
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#ifdef _WIN32
#include <windows.h>
#endif
#include <string.h>
#include <stdio.h>

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#ifndef _WIN32
unsigned long GetTickCount();
#endif

void psleep(int milliseconds);
int nextPow2(int x);
void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a);
void drawWiredQuad(float x,float y,float w,float h,float r,float g,float b,float a);
void drawTexturedQuad(float x,float y,float w,float h,float sw,float sh);
void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh);
unsigned int loadTexture(const char* name);

unsigned int createTexture(int w,int h,unsigned int format=GL_RGB);
