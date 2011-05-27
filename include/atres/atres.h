/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions for text rendering.

#ifndef ATRES_H
#define ATRES_H

#include <april/Color.h>
#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "Renderer.h"
#include "atresExport.h"

namespace atres
{
	class Font;
	
    atresFnExport void init();
    atresFnExport void destroy();
	atresFnExport void setLogFunction(void (*fnptr)(chstr));
	atresFnExport void log(chstr message, chstr prefix = "[atres] ");
	atresFnExport void loadFont(chstr filename);
	
	unsigned int getCharUtf8(const char* s, int* char_len_out);
	
	atresFnExport hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
	atresFnExport harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight);
	atresFnExport harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
	atresFnExport harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
	atresFnExport harray<RenderSequence> createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
	atresFnExport harray<RenderSequence> optimizeSequences(harray<RenderSequence> sequences);
	
	atresFnExport void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	atresFnExport void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	atresFnExport void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	atresFnExport void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	
	atresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	atresFnExport void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2()) DEPRECATED_ATTRIBUTE;
	
	atresFnExport float getFontHeight(chstr fontName);
	atresFnExport float getTextWidth(chstr fontName, chstr text);
	atresFnExport float getTextHeight(chstr fontName, chstr text, float maxWidth);
	atresFnExport int getTextCount(chstr fontName, chstr text, float maxWidth);
	atresFnExport float getTextWidthUnformatted(chstr fontName, chstr text);
	atresFnExport float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
	atresFnExport int getTextCountUnformatted(chstr fontName, chstr text, float maxWidth);
	atresFnExport hstr prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags);
	atresFnExport harray<FormatTag> prepareTags(chstr fontName);
	atresFnExport RenderLine getFittingLine(grect rect, chstr text, harray<FormatTag> tags);
	
	atresFnExport void setDefaultFont(chstr name);
    atresFnExport Font* getFont(chstr name);
	atresFnExport bool hasFont(chstr name);
	atresFnExport void setCacheSize(int value);
	atresFnExport void updateCache();
	atresFnExport void setGlobalOffsets(bool value);
	atresFnExport gvec2 getShadowOffset();
	atresFnExport void setShadowOffset(gvec2 value);
	atresFnExport april::Color getShadowColor();
	atresFnExport void setShadowColor(april::Color value);
	atresFnExport float getBorderOffset();
	atresFnExport void setBorderOffset(float value);
	atresFnExport april::Color getBorderColor();
	atresFnExport void setBorderColor(april::Color value);
	atresFnExport void addColor(chstr key, chstr value);

};

#endif

