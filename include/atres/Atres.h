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
		NONE,SHADOW,BORDER
	};
	
	class Font;
	class RenderInterface;

    void AtresFnExport init(RenderInterface* iface);
    void AtresFnExport destroy();
	
	float AtresFnExport drawText(float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float AtresFnExport drawWrappedText(float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float AtresFnExport drawText(std::string font_name,float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float AtresFnExport drawWrappedText(std::string font_name,float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	float AtresFnExport getTextWidth(std::string font_name,std::string text);
	float AtresFnExport getWrappedTextHeight(std::string font_name,float w_max,std::string text);

	void AtresFnExport setRenderInterface(RenderInterface* iface);
	RenderInterface* AtresFnExport getRenderInterface();

	void AtresFnExport setDefaultFont(std::string name);
	void AtresFnExport loadFont(std::string filename);
    Font* AtresFnExport getFont(std::string name);
};

#endif

