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
#include <gtypes/Vector2.h>
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
		bool italic;
		bool underline;
		bool strikethrough;
	};
	
	enum Alignment
	{
		LEFT,
		RIGHT,
		CENTER,
		LEFT_WRAPPED,
		RIGHT_WRAPPED,
		CENTER_WRAPPED,
		TOP,
		BOTTOM
	};

	enum Effect
	{
		NONE,
		SHADOW,
		BORDER
	};
	
	class Font;

    AtresFnExport void init();
    AtresFnExport void destroy();
	AtresFnExport void setLogFunction(void (*fnptr)(chstr));
	void logMessage(chstr message, chstr prefix = "[atres] ");
	void atres_writelog(chstr message);
	
	AtresFnExport void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextShadowed(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextBordered(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	
	AtresFnExport void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
	AtresFnExport void drawTextShadowed(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
	AtresFnExport void drawTextBordered(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
	AtresFnExport float getFontHeight(chstr fontName);
	AtresFnExport float getTextWidth(chstr fontName, chstr text);
	AtresFnExport float getTextHeight(chstr fontName, chstr text, float maxWidth);
	AtresFnExport int getTextCount(chstr fontName, chstr text, float maxWidth, float maxHeight);
	
	AtresFnExport void setDefaultFont(chstr name);
	AtresFnExport void loadFont(chstr filename);
    AtresFnExport Font* getFont(chstr name);
	
	AtresFnExport gvec2 getShadowOffset();
	AtresFnExport void setShadowOffset(gvec2 value);
	AtresFnExport April::Color getShadowColor();
	AtresFnExport void setShadowColor(April::Color value);
	AtresFnExport float getBorderOffset();
	AtresFnExport void setBorderOffset(float value);
	AtresFnExport April::Color getBorderColor();
	AtresFnExport void setBorderColor(April::Color value);
};

#endif

