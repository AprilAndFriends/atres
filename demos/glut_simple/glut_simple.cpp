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
#include "../demo_basecode.h"
#include <string>
#include "atres/Atres.h"

//unsigned int tex_id;

std::string window_name="glut_simple";
int window_w=800,window_h=600;

void draw()
{
	//glBindTexture(GL_TEXTURE_2D,tex_id);

	Atres::drawText(100,100,"drawText() call");
	Atres::drawWrappedText(200,200,230,"drawWrappedText() call wraps text into multiple lines",1,1,0);
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
