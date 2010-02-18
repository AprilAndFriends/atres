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
#include <fstream>
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
#ifndef __APPLE__
#include <GL/glx.h>
#endif

#include <sys/time.h>
unsigned long GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) return 0;
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

#include <string>
#ifdef _WIN32
#pragma warning( disable: 4996 ) // MSVC++
#endif
extern std::string window_name;
extern int window_w,window_h;

float mx=0,my=0;



void init();
void destroy();
void update(float);
void draw();
void setDebugTitle(char* out);
void OnKeyPress(int key);
void OnClick(float x,float y);

void psleep(int milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
    usleep(milliseconds*1000);
#endif
}

int nextPow2(int x)
{
	int y;
	for (y=1;y<x;y*=2);
	return y;
}

void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a)
{
	glColor4f(r,g,b,a);
	glBegin (GL_QUADS);
	glVertex3f(x,  y,  0.0f);
	glVertex3f(x+w,y,  0.0f);
	glVertex3f(x+w,y+h,0.0f);
	glVertex3f(x,  y+h,0.0f);
	glEnd();
	glColor4f(1,1,1,1);
}

void drawWiredQuad(float x,float y,float w,float h,float r,float g,float b,float a)
{
	glColor4f(r,g,b,a);
	glBegin (GL_LINE_LOOP);
	glVertex3f(x,  y,  0.0f);
	glVertex3f(x+w,y,  0.0f);
	glVertex3f(x+w,y+h,0.0f);
	glVertex3f(x,  y+h,0.0f);
	glEnd();
	glColor4f(1,1,1,1);
}

void drawTexturedQuad(float x,float y,float w,float h,float sw,float sh)
{
	glBegin (GL_QUADS);
	glTexCoord2f(0,  0); glVertex3f(x,  y,  0.0f);
	glTexCoord2f(sw, 0); glVertex3f(x+w,y,  0.0f);
	glTexCoord2f(sw,sh); glVertex3f(x+w,y+h,0.0f);
	glTexCoord2f(0, sh); glVertex3f(x,  y+h,0.0f);
	glEnd();
}

void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh)
{
	glBegin (GL_QUADS);
	glTexCoord2f(sx,   sy);    glVertex3f(x,  y,  0.0f);
	glTexCoord2f(sx+sw,sy);    glVertex3f(x+w,y,  0.0f);
	glTexCoord2f(sx+sw,sy+sh); glVertex3f(x+w,y+h,0.0f);
	glTexCoord2f(sx,   sy+sh); glVertex3f(x,  y+h,0.0f);
	glEnd();
}

unsigned int loadTexture(const char* name)
{
	unsigned int tex;
	short w,h;
	unsigned char bits,flags;


	std::ifstream f(name,std::ifstream::binary|std::ifstream::in);
	
	if (!f.is_open()) return 0;
	f.seekg(12,std::ifstream::beg);
	f.read((char*) &w,2);
	f.read((char*) &h,2);
	f.read((char*) &bits,1);
	f.read((char*) &flags,1);


	unsigned char* data=new unsigned char[w*h*bits/8];
	f.read((char*) data,w*h*bits/8);
	f.close();
	

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, bits/8, w,h, 0, bits == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,data);

	delete [] data;

	return tex;
}


unsigned int createTexture(int w,int h,unsigned int format=GL_RGB)
{
	unsigned int tex_id;
	glGenTextures(1,&tex_id);
	glBindTexture(GL_TEXTURE_2D,tex_id);
	unsigned char* b=new unsigned char[w*h*4];
	memset(b,0,w*h*4);

	glTexImage2D(GL_TEXTURE_2D,0,(format == GL_RGB) ? 3 : 4,w,h,0,format,GL_UNSIGNED_BYTE,b);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	delete [] b;
	return tex_id;
}

void display()
{
#ifdef __ZBUFFER
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#else
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	draw();

	glutSwapBuffers();

	static unsigned long time=GetTickCount();
	unsigned long t=GetTickCount();

	float diff=(t-time)/1000.0f;
	if (diff > 0.25f)
		diff=0.05f; // prevent spikes (usually happen on app load)
	update(diff);

	static unsigned int fps_timer=time,fps_counter=0;
	if (t-fps_timer >= 250)
	{
		char title[512],debug[256]="";

		setDebugTitle(debug);
		sprintf(title,"%s: %d FPS; %s",window_name.c_str(),fps_counter*4,debug);
		glutSetWindowTitle(title);
		fps_counter=0;
		fps_timer=t;
	}
	else fps_counter++;

	time=t;

}

void reshape(int w,int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glViewport(0, 0, w, h);
#ifdef __3D_PROJECTION
    gluPerspective(45,(float) window_w/window_h,10,10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#else
	gluOrtho2D(0,800,600,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif

}

void keyboard(unsigned char key,int x,int y)
{
    if (key == 27) // esc
    {
        destroy();
        exit(0);
    }
    else OnKeyPress(key);
}

void keyboard_special(int key,int x,int y)
{
    OnKeyPress(key);
}

void mouse(int button,int state, int x, int y)
{
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
	{
		mx=((float) x/glutGet(GLUT_WINDOW_WIDTH))*window_w;
		my=((float) y/glutGet(GLUT_WINDOW_HEIGHT))*window_h;
		OnClick(mx,my);
	}
}

void motion(int x,int y)
{
	mx=((float) x/glutGet(GLUT_WINDOW_WIDTH))*window_w;
	my=((float) y/glutGet(GLUT_WINDOW_HEIGHT))*window_h;
}

void getCursorPos(float* xout,float* yout)
{
	*xout=mx;
	*yout=my;
}

int main(int argc,char** argv)
{
	glutInit(&argc, argv);
#ifdef __ZBUFFER
	glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
#else
	glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGBA);
#endif
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-window_w)/2,(glutGet(GLUT_SCREEN_HEIGHT)-window_h)/2);
	glutInitWindowSize(window_w,window_h);
	glutCreateWindow(window_name.c_str());
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutSpecialFunc(keyboard_special);
	glutIdleFunc(display);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	try { glutMainLoop(); }
	catch (void*) {}

	destroy();

    glutDestroyWindow(0);

	return 0;
}




