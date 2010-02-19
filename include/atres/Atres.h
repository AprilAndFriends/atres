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
#ifndef ATRES_H
#define ATRES_H

#include <string>

namespace Atres
{
	enum Alignment
	{
		LEFT,RIGHT,CENTER
	};

	enum Effect
	{
		NONE,SHADOW,BORDER
	};
	
	class Font;
	class RenderInterface;

    void init(RenderInterface* iface);
    void destroy();
	
	float drawText(float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float drawWrappedText(float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float drawText(std::string font_name,float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float drawWrappedText(std::string font_name,float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float getTextWidth(std::string font_name,std::string text,Alignment alignment=LEFT);
	float getWrappedTextHeight(std::string font_name,float w_max,std::string text,Alignment alignment=LEFT);

	void setRenderInterface(RenderInterface* iface);
	RenderInterface* getRenderInterface();

	void setDefaultFont(std::string name);
	void loadFont(std::string filename);
    Font* getFont(std::string name);
};

#endif

