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
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "Utility.h"

#define BUFFER_MAX_CHARACTERS 8192

namespace atres
{
	class FontResource;

	class atresExport Renderer
	{
	public:
		Renderer();
		~Renderer();

		void drawRenderSequence(RenderSequence& sequence);
		void registerFontResource(FontResource* fontResource);
	
		hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
		harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight);
		harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
		harray<RenderWord> createRenderWords(grect rect, chstr text, harray<FormatTag> tags, bool limitWidth = false);
		harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
		harray<RenderSequence> createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
		harray<RenderSequence> optimizeSequences(harray<RenderSequence>& sequences);
	
		void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawText(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
	
		DEPRECATED_ATTRIBUTE void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = APRIL_COLOR_WHITE, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
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
		FontResource* getFontResource(chstr name);
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

	private:
		harray<FormatTag> _tags;
		harray<FormatTag> _stack;
		FormatTag _currentTag;
		FormatTag _nextTag;

		hstr _fontName;
		FontResource* _fontResource;
		hmap<unsigned int, CharacterDefinition> _characters;
		CharacterDefinition* _character;
		float _lineHeight;
		float _scale;

		harray<RenderSequence> _sequences;
		RenderSequence _sequence;
		harray<RenderSequence> _shadowSequences;
		RenderSequence _shadowSequence;
		harray<RenderSequence> _borderSequences;
		RenderSequence _borderSequence;
		RenderRectangle _renderRect;

		april::Color _color;
		hstr _hex;
		int _effectMode;
		int _alpha;

		harray<RenderLine> _lines;
		RenderLine _line;
		RenderWord _word;

		april::Texture* _texture;
		unsigned int _code;
		bool _colorChanged;

		bool _needCache;
		CacheEntry _entry;
		grect _drawRect;
		harray<RenderSequence> _currentSequences;

		void _initializeFormatTags(harray<FormatTag>& tags);
		void _initializeLineProcessing(harray<RenderLine> lines = harray<RenderLine>());
		void _initializeRenderSequences();
		void _checkFormatTags(chstr text, int index);
		void _processFormatTags(chstr text, int index);


	};
	
};

#endif

