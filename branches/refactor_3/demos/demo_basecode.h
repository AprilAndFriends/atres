/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
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
#include "atres/RenderingInterface.h"
#include <map>

class AtresOpenGLInterface : public Atres::RenderInterface
{
public:
	AtresOpenGLInterface();
	~AtresOpenGLInterface();
	
	unsigned int loadResource(const char* filename);
	void render(Atres::CharacterRenderOp* rops,int n);
};



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

extern AtresOpenGLInterface ogl_iface;