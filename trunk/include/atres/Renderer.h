/// @file
/// @version 3.43
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
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

namespace atres
{
	class Font;
	template <class T>
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
		/// @brief Option to use old, non-smart line-break parsing for ideographs.
		HL_DEFINE_GET(bool, useLegacyLineBreakParsing, UseLegacyLineBreakParsing);
		void setUseLegacyLineBreakParsing(bool value);
		/// @brief When turned on, this regards ideographs as words when segmenting text into lines.
		/// @note This is particularly useful when having Japanese or Chinese text. Korean text has actual spaces so this is not necessary, but still useful.
		HL_DEFINE_GET(bool, useIdeographWords, UseIdeographWords);
		void setUseIdeographWords(bool value);
		/// @brief When turned off, this turns justified text into left_wrapped. This is to counter languages with problematic characters.
		HL_DEFINE_GET(bool, justifiedEnabled, JustifiedEnabled);
		void setJustifiedEnabled(bool value);
		void setDefaultFont(chstr name);
		hstr getDefaultFont();
		bool hasFont(chstr name);
		void setCacheSize(int value);

		/// @param[in] allowDefault Allows font to be set as default if necessary
		void registerFont(Font* font, bool allowDefault = true);
		void destroyAllFonts();
		void destroyFont(Font* font);
		void unregisterFont(Font* font);
		void registerFontAlias(chstr name, chstr alias);
		Font* getFont(chstr name);
		
		const hmap<hstr, Font*>& getFonts() const { return this->fonts; }
		void addColor(chstr key, chstr value);

		void analyzeText(chstr fontName, chstr text);
		hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
		harray<RenderLine> removeOutOfBoundLines(grect rect, harray<RenderLine> lines);
		harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float x, float lineHeight, float offsetY);
		harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float y, float lineWidth);
		harray<RenderWord> createRenderWords(grect rect, chstr text, harray<FormatTag> tags);
		harray<RenderLine> createRenderLines(grect rect, chstr text, harray<FormatTag> tags, Alignment horizontal, Alignment vertical, gvec2 offset = gvec2(), bool keepWrappedSpaces = false);
		RenderText createRenderText(grect rect, harray<RenderLine> lines, harray<FormatTag> tags);
		harray<RenderSequence> optimizeSequences(harray<RenderSequence>& sequences);
	
		void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		void drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		void drawText(grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		void drawTextUnformatted(grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		harray<RenderLine> makeRenderLines(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		harray<RenderLine> makeRenderLinesUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal = LEFT,
			Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());

		DEPRECATED_ATTRIBUTE void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, april::Color color = april::Color::White, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
		DEPRECATED_ATTRIBUTE void drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal = LEFT, Alignment vertical = CENTER, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255, gvec2 offset = gvec2());
	
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

		DEPRECATED_ATTRIBUTE float getFontHeight(chstr fontName);
		DEPRECATED_ATTRIBUTE float getFontLineHeight(chstr fontName);
		DEPRECATED_ATTRIBUTE inline void registerFontResource(Font* font, bool allowDefault = true) { this->registerFont(font, allowDefault); }
		DEPRECATED_ATTRIBUTE inline void destroyAllFontResources() { this->destroyAllFonts(); }
		DEPRECATED_ATTRIBUTE inline void destroyFontResource(Font* font) { this->destroyFont(font); }
		DEPRECATED_ATTRIBUTE inline void unregisterFontResource(Font* font) { this->unregisterFont(font); }
		DEPRECATED_ATTRIBUTE inline void registerFontResourceAlias(chstr name, chstr alias) { this->registerFontAlias(name, alias); }
		DEPRECATED_ATTRIBUTE inline Font* getFontResource(chstr name) { return this->getFont(name); }

	protected:
		hmap<hstr, Font*> fonts;
		Font* defaultFont;
		hmap<hstr, hstr> colors;
		gvec2 shadowOffset;
		april::Color shadowColor;
		float borderOffset;
		april::Color borderColor;
		bool globalOffsets;
		bool useLegacyLineBreakParsing;
		bool useIdeographWords;
		bool justifiedEnabled;
		Cache<CacheEntryText>* cacheText;
		Cache<CacheEntryText>* cacheTextUnformatted;
		Cache<CacheEntryLines>* cacheLines;
		Cache<CacheEntryLines>* cacheLinesUnformatted;
		Cache<CacheEntryLine>* cacheLine;

		void _updateCache();

		void _initializeFormatTags(harray<FormatTag>& tags);
		void _initializeLineProcessing(harray<RenderLine> lines = harray<RenderLine>());
		void _initializeRenderSequences();
		void _checkFormatTags(chstr text, int index);
		void _processFormatTags(chstr text, int index);
		void _checkSequenceSwitch();
		RenderLine _calculateFittingLine(grect rect, chstr text, harray<FormatTag> tags);
		bool _checkTextures();
		harray<FormatTag> _makeDefaultTags(april::Color color, chstr fontName, hstr& text);
		harray<FormatTag> _makeDefaultTagsUnformatted(april::Color color, chstr fontName);

		void _drawRenderText(RenderText& renderText, april::Color);
		void _drawRenderSequence(RenderSequence& sequence, april::Color);

	private:
		harray<FormatTag> _tags;
		harray<FormatTag> _stack;
		FormatTag _currentTag;
		FormatTag _nextTag;

		hstr _fontName;
		Font* _font;
		hmap<unsigned int, CharacterDefinition> _characters;
		CharacterDefinition* _character;
		float _height;
		float _lineHeight;
		float _descender;
		float _fontScale;
		float _textScale;
		float _scale;

		harray<RenderSequence> _textSequences;
		RenderSequence _textSequence;
		harray<RenderSequence> _shadowSequences;
		RenderSequence _shadowSequence;
		harray<RenderSequence> _borderSequences;
		RenderSequence _borderSequence;
		RenderRectangle _renderRect;

		april::Color _textColor;
		april::Color _shadowColor;
		april::Color _borderColor;
		hstr _hex;
		int _effectMode;
		int _alpha;

		harray<RenderLine> _lines;
		RenderLine _line;
		RenderWord _word;

		april::Texture* _texture;
		unsigned int _code;

		CacheEntryText _cacheEntryText;
		CacheEntryLines _cacheEntryLines;
		CacheEntryLine _cacheEntryLine;

	};
	
	atresExport extern Renderer* renderer;

};

#endif

