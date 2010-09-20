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

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hstring.h>

#include "AtresExport.h"

namespace Atres
{
	struct AtresExport CharacterRenderOp
	{
		April::Texture* texture;
		grect src;
		grect dest;
		April::Color color;
		unsigned char italic;
		unsigned char underline;
		unsigned char strikethrough;
	};
	
	enum Alignment
	{
		LEFT,
		RIGHT,
		CENTER,
		LEFT_WRAPPED,
		RIGHT_WRAPPED,
		CENTER_WRAPPED
	};

	enum Effect
	{
		NONE,
		SHADOW,
		BORDER,
		BORDER_4
	};
	
	class Font;

    AtresFnExport void init();
    AtresFnExport void destroy();
	AtresFnExport void setLogFunction(void (*fnptr)(chstr));
	void logMessage(chstr message, chstr prefix = "[atres] ");
	void atres_writelog(chstr message);
	
	AtresFnExport float drawText(float x, float y, float w_max, float h_max, chstr text, float r = 1, float g = 1, float b = 1, float a = 1, Alignment alignment = LEFT, Effect effect = NONE);
	AtresFnExport float drawText(chstr fontName, float x, float y, float w_max, float h_max, chstr text, float r = 1, float g = 1, float b = 1, float a = 1, Alignment alignment = LEFT, Effect effect = NONE);
	AtresFnExport float drawWrappedText(float x, float y, float w_max, float h_max, chstr text, float r = 1, float g = 1, float b = 1, float a = 1, Alignment alignment = LEFT, Effect effect = NONE);
	AtresFnExport float drawWrappedText(chstr fontName, float x, float y, float w_max, float h_max, chstr text, float r = 1, float g = 1, float b = 1, float a = 1, Alignment alignment = LEFT, Effect effect = NONE);
	AtresFnExport float getTextWidth(chstr font_name,chstr text);
	AtresFnExport float getTextHeight(chstr font_name,chstr text);
	AtresFnExport float getWrappedTextHeight(chstr font_name,float w_max,chstr text);
	AtresFnExport int getWrappedTextCount(chstr font_name,float w_max,float h_max,chstr text);

	AtresFnExport void setDefaultFont(chstr name);
	AtresFnExport void loadFont(chstr filename);
    AtresFnExport Font* getFont(chstr name);
};

#endif

