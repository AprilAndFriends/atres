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

#include <hltypes/hstring.h>
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
	
	AtresFnExport float drawText(float x,float y,chstr text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawWrappedText(float x,float y,float w_max,chstr text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawText(chstr font_name,float x,float y,chstr text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float drawWrappedText(chstr font_name,float x,float y,float w_max,chstr text,float r=1,float g=1,float b=1,float a=1,Alignment alignment=LEFT,Effect effect=NONE);
	AtresFnExport float getTextWidth(chstr font_name,chstr text);
	AtresFnExport float getTextHeight(chstr font_name,chstr text);
	AtresFnExport float getWrappedTextHeight(chstr font_name,float w_max,chstr text);

	AtresFnExport void setRenderInterface(RenderInterface* iface);
	AtresFnExport RenderInterface* getRenderInterface();

	AtresFnExport void setDefaultFont(chstr name);
	AtresFnExport void loadFont(chstr filename);
    AtresFnExport Font* getFont(chstr name);
};

#endif

