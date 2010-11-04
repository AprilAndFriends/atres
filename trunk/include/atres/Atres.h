/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_H
#define ATRES_H

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "AtresExport.h"

#define BUFFER_MAX_CHARACTERS 32768

namespace Atres
{
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
	
	enum FormatTagType
	{
		ESCAPE,
		FORMAT_NORMAL,
		FORMAT_SHADOW,
		FORMAT_BORDER,
		FORMAT_COLOR,
		FORMAT_FONT,
		CLOSE
	};
	
	struct AtresExport RenderRectangle
	{
		grect src;
		grect dest;
		RenderRectangle() : src(), dest() { }
	};
	
	struct AtresExport RenderSequence
	{
		April::Texture* texture;
		April::Color color;
		harray<RenderRectangle> rectangles;
		RenderSequence() : texture(NULL), color(April::Color()), rectangles(harray<RenderRectangle>()) { }
	};
	
	struct AtresExport RenderLine
	{
		hstr text;
		grect rect;
		int start;
		RenderLine() : text(""), rect(), start(0) { }
	};
	
	struct AtresExport FormatTag
	{
		FormatTagType type;
		hstr data;
		int start;
		int count;
		FormatTag() : type(ESCAPE), data(""), start(0), count(0) { }
	};
	
	struct CacheEntry
	{
		hstr fontName;
		int index;
		gvec2 size;
		Alignment horizontal;
		Alignment vertical;
		April::Color color;
		gvec2 offset;
		harray<RenderSequence> sequences;
		CacheEntry() : fontName(""), index(0), size(gvec2()), horizontal(CENTER_WRAPPED), vertical(CENTER),
			color(April::Color()), offset(gvec2()), sequences(harray<RenderSequence>()) { }
	};
	
	class Font;
	
    AtresFnExport void init();
    AtresFnExport void destroy();
	AtresFnExport void setLogFunction(void (*fnptr)(chstr));
	void logMessage(chstr message, chstr prefix = "[atres] ");
	void atres_writelog(chstr message);
	AtresFnExport void loadFont(chstr filename);
	
	unsigned int getCharUtf8(const char* s, int* char_len_out);
	
	hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
	harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight);
	harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
	harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
	harray<RenderSequence> createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
	harray<RenderSequence> optimizeSequences(harray<RenderSequence> sequences);
	
	AtresFnExport void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	
	AtresFnExport void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	AtresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
	AtresFnExport float getFontHeight(chstr fontName);
	AtresFnExport float getTextWidth(chstr fontName, chstr text);
	AtresFnExport float getTextHeight(chstr fontName, chstr text, float maxWidth);
	AtresFnExport int getTextCount(chstr fontName, chstr text, float maxWidth);
	AtresFnExport float getTextWidthUnformatted(chstr fontName, chstr text);
	AtresFnExport float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
	AtresFnExport int getTextCountUnformatted(chstr fontName, chstr text, float maxWidth);
	hstr prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags);
	harray<FormatTag> prepareTags(chstr fontName);
	RenderLine getFittingLine(grect rect, chstr text, harray<FormatTag> tags);
	
	AtresFnExport void setDefaultFont(chstr name);
    AtresFnExport Font* getFont(chstr name);
	AtresFnExport bool hasFont(chstr name);
	AtresFnExport void setCacheSize(int value);
	AtresFnExport void updateCache();
	AtresFnExport void setGlobalOffsets(bool value);
	AtresFnExport gvec2 getShadowOffset();
	AtresFnExport void setShadowOffset(gvec2 value);
	AtresFnExport April::Color getShadowColor();
	AtresFnExport void setShadowColor(April::Color value);
	AtresFnExport float getBorderOffset();
	AtresFnExport void setBorderOffset(float value);
	AtresFnExport April::Color getBorderColor();
	AtresFnExport void setBorderColor(April::Color value);
	AtresFnExport void addColor(chstr key, chstr value);
};

#endif

