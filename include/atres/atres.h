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

#include "atresExport.h"

#define BUFFER_MAX_CHARACTERS 8192

namespace atres
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
	
	struct atresExport RenderRectangle
	{
		grect src;
		grect dest;
		RenderRectangle() : src(), dest() { }
	};
	
	struct atresExport RenderSequence
	{
		April::Texture* texture;
		April::Color color;
		harray<RenderRectangle> rectangles;
		RenderSequence() : texture(NULL), color(April::Color()), rectangles(harray<RenderRectangle>()) { }
	};
	
	struct atresExport RenderLine
	{
		hstr text;
		grect rect;
		int start;
		RenderLine() : text(""), rect(), start(0) { }
	};
	
	struct atresExport FormatTag
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
	
    atresFnExport void init();
    atresFnExport void destroy();
	atresFnExport void setLogFunction(void (*fnptr)(chstr));
	void logMessage(chstr message, chstr prefix = "[atres] ");
	void atres_writelog(chstr message);
	atresFnExport void loadFont(chstr filename);
	
	unsigned int getCharUtf8(const char* s, int* char_len_out);
	
	hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
	harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight);
	harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
	harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
	harray<RenderSequence> createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
	harray<RenderSequence> optimizeSequences(harray<RenderSequence> sequences);
	
	atresFnExport void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2());
	atresFnExport void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2());
	atresFnExport void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2());
	atresFnExport void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2());
	
	atresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, April::Color color = April::Color(), float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, float angle = 0.0f, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	
	atresFnExport float getFontHeight(chstr fontName);
	atresFnExport float getTextWidth(chstr fontName, chstr text);
	atresFnExport float getTextHeight(chstr fontName, chstr text, float maxWidth);
	atresFnExport int getTextCount(chstr fontName, chstr text, float maxWidth);
	atresFnExport float getTextWidthUnformatted(chstr fontName, chstr text);
	atresFnExport float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
	atresFnExport int getTextCountUnformatted(chstr fontName, chstr text, float maxWidth);
	hstr prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags);
	harray<FormatTag> prepareTags(chstr fontName);
	RenderLine getFittingLine(grect rect, chstr text, harray<FormatTag> tags);
	
	atresFnExport void setDefaultFont(chstr name);
    atresFnExport Font* getFont(chstr name);
	atresFnExport bool hasFont(chstr name);
	atresFnExport void setCacheSize(int value);
	atresFnExport void updateCache();
	atresFnExport void setGlobalOffsets(bool value);
	atresFnExport gvec2 getShadowOffset();
	atresFnExport void setShadowOffset(gvec2 value);
	atresFnExport April::Color getShadowColor();
	atresFnExport void setShadowColor(April::Color value);
	atresFnExport float getBorderOffset();
	atresFnExport void setBorderOffset(float value);
	atresFnExport April::Color getBorderColor();
	atresFnExport void setBorderColor(April::Color value);
	atresFnExport void addColor(chstr key, chstr value);
};

#endif

