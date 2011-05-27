/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents a font renderer.

#ifndef ATRES_RENDERER_H
#define ATRES_RENDERER_H

#include <april/Color.h>
#include <april/Texture.h>
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
	
	struct RenderRectangle
	{
		grect src;
		grect dest;
	};
	
	struct RenderSequence
	{
		april::Texture* texture;
		april::Color color;
		harray<RenderRectangle> rectangles;
		RenderSequence() : texture(NULL) { }
	};
	
	struct RenderLine
	{
		hstr text;
		grect rect;
		int start;
		RenderLine() : start(0) { }
	};
	
	struct FormatTag
	{
		FormatTagType type;
		hstr data;
		int start;
		int count;
		FormatTag() : type(ESCAPE), start(0), count(0) { }
	};
	
	struct CacheEntry
	{
		hstr fontName;
		int index;
		gvec2 size;
		Alignment horizontal;
		Alignment vertical;
		april::Color color;
		gvec2 offset;
		harray<RenderSequence> sequences;
		CacheEntry() : index(0), horizontal(CENTER_WRAPPED), vertical(CENTER) { }
	};

	class Font;

	class atresExport Renderer
	{
	public:
		Renderer();
		~Renderer();

		void drawRenderSequence(RenderSequence& sequence);
		void loadFont(chstr filename);
	
		hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
		harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight);
		harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
		harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
		harray<RenderSequence> createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
		harray<RenderSequence> optimizeSequences(harray<RenderSequence> sequences);
	
		void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	
		void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
		void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	
		float getFontHeight(chstr fontName);
		float getTextWidth(chstr fontName, chstr text);
		float getTextHeight(chstr fontName, chstr text, float maxWidth);
		int getTextCount(chstr fontName, chstr text, float maxWidth);
		float getTextWidthUnformatted(chstr fontName, chstr text);
		float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
		int getTextCountUnformatted(chstr fontName, chstr text, float maxWidth);
		hstr prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags);
		harray<FormatTag> prepareTags(chstr fontName);
		RenderLine getFittingLine(grect rect, chstr text, harray<FormatTag> tags);
	
		void setDefaultFont(chstr name);
		Font* getFont(chstr name);
		bool hasFont(chstr name);
		void setCacheSize(int value);
		void updateCache();
		void setGlobalOffsets(bool value);
		gvec2 getShadowOffset();
		void setShadowOffset(gvec2 value);
		april::Color getShadowColor();
		void setShadowColor(april::Color value);
		float getBorderOffset();
		void setBorderOffset(float value);
		april::Color getBorderColor();
		void setBorderColor(april::Color value);
		void addColor(chstr key, chstr value);


	};
	
};

#endif

