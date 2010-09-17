/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>
#include "../demo_basecode.h"
#include "atres/Atres.h"

//unsigned int tex_id;

hstr window_name="glut_simple";
int window_w=800,window_h=600;

void drawRect(float x,float y,float w,float h)
{
	glBindTexture(GL_TEXTURE_2D,0);
	
	glBegin(GL_LINE_LOOP);
	
	glVertex3f(x,y,0);
	glVertex3f(x+w,y,0);
	glVertex3f(x+w,y+h,0);
	glVertex3f(x,y+h,0);
	
	
	glEnd();
}

void draw()
{
	//glBindTexture(GL_TEXTURE_2D,tex_id);

	glColor3f(1,1,1);

	char mltext[]="Label7: V_Warden_1_DAMAGE";
	drawRect(200,200,230,Atres::getWrappedTextHeight("",230,mltext));

	Atres::drawText("Arial:0.5",10,500,"a scaled font");
	Atres::drawText(100,100,"drawText() call");
	Atres::drawWrappedText(200+230/2,200,230,mltext,1,1,0,1,Atres::CENTER);
	//drawTexturedQuad(0,0,800,600,1,1);
}

void update(float time_increase)
{

}

void OnKeyPress(int key)
{
 
}

void OnClick(float x,float y)
{

}

void setDebugTitle(char* out)
{
}

void init()
{
	//tex_id=loadTexture("../media/arial.tga");
	Atres::init(&ogl_iface);
	Atres::loadFont("../media/arial.font");
	glDisable(GL_CULL_FACE);
}

void destroy()
{
	Atres::destroy();
}
