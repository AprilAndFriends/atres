/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_H
#define ATRES_H

#include <string>
#include "AtresExport.h"

namespace Atres
{
	enum Alignment
	{
		LEFT,RIGHT,CENTER
	};

	enum Effect
	{
		NONE,SHADOW,BORDER,BORDER_EX
	};
	
	class Font;
	class RenderInterface;

    AtresFnExport void init(RenderInterface* iface);
    AtresFnExport void destroy();
	
	AtresFnExport float drawText(float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawWrappedText(float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawText(std::string font_name,float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawWrappedText(std::string font_name,float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float getTextWidth(std::string font_name,std::string text);
	AtresFnExport float getWrappedTextHeight(std::string font_name,float w_max,std::string text);

	AtresFnExport void setRenderInterface(RenderInterface* iface);
	AtresFnExport RenderInterface* getRenderInterface();

	AtresFnExport void setDefaultFont(std::string name);
	AtresFnExport void loadFont(std::string filename);
    AtresFnExport Font* getFont(std::string name);
};

#endif

