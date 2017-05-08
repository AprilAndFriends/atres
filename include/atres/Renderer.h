/// @file
/// @version 4.1
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
	class FontIconMap;
	template <typename T>
	class Cache;

	class atresExport Renderer
	{
	public:
		Renderer();
		~Renderer();

		HL_DEFINE_GET(gvec2, shadowOffset, ShadowOffset);
		void setShadowOffset(cgvec2 value);
		HL_DEFINE_GET(april::Color, shadowColor, ShadowColor);
		void setShadowColor(const april::Color& value);
		HL_DEFINE_GET(float, borderThickness, BorderThickness);
		void setBorderThickness(float value);
		HL_DEFINE_GET(april::Color, borderColor, BorderColor);
		void setBorderColor(const april::Color& value);
		HL_DEFINE_GET(float, strikeThroughThickness, StrikeThroughThickness);
		void setStrikeThroughThickness(float value);
		HL_DEFINE_GET(float, underlineThickness, UnderlineThickness);
		void setUnderlineThickness(float value);
		HL_DEFINE_ISSET(globalOffsets, GlobalOffsets);
		/// @brief Option to use old, non-smart line-break parsing for ideographs.
		HL_DEFINE_IS(useLegacyLineBreakParsing, UseLegacyLineBreakParsing);
		void setUseLegacyLineBreakParsing(bool value);
		/// @brief When turned on, this regards ideographs as words when segmenting text into lines.
		/// @note This is particularly useful when having Japanese or Chinese text. Korean text has actual spaces so this is not necessary, but still useful.
		HL_DEFINE_IS(useIdeographWords, UseIdeographWords);
		void setUseIdeographWords(bool value);
		/// @brief Allows to turn justified text into another formatting. This is to counter languages with problematic characters.
		HL_DEFINE_GET(Horizontal, justifiedDefault, JustifiedDefault);
		void setJustifiedDefault(Horizontal value);
		hstr getDefaultFontName() const;
		void setDefaultFontName(chstr value);
		void setCacheSize(int value);

		bool hasFont(chstr name) const;

		/// @param[in] allowDefault Allows font to be set as default if necessary
		void registerFont(Font* font, bool allowDefault = true);
		void destroyAllFonts();
		void destroyFont(Font* font);
		void unregisterFont(Font* font);
		void registerFontAlias(chstr name, chstr alias);
		Font* getFont(chstr name);
		
		inline const hmap<hstr, Font*>& getFonts() const { return this->fonts; }

		void analyzeText(chstr fontName, chstr text);
		hstr analyzeFormatting(chstr text, harray<FormatTag>& tags);
		harray<RenderLine> removeOutOfBoundLines(const harray<RenderLine>& lines, cgrect rect);
		void verticalCorrection(harray<RenderLine>& lines, cgrect rect, Vertical vertical, float x, float lineHeight, float descender, float internalDescender);
		void horizontalCorrection(harray<RenderLine>& lines, cgrect rect, Horizontal horizontal, float y);
		harray<RenderWord> createRenderWords(cgrect rect, chstr text, const harray<FormatTag>& tags);
		harray<RenderLine> createRenderLines(cgrect rect, chstr text, const harray<FormatTag>& tags, Horizontal horizontal, Vertical vertical, cgvec2 offset = gvec2(), bool keepWrappedSpaces = false);
		RenderText createRenderText(cgrect rect, chstr text, const harray<RenderLine>& lines, const harray<FormatTag>& tags);
		harray<RenderSequence> optimizeSequences(harray<RenderSequence>& sequences);
		harray<RenderLiningSequence> optimizeSequences(harray<RenderLiningSequence>& sequences);

		void drawText(chstr fontName, cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());
		void drawTextUnformatted(chstr fontName, cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());
		void drawText(cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());
		void drawTextUnformatted(cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());
		harray<RenderLine> makeRenderLines(chstr fontName, cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());
		harray<RenderLine> makeRenderLinesUnformatted(chstr fontName, cgrect rect, chstr text, Horizontal horizontal = Horizontal::Left,
			Vertical vertical = Vertical::Center, const april::Color& color = april::Color::White, cgvec2 offset = gvec2());

		float getTextWidth(chstr fontName, chstr text);
		float getTextWidth(chstr text);
		float getTextWidthUnformatted(chstr fontName, chstr text);
		float getTextWidthUnformatted(chstr text);
		float getTextAdvanceX(chstr fontName, chstr text);
		float getTextAdvanceX(chstr text);
		float getTextAdvanceXUnformatted(chstr fontName, chstr text);
		float getTextAdvanceXUnformatted(chstr text);
		// @note If you used getTextWidth() or makeRenderLines() to get the text width, you probably want to add a very little number to that width (e.g. 0.01)
		// due to floating point rounding errors.
		// @see getTextWidth()
		// @see makeRenderLines()
		float getTextHeight(chstr fontName, chstr text, float maxWidth);
		// @note If you used getTextWidth() or makeRenderLines() to get the text width, you probably want to add a very little number to that width (e.g. 0.01)
		// due to floating point rounding errors.
		// @see getTextWidth()
		// @see makeRenderLines()
		float getTextHeight(chstr text, float maxWidth);
		// @note If you used getTextWidth() or makeRenderLines() to get the text width, you probably want to add a very little number to that width (e.g. 0.01)
		// due to floating point rounding errors.
		// @see getTextWidth()
		// @see makeRenderLines()
		float getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth);
		// @note If you used getTextWidth() or makeRenderLines() to get the text width, you probably want to add a very little number to that width (e.g. 0.01)
		// due to floating point rounding errors.
		// @see getTextWidth()
		// @see makeRenderLines()
		float getTextHeightUnformatted(chstr text, float maxWidth);

		void clearCache();

		DEPRECATED_ATTRIBUTE void addColor(chstr key, chstr value);
		DEPRECATED_ATTRIBUTE bool getGlobalOffsets() { return this->isGlobalOffsets(); }
		DEPRECATED_ATTRIBUTE bool getUseLegacyLineBreakParsing() { return this->isUseLegacyLineBreakParsing(); }
		DEPRECATED_ATTRIBUTE bool getUseIdeographWords() { return this->isUseIdeographWords(); }
		DEPRECATED_ATTRIBUTE hstr getDefaultFont() const { return this->getDefaultFontName(); }
		DEPRECATED_ATTRIBUTE void setDefaultFont(chstr value) { this->setDefaultFontName(value); }

	protected:
		hmap<hstr, Font*> fonts;
		Font* defaultFont;
		gvec2 shadowOffset;
		april::Color shadowColor;
		float borderThickness;
		float strikeThroughThickness;
		float underlineThickness;
		april::Color borderColor;
		bool globalOffsets;
		bool useLegacyLineBreakParsing;
		bool useIdeographWords;
		Horizontal justifiedDefault;
		Cache<CacheEntryText>* cacheText;
		Cache<CacheEntryText>* cacheTextUnformatted;
		Cache<CacheEntryLines>* cacheLines;
		Cache<CacheEntryLines>* cacheLinesUnformatted;

		void _updateCache();

		void _initializeFormatTags(const harray<FormatTag>& tags);
		void _initializeLineProcessing(const harray<RenderLine>& lines = harray<RenderLine>());
		void _initializeRenderSequences();
		void _checkFormatTags(chstr text, int index);
		void _processFormatTags(chstr text, int index);
		void _checkSequenceSwitch();
		void _updateLiningSequenceSwitch(bool force = false);
		bool _checkTextures();
		harray<FormatTag> _makeDefaultTags(const april::Color& color, chstr fontName, hstr& text);
		harray<FormatTag> _makeDefaultTagsUnformatted(const april::Color& color, chstr fontName);

		void _drawRenderText(RenderText& renderText, const april::Color& color);
		void _drawRenderSequence(RenderSequence& sequence, const april::Color& color);
		void _drawRenderLiningSequence(RenderLiningSequence& sequence, const april::Color& color);

	private:
		harray<FormatTag> _tags;
		harray<FormatTag> _stack;
		FormatTag _currentTag;
		FormatTag _nextTag;

		hstr _fontName;
		Font* _font;
		FontIconMap* _iconFont;
		hstr _fontIconName;
		hmap<unsigned int, CharacterDefinition*>& _characters;
		hmap<unsigned int, CharacterDefinition*> _dummyCharacters; // required for some compilers
		hmap<hstr, IconDefinition*>& _icons;
		hmap<hstr, IconDefinition*> _dummyIcons; // required for some compilers
		CharacterDefinition* _character;
		BorderCharacterDefinition* _borderCharacter;
		IconDefinition* _icon;
		BorderIconDefinition* _borderIcon;
		float _height;
		float _lineHeight;
		float _descender;
		float _internalDescender;
		float _strikeThroughOffset;
		float _underlineOffset;
		float _fontScale;
		float _fontBaseScale;
		float _iconFontScale;
		float _iconFontBearingX;
		float _iconFontOffsetY;
		float _textScale;
		float _scale;
		gvec2 _shadowOffset;
		gvec2 _textShadowOffset;
		float _borderThickness;
		float _borderFontThickness;
		float _textBorderThickness;

		harray<RenderSequence> _textSequences;
		RenderSequence _textSequence;
		harray<RenderSequence> _shadowSequences;
		RenderSequence _shadowSequence;
		harray<RenderSequence> _borderSequences;
		RenderSequence _borderSequence;
		RenderRectangle _renderRect;
		harray<RenderLiningSequence> _textLiningSequences;
		RenderLiningSequence _textStrikeThroughSequence;
		RenderLiningSequence _textUnderlineSequence;
		harray<RenderLiningSequence> _shadowLiningSequences;
		RenderLiningSequence _shadowStrikeThroughSequence;
		RenderLiningSequence _shadowUnderlineSequence;
		harray<RenderLiningSequence> _borderLiningSequences;
		RenderLiningSequence _borderStrikeThroughSequence;
		RenderLiningSequence _borderUnderlineSequence;

		april::Color _textColor;
		april::Color _shadowColor;
		april::Color _borderColor;
		april::Color _strikeThroughColor;
		april::Color _underlineColor;
		hstr _hex;
		hstr _parameterString0;
		hstr _parameterString1;
		int _effectMode;
		bool _strikeThroughActive;
		float _strikeThroughThickness;
		float _textStrikeThroughThickness;
		bool _underlineActive;
		float _underlineThickness;
		float _textUnderlineThickness;
		int _alpha;

		harray<RenderLine> _lines;
		RenderLine _line;
		RenderWord _word;

		april::Texture* _texture;
		unsigned int _code;
		hstr _iconName;

		CacheEntryText _cacheEntryText;
		CacheEntryLines _cacheEntryLines;
		CacheEntryLine _cacheEntryLine;

	};
	
	atresExport extern Renderer* renderer;

};

#endif
