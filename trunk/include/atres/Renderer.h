/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.62
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
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "Utility.h"

#define BUFFER_MAX_CHARACTERS 16384

namespace atres
{
	class FontResource;
	template <class K, class V>
	class Cache;

	class atresExport Renderer
	{
	public:
		Renderer();
		~Renderer();

		HL_DEFINE_GET(gvec2, shadowOffset, ShadowOffset);
		void setShadowOffset(gvec2 value);
		HL_DEFINE_GET(april::Color, shadowColor, ShadowColor);
		void setShadowColor(april::Color value);
		HL_DEFINE_GET(float, borderOffset, BorderOffset);
		void setBorderOffset(float value);
		HL_DEFINE_GET(april::Color, borderColor, BorderColor);
		void setBorderColor(april::Color value);
		HL_DEFINE_GETSET(bool, globalOffsets, GlobalOffsets);
		void setDefaultFont(chstr name);
		bool hasFont(chstr name);
		void setCacheSize(int value);

		void registerFontResource(FontResource* fontResource);
		void destroyFontResource(FontResource* resource);
		void unregisterFontResource(FontResource* resource);
		FontResource* getFontResource(chstr name);
		void addColor(chstr key, chstr value);

		void analyzeText(chstr text);
		hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
		harray<RenderLine> removeOutOfBoundLines(grect rect, harray<RenderLine> lines);
		harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight, float correctedHeight);
		harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
		harray<RenderWord> createRenderWords(grect rect, chstr text, harray<FormatTag> tags);
		harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2());
		RenderText createRenderText(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
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
		float getFontLineHeight(chstr fontName);
		float getTextWidth(chstr fontName, chstr text);
		float getTextHeight(chstr fontName, chstr text, float maxWidth);
		int getTextCount(chstr fontName, chstr text, float maxWidth);
		float getTextWidthUnformatted(chstr fontName, chstr text);
		float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
		int getTextCountUnformatted(chstr fontName, chstr text, float maxWidth);
		hstr prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags);
		harray<FormatTag> prepareTags(chstr fontName);
		RenderLine getFittingLine(chstr fontName, grect rect, chstr text, harray<FormatTag> tags);
		void clearCache();

	protected:
		hmap<hstr, FontResource*> fonts;
		FontResource* defaultFont;
		hmap<hstr, hstr> colors;
		gvec2 shadowOffset;
		april::Color shadowColor;
		float borderOffset;
		april::Color borderColor;
		bool globalOffsets;

		void _updateCache();

		void _initializeFormatTags(harray<FormatTag>& tags);
		void _initializeLineProcessing(harray<RenderLine> lines = harray<RenderLine>());
		void _initializeRenderSequences();
		void _checkFormatTags(chstr text, int index);
		void _processFormatTags(chstr text, int index);
		void _checkSequenceSwitch();
		RenderLine _calculateFittingLine(grect rect, chstr text, harray<FormatTag> tags);
		bool _checkTextures();

		void _drawRenderText(RenderText& renderText, april::Color);
		void _drawRenderSequence(RenderSequence& sequence, april::Color);

	protected:
		Cache<CacheKeySequence, RenderText>* cache;
		Cache<CacheKeySequence, RenderText>* cacheUnformatted;
		Cache<CacheKeyLine, RenderLine>* cacheLines;

	private:
		harray<FormatTag> _tags;
		harray<FormatTag> _stack;
		FormatTag _currentTag;
		FormatTag _nextTag;

		hstr _fontName;
		FontResource* _fontResource;
		hmap<unsigned int, CharacterDefinition> _characters;
		CharacterDefinition* _character;
		float _height;
		float _lineHeight;
		float _correctedHeight;
		float _scale;

		harray<RenderSequence> _textSequences;
		RenderSequence _textSequence;
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

		CacheKeySequence _cacheKeySequence;
		CacheKeyLine _cacheKeyLine;
		RenderText _currentRenderText;
		RenderLine _currentLine;

	};
	
	atresExport extern Renderer* renderer;

};

#endif

