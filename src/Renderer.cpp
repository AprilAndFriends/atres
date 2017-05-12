/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <math.h>
#include <stdio.h>

#include <april/april.h>
#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "Cache.h"
#include "Font.h"
#include "FontIconMap.h"

#ifdef _DEBUG
//#define _DEBUG_RENDER_TEXT
#endif

#define IS_IDEOGRAPH(code) \
	( \
		((code) >= 0x3040 && (code) <= 0x309F) ||	/* Hiragana */ \
		((code) >= 0x30A0 && (code) <= 0x30FF) ||	/* Katakana */ \
		((code) >= 0x3400 && (code) <= 0x4DFF) ||	/* CJK Unified Ideographs Extension A */ \
		((code) >= 0x4E00 && (code) <= 0x9FFF) ||	/* CJK Unified Ideographs */ \
		((code) >= 0xF900 && (code) <= 0xFAFF) ||	/* CJK Compatibility Ideographs */ \
		((code) >= 0x20000 && (code) <= 0x2A6DF) ||	/* CJK Unified Ideographs Extension B */ \
		((code) >= 0x2F800 && (code) <= 0x2FA1F)	/* CJK Compatibility Ideographs Supplement */ \
	)

#define IS_PUNCTUATION_CHAR(code) \
	( \
		(code) == 0x2015 ||	/* long dash */ \
		(code) == 0x2025 ||	/* ellipsis char */ \
		(code) == 0x2026 ||	/* ellipsis char */ \
		(code) == 0x3000 ||	/* ideographic space */ \
		(code) == 0x3001 ||	/* ideographic comma */ \
		(code) == 0x3002 ||	/* ideographic full stop/period */ \
		(code) == 0x3009 ||	/* ideographic closing angle bracket */ \
		(code) == 0x300B ||	/* ideographic closing double angle bracket */ \
		(code) == 0x300D ||	/* ideographic closing quotation mark */ \
		(code) == 0x300F ||	/* ideographic closing double quotation mark */ \
		(code) == 0x3011 ||	/* ideographic closing weird bracket */ \
		(code) == 0x3015 ||	/* ideographic closing tortoise shell bracket */ \
		(code) == 0x3017 ||	/* ideographic closing white weird bracket */ \
		(code) == 0x3019 ||	/* ideographic closing white tortoise shell bracket */ \
		(code) == 0x301B ||	/* ideographic closing double bracket */ \
		(code) == 0x301C ||	/* ideographic wave-dash */ \
		(code) == 0x30FB ||	/* Japanese middle dot */ \
		(code) == 0x30FC ||	/* Japanese dash char */ \
		(code) == 0x4E00 ||	/* fullwidth dash char */ \
		(code) == 0xFF01 ||	/* fullwidth exclamation mark */ \
		(code) == 0xFF09 ||	/* fullwidth closing parenthesis */ \
		(code) == 0xFF0C ||	/* fullwidth comma */ \
		(code) == 0xFF1A ||	/* fullwidth colon */ \
		(code) == 0xFF1E ||	/* fullwidth greater-than sign */ \
		(code) == 0xFF1F ||	/* fullwidth question mark */ \
		(code) == 0xFF3D ||	/* fullwidth closing bracket */ \
		(code) == 0xFF5D ||	/* fullwidth closing brace */ \
		(code) == 0xFF60 ||	/* fullwidth double closing parenthesis */ \
		(code) == 0xFF63	/* fullwidth closing quotation mark */ \
	)

#define UNICODE_CHAR_SPACE 0x20
#define UNICODE_CHAR_ZERO_WIDTH_SPACE 0x200B
#define UNICODE_CHAR_NEWLINE 0x0A

#define EFFECT_MODE_NORMAL 0
#define EFFECT_MODE_SHADOW 1
#define EFFECT_MODE_BORDER 2

#define CHECK_RECT_SIZE 100000.0f // because of the 7-digit precision in floats

namespace atres
{
	static hstr _iconPlaceholder = hstr::fromUnicode((unsigned int)0xA0);
	static float sqrt05 = hsqrt(0.5f);

	Renderer* renderer = NULL;

	Renderer::Renderer() : _characters(_dummyCharacters), _dummyCharacters(hmap<unsigned int, CharacterDefinition*>()),
		_icons(_dummyIcons), _dummyIcons(hmap<hstr, IconDefinition*>())
	{
		// init
		this->shadowOffset.set(1.0f, 1.0f);
		this->shadowColor = april::Color::Black;
		this->borderThickness = 1.0f;
		this->strikeThroughThickness = 1.0f;
		this->underlineThickness = 1.0f;
		this->borderColor = april::Color::Black;
		this->globalOffsets = false;
		this->useLegacyLineBreakParsing = false;
		this->useIdeographWords = false;
		this->justifiedDefault = Horizontal::Justified;
		this->defaultFont = NULL;
		// misc init
		this->_font = NULL;
		this->_iconFont = NULL;
		this->_texture = NULL;
		this->_height = 0.0f;
		this->_lineHeight = 0.0f;
		this->_descender = 0.0f;
		this->_internalDescender = 0.0f;
		this->_strikeThroughOffset = 0.0f;
		this->_underlineOffset = 0.0f;
		this->_fontScale = 1.0f;
		this->_fontBaseScale = 1.0f;
		this->_iconFontScale = 1.0f;
		this->_iconFontBearingX = 0.0f;
		this->_iconFontOffsetY = 0.0f;
		this->_textScale = 1.0f;
		this->_scale = 1.0f;
		this->_shadowOffset.set(1.0f, 1.0f);
		this->_textShadowOffset.set(1.0f, 1.0f);
		this->_borderThickness = 1.0f;
		this->_borderFontThickness = 1.0f;
		this->_textBorderThickness = 1.0f;
		this->_effectMode = 0;
		this->_strikeThroughActive = false;
		this->_strikeThroughThickness = 1.0f;
		this->_textStrikeThroughThickness = 1.0f;
		this->_underlineActive = false;
		this->_underlineThickness = 1.0f;
		this->_textUnderlineThickness = 1.0f;
		this->_alpha = -1;
		// cache
		this->cacheText = new Cache<CacheEntryText>();
		this->cacheTextUnformatted = new Cache<CacheEntryText>();
		this->cacheLines = new Cache<CacheEntryLines>();
		this->cacheLinesUnformatted = new Cache<CacheEntryLines>();
	}

	Renderer::~Renderer()
	{
		this->destroyAllFonts();
		delete this->cacheText;
		delete this->cacheTextUnformatted;
		delete this->cacheLines;
		delete this->cacheLinesUnformatted;
	}

	void Renderer::setShadowOffset(cgvec2 value)
	{
		if (this->shadowOffset != value)
		{
			this->shadowOffset = value;
			this->clearCache();
		}
	}
	
	void Renderer::setShadowColor(const april::Color& value)
	{
		if (this->shadowColor != value)
		{
			this->shadowColor = value;
			this->clearCache();
		}
	}
	
	void Renderer::setBorderThickness(float value)
	{
		if (this->borderThickness != value)
		{
			this->borderThickness = value;
			this->clearCache();
		}
	}
	
	void Renderer::setBorderColor(const april::Color& value)
	{
		if (this->borderColor != value)
		{
			this->borderColor = value;
			this->clearCache();
		}
	}

	void Renderer::setStrikeThroughThickness(float value)
	{
		if (this->strikeThroughThickness != value)
		{
			this->strikeThroughThickness = value;
			this->clearCache();
		}
	}

	void Renderer::setUnderlineThickness(float value)
	{
		if (this->underlineThickness != value)
		{
			this->underlineThickness = value;
			this->clearCache();
		}
	}

	void Renderer::setUseLegacyLineBreakParsing(bool value)
	{
		if (this->useLegacyLineBreakParsing != value)
		{
			this->useLegacyLineBreakParsing = value;
			this->clearCache();
		}
	}

	void Renderer::setUseIdeographWords(bool value)
	{
		if (this->useIdeographWords != value)
		{
			this->useIdeographWords = value;
			this->clearCache();
		}
	}

	void Renderer::setJustifiedDefault(Horizontal value)
	{
		if (this->justifiedDefault != value)
		{
			this->justifiedDefault = value;
			this->clearCache();
		}
	}

	hstr Renderer::getDefaultFontName() const
	{
		return (this->defaultFont != NULL ? this->defaultFont->getName() : "");
	}

	void Renderer::setDefaultFontName(chstr name)
	{
		if (name == "")
		{
			this->defaultFont = NULL;
			this->clearCache();
			return;
		}
		if (!this->fonts.hasKey(name))
		{
			throw ResourceNotExistsException("Font", name, "atres");
		}
		Font* newFont = this->fonts[name];
		if (this->defaultFont != newFont)
		{
			this->defaultFont = newFont;
			this->clearCache();
		}
	}
	
	void Renderer::setCacheSize(int value)
	{
		this->cacheText->setMaxSize(value);
		this->cacheTextUnformatted->setMaxSize(value);
		this->cacheLines->setMaxSize(value);
		this->cacheLinesUnformatted->setMaxSize(value);
	}

	bool Renderer::hasFont(chstr name) const
	{
		return (name == "" && this->defaultFont != NULL || this->fonts.hasKey(name));
	}
	
	void Renderer::registerFont(Font* font, bool allowDefault)
	{
		font->load();
		hstr name = font->getName();
		hlog::write(logTag, "Registering font: " + name);
		if (this->fonts.hasKey(name))
		{
			throw ResourceAlreadyExistsException("font", name, "atres");
		}
		this->clearCache(); // there may be old cached definitions, they must be removed
		this->fonts[name] = font;
		if (this->defaultFont == NULL && allowDefault)
		{
			this->defaultFont = font;
		}
	}
	
	void Renderer::unregisterFont(Font* font)
	{
		hlog::write(logTag, "Unregistering font: " + font->getName());
		if (!this->fonts.hasValue(font))
		{
			throw ResourceNotExistsException("font", font->getName(), "atres");
		}
		this->clearCache(); // there may be old cached definitions, they must be removed
		harray<hstr> keys = this->fonts.keys();
		foreach (hstr, it, keys) // removing aliases
		{
			if (this->fonts[*it] == font)
			{
				this->fonts.removeKey(*it);
			}
		}
		if (this->defaultFont == font)
		{
			this->defaultFont = (this->fonts.size() > 0 ? this->fonts.values().first() : NULL);
		}
	}

	void Renderer::registerFontAlias(chstr name, chstr alias)
	{
		if (this->fonts.hasKey(alias))
		{
			throw ResourceAlreadyExistsException("font", alias, "atres");
		}
		Font* font = this->getFont(name);
		if (font != NULL)
		{
			hlog::writef(logTag, "Registering font alias '%s' for '%s'.", alias.cStr(), font->getName().cStr());
			this->fonts[alias] = font;
		}
		else
		{
			hlog::errorf(logTag, "Could not register alias '%s' for font '%s'. The font does not exist.", alias.cStr(), name.cStr());
		}
	}
	
	void Renderer::destroyAllFonts()
	{
		this->defaultFont = NULL;
		this->clearCache();
		harray<Font*> fonts = this->fonts.values().removedDuplicates();
		foreach (Font*, it, fonts)
		{
			delete (*it);
		}
		this->fonts.clear();
	}
	
	void Renderer::destroyFont(Font* font)
	{
		if (font == NULL)
		{
			throw Exception("Unable to destroy font, NULL argument passed to atres::Renderer::destroyFont()");
		}
		this->unregisterFont(font);
		delete font;
	}

	Font* Renderer::getFont(chstr name)
	{
		if (name == "" && this->defaultFont != NULL)
		{
			this->defaultFont->setScale(1.0f);
			return this->defaultFont;
		}
		Font* font = NULL;
		if (this->fonts.hasKey(name))
		{
			font = this->fonts[name];
			font->setScale(1.0f);
			return font;
		}
		int position = (int)name.indexOf(":");
		if (position >= 0)
		{
			font = this->getFont(name(0, position));
			if (font != NULL)
			{
				++position;
				font->setScale((float)(name(position, name.size() - position)));
			}
		}
		return font;
	}
	
	void Renderer::addColor(chstr key, chstr value)
	{
		april::addSymbolicColor(key, value);
	}
	
	void Renderer::_updateCache()
	{
		this->cacheText->update();
		this->cacheTextUnformatted->update();
		this->cacheLines->update();
		this->cacheLinesUnformatted->update();
	}
	
	void Renderer::clearCache()
	{
		if (this->cacheText->size() > 0)
		{
			hlog::writef(logTag, "Clearing %d text cache entries...", this->cacheText->size());
			this->cacheText->clear();
		}
		if (this->cacheTextUnformatted->size() > 0)
		{
			hlog::writef(logTag, "Clearing %d unformatted text cache entries...", this->cacheTextUnformatted->size());
			this->cacheTextUnformatted->clear();
		}
		if (this->cacheLines->size() > 0)
		{
			hlog::writef(logTag, "Clearing %d lines cache entries...", this->cacheLines->size());
			this->cacheLines->clear();
		}
		if (this->cacheLinesUnformatted->size() > 0)
		{
			hlog::writef(logTag, "Clearing %d unformatted lines cache entries...", this->cacheLinesUnformatted->size());
			this->cacheLinesUnformatted->clear();
		}
	}
	
	void Renderer::analyzeText(chstr fontName, chstr text)
	{
		// makes sure dynamically allocated characters are loaded
		std::ustring chars = text.uStr();
		Font* font = this->getFont(fontName);
		if (font != NULL)
		{
			for_itert (unsigned int, i, 0, chars.size())
			{
				font->hasCharacter(chars[i]);
			}
		}
	}

	hstr Renderer::analyzeFormatting(chstr text, harray<FormatTag>& tags)
	{
		std::ustring uText = text.uStr();
		const unsigned int* str = uText.c_str();
		int start = 0;
		int end = 0;
		bool ignoreFormatting = false;
		harray<char> stack;
		FormatTag tag;
		harray<FormatTag> foundTags;
		// parse formatting in Unicode
		while (true)
		{
			start = (int)uText.find_first_of('[', start);
			if (start < 0)
			{
				break;
			}
			end = (int)uText.find_first_of(']', start);
			if (end < 0)
			{
				break;
			}
			++end;
			tag.data = "";
			tag.start = text.utf8SubString(0, start).size();
			tag.count = text.utf8SubString(start, end - start).size();
			if (ignoreFormatting)
			{
				if (str[start + 1] != '/' || str[start + 2] != '-')
				{
					start = end;
					continue;
				}
				ignoreFormatting = false;
				stack.removeLast();
				if (str[start + 2] != 'i')
				{
					tag.type = FormatTag::Type::Close;
				}
				else
				{
					tag.type = FormatTag::Type::CloseConsume;
				}
			}
			else if (end - start == 2) // empty command
			{
				tag.type = FormatTag::Type::Escape;
			}
			else if (str[start + 1] == '/') // closing command
			{
				if (stack.size() == 0 || (unsigned int)stack.last() != str[start + 2]) // interleaving, ignore the tag
				{
					start = end;
					hlog::warnf(logTag, "Closing tag that was not opened ('[/%c]' in '%s')!", str[start + 2], str);
					continue;
				}
				stack.removeLast();
				if (str[start + 2] != 'i')
				{
					tag.type = FormatTag::Type::Close;
				}
				else
				{
					tag.type = FormatTag::Type::CloseConsume;
				}
			}
			else // opening new tag
			{
				switch (str[start + 1])
				{
				case 'f':
					tag.type = FormatTag::Type::Font;
					break;
				case 'i':
					tag.type = FormatTag::Type::Icon;
					break;
				case 'c':
					tag.type = FormatTag::Type::Color;
					break;
				case 'x':
					tag.type = FormatTag::Type::Scale;
					break;
				case 'n':
					tag.type = FormatTag::Type::NoEffect;
					break;
				case 's':
					tag.type = FormatTag::Type::Shadow;
					break;
				case 'b':
					tag.type = FormatTag::Type::Border;
					break;
				case 't':
					tag.type = FormatTag::Type::StrikeThrough;
					break;
				case 'u':
					tag.type = FormatTag::Type::Underline;
					break;
				case '-': // ignore formattting from here on
					tag.type = FormatTag::Type::IgnoreFormatting;
					ignoreFormatting = true;
					break;
				default: // command not supported, regard as normal text
					start = end;
					continue;
				}
				stack += str[start + 1];
				if (end - start > 4)
				{
					tag.data = text.utf8SubString(start + 3, end - start - 4);
				}
			}
			foundTags += tag;
			start = end;
		}
		// add closing tags where missing
		hstr result;
		int index = 0;
		int count = 0;
		bool hasPreviousTag = false;
		foreach (FormatTag, it, foundTags)
		{
			if ((*it).type != FormatTag::Type::CloseConsume)
			{
				result += text(index, (*it).start - index);
			}
			else if (hasPreviousTag)
			{
				tags.last().consumedData = text(index, (*it).start - index);
				count += (*it).start - index;
				hasPreviousTag = false;
			}
			index = (*it).start + (*it).count;
			if ((*it).type == FormatTag::Type::Escape)
			{
				--count;
				result += '[';
			}
			else
			{
				(*it).start -= count;
				if ((*it).type == FormatTag::Type::Icon)
				{
					hasPreviousTag = true;
					// using a non-breaking space to indicate an icon being rendered here
					count -= _iconPlaceholder.size();
					result += _iconPlaceholder;
				}
				tags += (*it);
			}
			count += (*it).count;
		}
		count = text.size() - index;
		if (count > 0)
		{
			result += text(index, count);
		}
		return result;
	}

	harray<RenderLine> Renderer::removeOutOfBoundLines(const harray<RenderLine>& lines, cgrect rect)
	{
		harray<RenderLine> result;
		foreachc (RenderLine, it, lines)
		{
			// zero-length rectangles should be included
			if ((*it).rect.w == 0.0f || (*it).rect.h == 0.0f || (*it).rect.intersects(rect))
			{
				result += (*it);
			}
		}
		return result;
	}
	
	void Renderer::verticalCorrection(harray<RenderLine>& lines, cgrect rect, Vertical vertical, float y, float lineHeight, float descender, float internalDescender)
	{
		harray<RenderLine> result;
		int lineCount = lines.size();
		if (lines.last().terminated)
		{
			++lineCount;
		}
		// vertical correction
		if (vertical == Vertical::Center)
		{
			y += (lineCount * lineHeight - rect.h + descender) * 0.5f;
		}
		else if (vertical == Vertical::Bottom)
		{
			y += lineCount * lineHeight - rect.h + internalDescender;
		}
		// remove lines that cannot be seen anyway
		foreach (RenderLine, it, lines)
		{
			(*it).rect.y -= y;
			foreach (RenderWord, it2, (*it).words)
			{
				(*it2).rect.y -= y;
			}
			result += (*it);
		}
	}
	
	void Renderer::horizontalCorrection(harray<RenderLine>& lines, cgrect rect, Horizontal horizontal, float x)
	{
		// horizontal correction not necessary when left aligned
		if (horizontal.isLeft() || horizontal == Horizontal::Justified && this->justifiedDefault != Horizontal::Justified)
		{
			foreach (RenderLine, it, lines)
			{
				(*it).rect.x -= x;
				foreach (RenderWord, it2, (*it).words)
				{
					(*it2).rect.x -= x;
				}
			}
			return;
		}
		float ox = 0.0f;
		if (horizontal != Horizontal::Justified || this->justifiedDefault != Horizontal::Justified)
		{
			if (horizontal == Horizontal::Justified)
			{
				horizontal = this->justifiedDefault;
			}
			// horizontal correction
			foreach (RenderLine, it, lines)
			{
				if (horizontal.isCenter())
				{
					ox = -x + (rect.w - (*it).rect.w) * 0.5f;
				}
				else if (horizontal.isRight())
				{
					ox = -x + rect.w - (*it).rect.w;
				}
				(*it).rect.x += ox;
				foreach (RenderWord, it2, (*it).words)
				{
					(*it2).rect.x += ox;
				}
			}
		}
		else // justified correction
		{
			float width = 0.0f;
			float widthPerSpace = 0.0f;
			float lineRight = 0.0f;
			harray<RenderWord> words;
			for_iter (i, 0, lines.size() - 1)
			{
				if (!lines[i].terminated) // if line was not actually terminated with a \n
				{
					if (lines[i].spaces > 0)
					{
						width = 0.0f;
						foreach (RenderWord, it2, lines[i].words)
						{
							if ((*it2).spaces == 0)
							{
								width += (*it2).advanceX;
							}
						}
						foreach_r (RenderWord, it2, lines[i].words)
						{
							if ((*it2).spaces == 0)
							{
								width += hmax((*it2).rect.w - (*it2).advanceX, 0.0f);
								break;
							}
						}
						widthPerSpace = (rect.w - width) / lines[i].spaces;
						width = 0.0f;
						lineRight = lines[i].rect.right();
						words.clear();
						foreach (RenderWord, it, lines[i].words)
						{
							if ((*it).spaces == 0)
							{
								(*it).rect.x += hroundf(width);
								words += (*it);
								lineRight = (*it).rect.right();
							}
							else
							{
								width += (*it).spaces * (widthPerSpace - (*it).rect.w);
							}
						}
						lines[i].words = words;
						lines[i].rect.w = lineRight - lines[i].rect.x;
					}
					else // no spaces, just force a centered horizontal alignment
					{
						ox = -x + (rect.w - lines[i].rect.w) * 0.5f;
						lines[i].rect.x += ox;
						foreach (RenderWord, it, lines[i].words)
						{
							(*it).rect.x += ox;
						}
					}
				}
			}
		}
	}

	void Renderer::_initializeFormatTags(const harray<FormatTag>& tags)
	{
		this->_tags = tags;
		this->_stack.clear();
		this->_currentTag = FormatTag();
		this->_nextTag = this->_tags.first();
		this->_fontName = "";
		this->_font = NULL;
		this->_iconFont = NULL;
		this->_texture = NULL;
		this->_characters = this->_dummyCharacters;
		this->_icons = this->_dummyIcons;
		this->_height = 0.0f;
		this->_lineHeight = 0.0f;
		this->_descender = 0.0f;
		this->_internalDescender = 0.0f;
		this->_strikeThroughOffset = 0.0f;
		this->_underlineOffset = 0.0f;
		this->_fontScale = 1.0f;
		this->_fontBaseScale = 1.0f;
		this->_iconFontScale = 1.0f;
		this->_iconFontBearingX = 0.0f;
		this->_iconFontOffsetY = 0.0f;
		this->_textScale = 1.0f;
		this->_scale = 1.0f;
		this->_shadowOffset.set(1.0f, 1.0f);
		this->_textShadowOffset.set(1.0f, 1.0f);
		this->_borderThickness = 1.0f;
		this->_borderFontThickness = 1.0f;
		this->_textBorderThickness = 1.0f;
		this->_strikeThroughActive = false;
		this->_strikeThroughThickness = 1.0f;
		this->_textStrikeThroughThickness = 1.0f;
		this->_underlineActive = false;
		this->_underlineThickness = 1.0f;
		this->_textUnderlineThickness = 1.0f;
	}

	void Renderer::_initializeRenderSequences()
	{
		this->_textSequences.clear();
		this->_textSequence = RenderSequence();
		this->_shadowSequences.clear();
		this->_shadowSequence = RenderSequence();
		this->_shadowSequence.color = this->shadowColor;
		this->_borderSequences.clear();
		this->_borderSequence = RenderSequence();
		this->_borderSequence.color = this->borderColor;
		this->_renderRect = RenderRectangle();
		this->_textLiningSequences.clear();
		this->_textStrikeThroughSequence = RenderLiningSequence();
		this->_textUnderlineSequence = RenderLiningSequence();
		this->_shadowLiningSequences.clear();
		this->_shadowStrikeThroughSequence = RenderLiningSequence();
		this->_shadowStrikeThroughSequence.color = this->shadowColor;
		this->_shadowUnderlineSequence = RenderLiningSequence();
		this->_shadowUnderlineSequence.color = this->shadowColor;
		this->_borderLiningSequences.clear();
		this->_borderStrikeThroughSequence = RenderLiningSequence();
		this->_borderStrikeThroughSequence.color = this->borderColor;
		this->_borderUnderlineSequence = RenderLiningSequence();
		this->_borderUnderlineSequence.color = this->borderColor;
		this->_textColor = april::Color::White;
		this->_shadowColor = this->shadowColor;
		this->_borderColor = this->borderColor;
		this->_strikeThroughColor = april::Color::White;
		this->_underlineColor = april::Color::White;
		this->_hex = "";
		this->_effectMode = 0;
		this->_strikeThroughActive = false;
		this->_strikeThroughThickness = 1.0f;
		this->_textStrikeThroughThickness = 1.0f;
		this->_underlineActive = false;
		this->_underlineThickness = 1.0f;
		this->_textUnderlineThickness = 1.0f;
		this->_alpha = -1;
	}

	void Renderer::_initializeLineProcessing(const harray<RenderLine>& lines)
	{
		this->_lines = lines;
		this->_line = RenderLine();
		this->_word = RenderWord();
	}

	void Renderer::_checkFormatTags(chstr text, int index)
	{
		while (this->_tags.size() > 0 && index >= this->_nextTag.start)
		{
			if (this->_nextTag.type == FormatTag::Type::Close || this->_nextTag.type == FormatTag::Type::CloseConsume)
			{
				this->_currentTag = this->_stack.removeLast();
				if (this->_currentTag.type == FormatTag::Type::Font)
				{
					this->_fontName = this->_currentTag.data;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_icons = this->_font->getIcons();
					this->_fontScale = this->_font->getScale();
					this->_fontBaseScale = this->_font->getBaseScale();
				}
				else if (this->_currentTag.type == FormatTag::Type::Icon)
				{
					this->_fontName = this->_currentTag.data;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_icons = this->_font->getIcons();
					this->_fontScale = this->_font->getScale();
					this->_fontBaseScale = this->_font->getBaseScale();
				}
				else if (this->_currentTag.type == FormatTag::Type::Scale)
				{
					this->_textScale = this->_currentTag.data;
				}
			}
			else if (this->_nextTag.type == FormatTag::Type::Font)
			{
				this->_currentTag.type = FormatTag::Type::Font;
				this->_currentTag.data = this->_fontName;
				this->_stack += this->_currentTag;
				if (this->_font == NULL) // if there is no previous font, some special values have to be obtained as well
				{
					this->_font = this->getFont(this->_nextTag.data);
					if (this->_font != NULL)
					{
						this->_height = this->_font->getHeight();
						this->_lineHeight = this->_font->getLineHeight();
						this->_descender = this->_font->getDescender();
						this->_internalDescender = this->_font->getInternalDescender();
						this->_strikeThroughOffset = this->_font->getStrikeThroughOffset();
						this->_underlineOffset = this->_font->getUnderlineOffset();
					}
				}
				else
				{
					this->_font = this->getFont(this->_nextTag.data);
				}
				if (this->_font != NULL)
				{
					this->_fontName = this->_nextTag.data;
					this->_characters = this->_font->getCharacters();
					this->_fontScale = this->_font->getScale();
					this->_fontBaseScale = this->_font->getBaseScale();
				}
				else
				{
					hlog::warnf(logTag, "Font '%s' does not exist!", this->_nextTag.data.cStr());
				}
			}
			else if (this->_nextTag.type == FormatTag::Type::Icon)
			{
				this->_currentTag.type = FormatTag::Type::Icon;
				this->_currentTag.data = this->_fontName;
				this->_currentTag.consumedData = this->_fontIconName;
				this->_stack += this->_currentTag;
				this->_iconFont = dynamic_cast<FontIconMap*>(this->getFont(this->_nextTag.data));
				if (this->_iconFont != NULL)
				{
					if (this->_font == NULL) // if there is no previous font, some special values have to be obtained as well
					{
						this->_height = this->_iconFont->getHeight();
						this->_lineHeight = this->_iconFont->getLineHeight();
						this->_descender = this->_iconFont->getDescender();
						this->_internalDescender = this->_iconFont->getInternalDescender();
						this->_strikeThroughOffset = this->_iconFont->getStrikeThroughOffset();
						this->_underlineOffset = this->_iconFont->getUnderlineOffset();
					}
					this->_fontName = this->_nextTag.data;
					this->_fontIconName = this->_nextTag.consumedData;
					this->_iconFont->hasIcon(this->_fontIconName);
					this->_icons = this->_iconFont->getIcons();
					this->_iconFontScale = this->_iconFont->getScale() * this->_fontScale / this->_fontBaseScale;
					this->_iconFontBearingX = this->_iconFont->getBearingX();
					this->_iconFontOffsetY = this->_iconFont->getOffsetY();
				}
				else
				{
					hlog::warnf(logTag, "Font '%s' does not exist!", this->_nextTag.data.cStr());
				}
			}
			else if (this->_nextTag.type == FormatTag::Type::Color)
			{
				this->_currentTag.type = FormatTag::Type::Color;
				this->_stack += this->_currentTag;
			}
			else if (this->_nextTag.type == FormatTag::Type::Scale)
			{
				this->_currentTag.type = FormatTag::Type::Scale;
				this->_currentTag.data = this->_textScale;
				this->_stack += this->_currentTag;
				this->_textScale = this->_nextTag.data;
			}
			else
			{
				this->_currentTag.type = FormatTag::Type::NoEffect;
				this->_stack += this->_currentTag;
			}
			this->_tags.removeFirst();
			if (this->_tags.size() > 0)
			{
				this->_nextTag = this->_tags.first();
			}
			else
			{
				this->_nextTag.start = text.size() + 1;
			}
		}
	}

	void Renderer::_processFormatTags(chstr text, int index)
	{
		while (this->_tags.size() > 0 && this->_word.start + index >= this->_nextTag.start)
		{
			if (this->_nextTag.type == FormatTag::Type::Close || this->_nextTag.type == FormatTag::Type::CloseConsume)
			{
				this->_currentTag = this->_stack.removeLast();
				if (this->_currentTag.type == FormatTag::Type::Font)
				{
					this->_fontName = this->_currentTag.data;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_icons = this->_font->getIcons();
					this->_fontScale = this->_font->getScale();
					this->_fontBaseScale = this->_font->getBaseScale();
				}
				else if (this->_currentTag.type == FormatTag::Type::Icon)
				{
					this->_fontName = this->_currentTag.data;
					this->_fontIconName = this->_currentTag.consumedData;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_icons = this->_font->getIcons();
					this->_fontScale = this->_font->getScale();
					this->_fontBaseScale = this->_font->getBaseScale();
					this->_iconFont = NULL;
				}
				else if (this->_currentTag.type == FormatTag::Type::Color)
				{
					if (!april::findSymbolicColor(this->_currentTag.data.lowered(), this->_hex))
					{
						this->_hex = this->_currentTag.data;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						if (this->_textColor == this->_strikeThroughColor)
						{
							this->_strikeThroughColor.set(this->_hex);
						}
						if (this->_textColor == this->_underlineColor)
						{
							this->_underlineColor.set(this->_hex);
						}
						this->_textColor.set(this->_hex);
					}
				}
				else if (this->_currentTag.type == FormatTag::Type::Scale)
				{
					this->_textScale = this->_currentTag.data;
				}
				else if (this->_currentTag.type == FormatTag::Type::NoEffect)
				{
					this->_effectMode = EFFECT_MODE_NORMAL;
				}
				else if (this->_currentTag.type == FormatTag::Type::Shadow)
				{
					this->_effectMode = EFFECT_MODE_SHADOW;
					if (this->_currentTag.data.count(',') == 2)
					{
						this->_currentTag.data.split(',', this->_parameterString0, this->_parameterString1);
						this->_textShadowOffset = april::hstrToGvec2(this->_parameterString1);
					}
					else
					{
						this->_parameterString0 = this->_currentTag.data;
					}
					if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
					{
						this->_hex = this->_parameterString0;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						this->_shadowColor.set(this->_hex);
					}
				}
				else if (this->_currentTag.type == FormatTag::Type::Border)
				{
					this->_effectMode = EFFECT_MODE_BORDER;
					if (this->_currentTag.data.count(',') == 1)
					{
						this->_currentTag.data.split(',', this->_parameterString0, this->_parameterString1);
						this->_textBorderThickness = (float)this->_parameterString1;
					}
					else
					{
						this->_parameterString0 = this->_currentTag.data;
					}
					if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
					{
						this->_hex = this->_parameterString0;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						this->_borderColor.set(this->_hex);
					}
				}
				else if (this->_currentTag.type == FormatTag::Type::StrikeThrough)
				{
					this->_strikeThroughActive = false;
					if (this->_currentTag.data.count(',') == 1)
					{
						this->_currentTag.data.split(',', this->_parameterString0, this->_parameterString1);
						this->_textStrikeThroughThickness = (float)this->_parameterString1;
					}
					else
					{
						this->_parameterString0 = this->_currentTag.data;
					}
					if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
					{
						this->_hex = this->_parameterString0;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						this->_strikeThroughColor.set(this->_hex);
					}

				}
				else if (this->_currentTag.type == FormatTag::Type::Underline)
				{
					this->_underlineActive = false;
					if (this->_currentTag.data.count(',') == 1)
					{
						this->_currentTag.data.split(',', this->_parameterString0, this->_parameterString1);
						this->_textUnderlineThickness = (float)this->_parameterString1;
					}
					else
					{
						this->_parameterString0 = this->_currentTag.data;
					}
					if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
					{
						this->_hex = this->_parameterString0;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						this->_underlineColor.set(this->_hex);
					}
				}
			}
			else
			{
				if (this->_nextTag.type == FormatTag::Type::Font)
				{
					this->_currentTag.type = this->_nextTag.type;
					this->_currentTag.data = this->_fontName;
					this->_stack += this->_currentTag;
					if (this->_font == NULL) // if there is no previous font, some special values have to be obtained as well
					{
						this->_font = this->getFont(this->_nextTag.data);
						if (this->_font != NULL)
						{
							this->_height = this->_font->getHeight();
							this->_lineHeight = this->_font->getLineHeight();
							this->_descender = this->_font->getDescender();
							this->_internalDescender = this->_font->getInternalDescender();
							this->_strikeThroughOffset = this->_font->getStrikeThroughOffset();
							this->_underlineOffset = this->_font->getUnderlineOffset();
						}
					}
					else
					{
						this->_font = this->getFont(this->_nextTag.data);
					}
					if (this->_font != NULL)
					{
						this->_fontName = this->_nextTag.data;
						this->_characters = this->_font->getCharacters();
						this->_fontScale = this->_font->getScale();
						this->_fontBaseScale = this->_font->getBaseScale();
					}
					else
					{
						hlog::warnf(logTag, "Font '%s' does not exist!", this->_nextTag.data.cStr());
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::Icon)
				{
					this->_currentTag.type = FormatTag::Type::Icon;
					this->_currentTag.data = this->_fontName;
					this->_currentTag.consumedData = this->_fontIconName;
					this->_stack += this->_currentTag;
					this->_iconFont = dynamic_cast<FontIconMap*>(this->getFont(this->_nextTag.data));
					if (this->_iconFont != NULL)
					{
						if (this->_font == NULL) // if there is no previous font, some special values have to be obtained as well
						{
							this->_height = this->_iconFont->getHeight();
							this->_lineHeight = this->_iconFont->getLineHeight();
							this->_descender = this->_iconFont->getDescender();
							this->_internalDescender = this->_iconFont->getInternalDescender();
							this->_strikeThroughOffset = this->_font->getStrikeThroughOffset();
							this->_underlineOffset = this->_font->getUnderlineOffset();
						}
						this->_fontName = this->_nextTag.data;
						this->_fontIconName = this->_nextTag.consumedData;
						this->_iconFont->hasIcon(this->_fontIconName);
						this->_icons = this->_iconFont->getIcons();
						this->_iconFontScale = this->_iconFont->getScale() * this->_fontScale / this->_fontBaseScale;
						this->_iconFontBearingX = this->_iconFont->getBearingX();
						this->_iconFontOffsetY = this->_iconFont->getOffsetY();
					}
					else
					{
						hlog::warnf(logTag, "Font '%s' does not exist!", this->_nextTag.data.cStr());
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::Color)
				{
					this->_currentTag.type = FormatTag::Type::Color;
					this->_currentTag.data = this->_textColor.hex();
					this->_stack += this->_currentTag;
					if (!april::findSymbolicColor(this->_nextTag.data.lowered(), this->_hex))
					{
						this->_hex = this->_nextTag.data;
					}
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
					{
						if (this->_textColor == this->_strikeThroughColor)
						{
							this->_strikeThroughColor.set(this->_hex);
						}
						if (this->_textColor == this->_underlineColor)
						{
							this->_underlineColor.set(this->_hex);
						}
						this->_textColor.set(this->_hex);
						this->_alpha == -1 ? this->_alpha = this->_textColor.a : this->_textColor.a = (unsigned char)(this->_alpha * this->_textColor.a_f());
					}
					else
					{
						hlog::warnf(logTag, "Color '%s' does not exist!", this->_hex.cStr());
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::Scale)
				{
					this->_currentTag.type = FormatTag::Type::Scale;
					this->_currentTag.data = this->_textScale;
					this->_stack += this->_currentTag;
					this->_textScale = this->_nextTag.data;
				}
				else if (this->_nextTag.type == FormatTag::Type::NoEffect)
				{
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FormatTag::Type::Border : (this->_effectMode == EFFECT_MODE_SHADOW ? FormatTag::Type::Shadow : FormatTag::Type::NoEffect));
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_NORMAL;
				}
				else if (this->_nextTag.type == FormatTag::Type::Shadow)
				{
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FormatTag::Type::Border : (this->_effectMode == EFFECT_MODE_SHADOW ? FormatTag::Type::Shadow : FormatTag::Type::NoEffect));
					this->_currentTag.data = this->_shadowColor.hex() + "," + april::gvec2ToHstr(this->_textShadowOffset);
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_SHADOW;
					this->_shadowColor = this->shadowColor;
					if (this->_nextTag.data != "")
					{
						this->_parameterString1 = "";
						if (this->_nextTag.data.count(',') == 2)
						{
							this->_nextTag.data.split(',', this->_parameterString0, this->_parameterString1);
							this->_textShadowOffset = april::hstrToGvec2(this->_parameterString1);
						}
						else
						{
							this->_parameterString0 = this->_nextTag.data;
						}
						if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
						{
							this->_hex = this->_parameterString0;
						}
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
						{
							this->_shadowColor.set(this->_hex);
						}
						else if (this->_parameterString1 == "" || this->_hex != "")
						{
							hlog::warnf(logTag, "Color '%s' does not exist!", this->_hex.cStr());
						}
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::Border)
				{
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FormatTag::Type::Border : (this->_effectMode == EFFECT_MODE_SHADOW ? FormatTag::Type::Shadow : FormatTag::Type::NoEffect));
					this->_currentTag.data = this->_borderColor.hex() + "," + hstr(this->_textBorderThickness);
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_BORDER;
					this->_borderColor = this->borderColor;
					if (this->_nextTag.data != "")
					{
						this->_parameterString1 = "";
						if (this->_nextTag.data.count(',') == 1)
						{
							this->_nextTag.data.split(',', this->_parameterString0, this->_parameterString1);
							this->_textBorderThickness = (float)this->_parameterString1;
						}
						else
						{
							this->_parameterString0 = this->_nextTag.data;
						}
						if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
						{
							this->_hex = this->_parameterString0;
						}
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
						{
							this->_borderColor.set(this->_hex);
						}
						else if (this->_parameterString1 == "" || this->_hex != "")
						{
							hlog::warnf(logTag, "Color '%s' does not exist!", this->_hex.cStr());
						}
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::StrikeThrough)
				{
					this->_currentTag.type = FormatTag::Type::StrikeThrough;
					this->_currentTag.data = this->_strikeThroughColor.hex() + "," + hstr(this->_textStrikeThroughThickness);
					this->_stack += this->_currentTag;
					this->_strikeThroughActive = true;
					if (this->_nextTag.data != "")
					{
						this->_parameterString1 = "";
						if (this->_nextTag.data.count(',') == 1)
						{
							this->_nextTag.data.split(',', this->_parameterString0, this->_parameterString1);
							this->_textStrikeThroughThickness = (float)this->_parameterString1;
						}
						else
						{
							this->_parameterString0 = this->_nextTag.data;
						}
						if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
						{
							this->_hex = this->_parameterString0;
						}
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
						{
							this->_strikeThroughColor.set(this->_hex);
						}
						else if (this->_parameterString1 == "" || this->_hex != "")
						{
							hlog::warnf(logTag, "Color '%s' does not exist!", this->_hex.cStr());
						}
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::Underline)
				{
					this->_currentTag.type = FormatTag::Type::Underline;
					this->_currentTag.data = this->_underlineColor.hex() + "," + hstr(this->_textUnderlineThickness);
					this->_stack += this->_currentTag;
					this->_underlineActive = true;
					if (this->_nextTag.data != "")
					{
						this->_parameterString1 = "";
						if (this->_nextTag.data.count(',') == 1)
						{
							this->_nextTag.data.split(',', this->_parameterString0, this->_parameterString1);
							this->_textUnderlineThickness = (float)this->_parameterString1;
						}
						else
						{
							this->_parameterString0 = this->_nextTag.data;
						}
						if (!april::findSymbolicColor(this->_parameterString0.lowered(), this->_hex))
						{
							this->_hex = this->_parameterString0;
						}
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.isHex())
						{
							this->_underlineColor.set(this->_hex);
						}
						else if (this->_parameterString1 == "" || this->_hex != "")
						{
							hlog::warnf(logTag, "Color '%s' does not exist!", this->_hex.cStr());
						}
					}
				}
				else if (this->_nextTag.type == FormatTag::Type::IgnoreFormatting)
				{
					this->_currentTag.type = FormatTag::Type::IgnoreFormatting;
					this->_stack += this->_currentTag;
				}
			}
			this->_tags.removeFirst();
			if (this->_tags.size() > 0)
			{
				this->_nextTag = this->_tags.first();
			}
			else if (this->_lines.size() > 0)
			{
				this->_nextTag.start = this->_line.words.last().start + this->_line.words.last().text.size() + 1;
			}
			else
			{
				this->_nextTag.start = this->_word.start + this->_word.text.size() + 1;
			}
			if (this->_iconFont != NULL)
			{
				this->_texture = this->_iconFont->getTexture(this->_fontIconName);
				this->_checkSequenceSwitch();
			}
			else if (this->_font != NULL)
			{
				this->_texture = this->_font->getTexture(this->_code);
				this->_checkSequenceSwitch();
			}
		}
		if (this->_tags.size() == 0)
		{
			if (this->_lines.size() > 0)
			{
				this->_nextTag.start = this->_line.words.last().start + this->_line.words.last().text.size() + 1;
			}
			else
			{
				this->_nextTag.start = this->_word.start + this->_word.text.size() + 1;
			}
		}
		// this additional check is required in case the texture had to be changed
		if (this->_iconFont != NULL)
		{
			this->_texture = this->_iconFont->getTexture(this->_fontIconName);
			this->_checkSequenceSwitch();
		}
		else if (this->_font != NULL)
		{
			this->_texture = this->_font->getTexture(this->_code);
			this->_checkSequenceSwitch();
		}
	}

	void Renderer::_checkSequenceSwitch()
	{
		if (this->_textSequence.texture != this->_texture || this->_textSequence.color != this->_textColor)
		{
			if (this->_textSequence.vertices.size() > 0)
			{
				this->_textSequences += this->_textSequence;
				this->_textSequence.vertices.clear();
			}
			this->_textSequence.texture = this->_texture;
			this->_textSequence.color = this->_textColor;
		}
		if (this->_shadowSequence.texture != this->_texture || this->_shadowSequence.color != this->_shadowColor)
		{
			if (this->_shadowSequence.vertices.size() > 0)
			{
				this->_shadowSequences += this->_shadowSequence;
				this->_shadowSequence.vertices.clear();
			}
			this->_shadowSequence.texture = this->_texture;
			this->_shadowSequence.color = this->_shadowColor;
		}
		if (this->_borderSequence.texture != this->_texture || this->_borderSequence.color != this->_borderColor)
		{
			if (this->_borderSequence.vertices.size() > 0)
			{
				this->_borderSequences += this->_borderSequence;
				this->_borderSequence.vertices.clear();
			}
			this->_borderSequence.texture = this->_texture;
			this->_borderSequence.color = this->_borderColor;
		}
		if (this->_textStrikeThroughSequence.color != this->_strikeThroughColor)
		{
			if (this->_textStrikeThroughSequence.vertices.size() > 0)
			{
				this->_textLiningSequences += this->_textStrikeThroughSequence;
				this->_textStrikeThroughSequence.vertices.clear();
			}
			this->_textStrikeThroughSequence.color = this->_strikeThroughColor;
		}
		if (this->_textUnderlineSequence.color != this->_underlineColor)
		{
			if (this->_textUnderlineSequence.vertices.size() > 0)
			{
				this->_textLiningSequences += this->_textUnderlineSequence;
				this->_textUnderlineSequence.vertices.clear();
			}
			this->_textUnderlineSequence.color = this->_underlineColor;
		}
		if (this->_shadowStrikeThroughSequence.color != this->_shadowColor)
		{
			if (this->_shadowStrikeThroughSequence.vertices.size() > 0)
			{
				this->_shadowLiningSequences += this->_shadowStrikeThroughSequence;
				this->_shadowStrikeThroughSequence.vertices.clear();
			}
			this->_shadowStrikeThroughSequence.color = this->_shadowColor;
			if (this->_shadowUnderlineSequence.vertices.size() > 0)
			{
				this->_shadowLiningSequences += this->_shadowUnderlineSequence;
				this->_shadowUnderlineSequence.vertices.clear();
			}
			this->_shadowUnderlineSequence.color = this->_shadowColor;
		}
		if (this->_borderStrikeThroughSequence.color != this->_borderColor)
		{
			if (this->_borderStrikeThroughSequence.vertices.size() > 0)
			{
				this->_borderLiningSequences += this->_borderStrikeThroughSequence;
				this->_borderStrikeThroughSequence.vertices.clear();
			}
			this->_borderStrikeThroughSequence.color = this->_borderColor;
			if (this->_borderUnderlineSequence.vertices.size() > 0)
			{
				this->_borderLiningSequences += this->_borderUnderlineSequence;
				this->_borderUnderlineSequence.vertices.clear();
			}
			this->_borderUnderlineSequence.color = this->_borderColor;
		}
	}

	void Renderer::_updateLiningSequenceSwitch(bool force)
	{
		if (!this->_strikeThroughActive || force)
		{
			if (this->_textStrikeThroughSequence.vertices.size() > 0)
			{
				this->_textLiningSequences += this->_textStrikeThroughSequence;
				this->_textStrikeThroughSequence.vertices.clear();
			}
			if (this->_shadowStrikeThroughSequence.vertices.size() > 0)
			{
				this->_shadowLiningSequences += this->_shadowStrikeThroughSequence;
				this->_shadowStrikeThroughSequence.vertices.clear();
			}
			if (this->_borderStrikeThroughSequence.vertices.size() > 0)
			{
				this->_borderLiningSequences += this->_borderStrikeThroughSequence;
				this->_borderStrikeThroughSequence.vertices.clear();
			}
		}
		if (!this->_underlineActive || force)
		{
			if (this->_textUnderlineSequence.vertices.size() > 0)
			{
				this->_textLiningSequences += this->_textUnderlineSequence;
				this->_textUnderlineSequence.vertices.clear();
			}
			if (this->_shadowUnderlineSequence.vertices.size() > 0)
			{
				this->_shadowLiningSequences += this->_shadowUnderlineSequence;
				this->_shadowUnderlineSequence.vertices.clear();
			}
			if (this->_borderUnderlineSequence.vertices.size() > 0)
			{
				this->_borderLiningSequences += this->_borderUnderlineSequence;
				this->_borderUnderlineSequence.vertices.clear();
			}
		}
	}

	harray<RenderWord> Renderer::createRenderWords(cgrect rect, chstr text, const harray<FormatTag>& tags)
	{
		this->_initializeFormatTags(tags);
		hstr initialFontName = this->_tags.first().data; // by convention, the first tag is the font name
		int actualSize = text.indexOf('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
		else if (actualSize < text.size())
		{
			hlog::warnf(logTag, "Text '%s' has \\0 character before the actual end!", text.cStr());
		}
		harray<RenderWord> result;
		RenderWord word;
		unsigned int code = 0;
		unsigned int previousCode = 0;
		harray<hstr> iconNames;
		float ax = 0.0f;
		float aw = 0.0f;
		float charX = 0.0f;
		float addW = 0.0f;
		float bearingX = 0.0f;
		float previousWordWidth = 0.0f;
		float wordWidth = 0.0f;
		float wordBearingX = 0.0f;
		float kerning = 0.0f;
		int start = 0;
		int i = 0;
		int chars = 0;
		int byteSize = 0;
		bool checkingSpaces = true;
		bool icon = false;
		bool tooLong = false;
		hstr iconName;
		harray<float> charXs;
		harray<float> charAdvanceXs;
		harray<float> segmentWidths;
		word.rect.x = rect.x;
		word.rect.y = rect.y;
		word.rect.h = this->_height;
		// checking all words
		while (i < actualSize)
		{
			start = i;
			chars = 0;
			charX = 0.0f;
			previousWordWidth = 0.0f;
			wordWidth = 0.0f;
			wordBearingX = 0.0f;
			icon = false;
			// checking a whole word
			while (i < actualSize)
			{
				ax = 0.0f;
				aw = 0.0f;
				addW = 0.0f;
				previousCode = code;
				code = text.firstUnicodeChar(i, &byteSize);
				this->_checkFormatTags(text, i);
				if (this->_iconFont != NULL)
				{
					if (i > start)
					{
						break;
					}
					icon = true;
					if (this->_icons.hasKey(this->_fontIconName))
					{
						this->_icon = this->_icons[this->_fontIconName];
						this->_scale = this->_iconFontScale * this->_textScale;
						ax = this->_icon->advance * this->_scale;
						if (this->_iconFontBearingX < 0.0f)
						{
							ax -= this->_iconFontBearingX * this->_scale;
							bearingX = charX + this->_iconFontBearingX * this->_scale;
							if (bearingX < 0)
							{
								charX = 0.0f;
								wordBearingX = hmin(wordBearingX, bearingX);
								foreach (float, it, charXs)
								{
									(*it) -= bearingX;
								}
								foreach (float, it, segmentWidths)
								{
									(*it) -= bearingX;
								}
							}
							else
							{
								charX = bearingX;
							}
						}
						aw = (this->_icon->rect.w + this->_iconFontBearingX) * this->_scale;
						addW = hmax(ax, aw);
					}
					previousWordWidth = wordWidth;
					wordWidth = hmax(charX + addW, wordWidth);
					if (wordWidth > rect.w) // word too long for line
					{
						wordWidth = previousWordWidth;
						tooLong = true;
						break;
					}
					charXs += charX;
					charX += ax;
					charAdvanceXs += ax;
					segmentWidths += wordWidth;
					i += byteSize;
					++chars;
					this->_iconFont = NULL;
					break;
				}
				if (code == UNICODE_CHAR_NEWLINE)
				{
					if (i == start)
					{
						i += byteSize;
						++chars;
					}
					break;
				}
				if ((code == UNICODE_CHAR_SPACE || code == UNICODE_CHAR_ZERO_WIDTH_SPACE) != checkingSpaces)
				{
					break;
				}
				// non-initial font might need character table update
				if (initialFontName != this->_fontName && !this->_characters.hasKey(code) && this->_font->hasCharacter(code))
				{
					this->_characters = this->_font->getCharacters();
				}
				if (this->_characters.hasKey(code))
				{
					this->_character = this->_characters[code];
					this->_scale = this->_fontScale * this->_textScale;
					kerning = 0.0f;
					if (this->_font != NULL)
					{
						kerning = this->_font->getKerning(previousCode, code);
					}
					ax = (this->_character->advance + kerning) * this->_scale;
					if (this->_character->bearing.x < 0.0f)
					{
						ax -= this->_character->bearing.x * this->_scale;
						bearingX = charX + this->_character->bearing.x * this->_scale;
						if (bearingX < 0)
						{
							charX = 0.0f;
							wordBearingX = hmin(wordBearingX, bearingX);
							foreach (float, it, charXs)
							{
								(*it) -= bearingX;
							}
							foreach (float, it, segmentWidths)
							{
								(*it) -= bearingX;
							}
						}
						else
						{
							charX = bearingX;
						}
					}
					aw = (this->_character->rect.w + this->_character->bearing.x + kerning) * this->_scale;
					addW = hmax(ax, aw);
				}
				else
				{
					addW = this->_font->getHeight() * 0.5f;
				}
				previousWordWidth = wordWidth;
				wordWidth = hmax(charX + addW, wordWidth);
				if (wordWidth > rect.w) // word too long for line
				{
					if (!checkingSpaces)
					{
						wordWidth = previousWordWidth;
						tooLong = true;
					}
					break;
				}
				charXs += charX;
				charX += ax;
				charAdvanceXs += ax;
				segmentWidths += wordWidth;
				i += byteSize;
				++chars;
				if (!checkingSpaces && i < actualSize)
				{
					if (!this->useLegacyLineBreakParsing)
					{
						if (!this->useIdeographWords)
						{
							if (chars >= 2 && IS_PUNCTUATION_CHAR(code))
							{
								break;
							}
						}
						else if (IS_IDEOGRAPH(code) || IS_PUNCTUATION_CHAR(code))
						{
							unsigned int nextCode = text.firstUnicodeChar(i);
							if (!IS_PUNCTUATION_CHAR(nextCode))
							{
								break;
							}
						}
					}
					else if (IS_PUNCTUATION_CHAR(code))
					{
						break;
					}
				}
			}
			if (i > start)
			{
				word.text = (!icon ? text(start, i - start) : "");
				word.rect.w = wordWidth;
				word.advanceX = charX;
				word.bearingX = wordBearingX;
				word.start = start;
				word.count = (!icon ? i - start : 0);
				word.spaces = (!icon && checkingSpaces ? i - start : 0);
				word.icon = icon;
				word.charXs = charXs;
				word.charAdvanceXs = charAdvanceXs;
				word.segmentWidths = segmentWidths;
				result += word;
				charXs.clear();
				charAdvanceXs.clear();
				segmentWidths.clear();
			}
			else if (tooLong) // this prevents an infinite loop if not at least one character fits in the line
			{
				hlog::warn(logTag, "String does not fit in rect: " + text);
				break;
			}
			tooLong = false;
			if (this->_iconFont != NULL)
			{
				checkingSpaces = false;
			}
			else if (!icon)
			{
				checkingSpaces = !checkingSpaces;
			}
		}
		return result;
	}

	harray<RenderLine> Renderer::createRenderLines(cgrect rect, chstr text, const harray<FormatTag>& tags,
		Horizontal horizontal, Vertical vertical, cgvec2 offset)
	{
		this->analyzeText(tags.first().data, text); // by convention, the first tag is the font name
		harray<RenderWord> words = this->createRenderWords(rect, text, tags);
		this->_initializeLineProcessing();
		// helper variables
		bool wrapped = horizontal.isWrapped();
		bool untrimmed = horizontal.isUntrimmed();
		float lineWidth = 0.0f;
		float x = 0.0f;
		bool nextLine = false;
		bool forcedNextLine = false;
		bool addWord = false;
		this->_line.rect.x = rect.x;
		this->_line.rect.h = this->_height;
		// iterate through each word
		for_iter (i, 0, words.size())
		{
			nextLine = (i == words.size() - 1);
			addWord = true;
			forcedNextLine = false;
			if (words[i].text == "\n")
			{
				addWord = false;
				nextLine = true;
				forcedNextLine = true;
			}
			else if (this->_line.words.size() == 0 && words[i].spaces > 0 && wrapped && !untrimmed)
			{
				addWord = false;
			}
			else if (lineWidth + words[i].rect.w > rect.w && wrapped)
			{
				if (this->_line.words.size() > 0)
				{
					addWord = false;
					--i;
				}
				// else the whole word is the only one in the line and doesn't fit, so just chop it off
				nextLine = true;
			}
			if (this->_line.words.size() == 0) // if no words yet, this word's start becomes the line start
			{
				this->_line.start = words[i].start;
			}
			if (addWord)
			{
				words[i].rect.y += this->_lines.size() * this->_lineHeight;
				lineWidth += words[i].advanceX;
				this->_line.words += words[i];
				this->_line.count += words[i].count;
			}
			if (nextLine)
			{
				// remove spaces at beginning and end in wrapped formatting styles
				if (wrapped && !untrimmed)
				{
					while (this->_line.words.size() > 0 && this->_line.words.first().spaces > 0)
					{
						this->_line.words.removeFirst();
					}
					while (this->_line.words.size() > 0 && this->_line.words.last().spaces > 0)
					{
						this->_line.words.removeLast();
					}
				}
				if (this->_line.words.size() > 0)
				{
					x = this->_line.words.first().rect.x;
					foreach (RenderWord, it, this->_line.words)
					{
						this->_line.text += (*it).text;
						this->_line.spaces += (*it).spaces;
						this->_line.advanceX += (*it).advanceX;
						(*it).rect.x = x;
						x += (*it).advanceX;
					}
					this->_line.rect.w = this->_line.advanceX + hmax(this->_line.words.last().rect.w - this->_line.words.last().advanceX, 0.0f);
				}
				this->_line.rect.y = rect.y + this->_lines.size() * this->_lineHeight;
				this->_line.terminated = forcedNextLine;
				if (this->_line.words.size() > 0 || this->_line.terminated) // prevents empty lines with only spaces to be used
				{
					this->_lines += this->_line;
				}
				// reset
				this->_line.text = "";
				this->_line.start = 0;
				this->_line.count = 0;
				this->_line.spaces = 0;
				this->_line.advanceX = 0.0f;
				this->_line.terminated = false;
				this->_line.rect.w = 0.0f;
				this->_line.words.clear();
				lineWidth = 0.0f;
			}
		}
		if (this->_lines.size() > 0)
		{
			this->verticalCorrection(this->_lines, rect, vertical, offset.y, this->_lineHeight, this->_descender, this->_internalDescender);
			this->_lines = this->removeOutOfBoundLines(this->_lines, rect);
			if (this->_lines.size() > 0)
			{
				this->horizontalCorrection(this->_lines, rect, horizontal, offset.x);
			}
		}
		return this->_lines;
	}
	
	RenderText Renderer::createRenderText(cgrect rect, chstr text, const harray<RenderLine>& lines, const harray<FormatTag>& tags)
	{
		this->analyzeText(tags.first().data, text); // by convention, the first tag is the font name
		this->_initializeFormatTags(tags);
		this->_initializeRenderSequences();
		this->_initializeLineProcessing(lines);
		// helper variables
		int byteSize = 0;
		float characterX = 0.0f;
		float advanceX = 0.0f;
		float wordX = 0.0f;
		float bearingX = 0.0f;
		RenderRectangle currentRect;
		grect area;
		grect drawRect;
		gvec2 rectSize;
		grect liningRect;
		int index = 0;
		// basic text with borders, shadows and icons
		for_iter (j, 0, this->_lines.size())
		{
			this->_line = this->_lines[j];
			bearingX = 0.0f;
			foreach (RenderWord, it, this->_line.words)
			{
				this->_word = (*it);
				wordX = this->_word.rect.x - this->_line.rect.x;
				bearingX += this->_word.bearingX;
				index = 0;
				if (this->_word.icon)
				{
					// checking first formatting tag changes
					this->_processFormatTags(this->_word.text, 0);
					this->_iconName = this->_fontIconName;
					// if icon exists in current font
					if (this->_icons.hasKey(this->_iconName))
					{
						// checking the particular character
						this->_scale = this->_iconFontScale * this->_textScale;
						this->_icon = this->_icons[this->_iconName];
						this->_shadowOffset = this->shadowOffset * this->_textShadowOffset;
						this->_borderThickness = this->borderThickness * this->_textBorderThickness;
						this->_borderFontThickness = this->_borderThickness;
						this->_strikeThroughThickness = this->strikeThroughThickness * this->_textStrikeThroughThickness;
						this->_underlineThickness = this->underlineThickness * this->_textUnderlineThickness;
						area = this->_word.rect;
						if (wordX + bearingX > 0.0f)
						{
							//area.x += bearingX;
						}
						else
						{
							//bearingX = 0.0f;
						}
						characterX = area.x + this->_word.charXs[index];
						area.x += this->_word.charXs[index];
						area.y += (this->_lineHeight - this->_height) * 0.5f + this->_iconFontOffsetY * this->_scale;
						area.w = this->_icon->rect.w * this->_scale;
						area.h = this->_icon->rect.h * this->_scale;
						area.y += this->_lineHeight * (1.0f - this->_textScale) * 0.5f;
						area.y += (this->_height - this->_icon->rect.h * this->_scale) * 0.5f;
						drawRect = rect;
						advanceX = this->_word.charXs[index] + this->_word.charAdvanceXs[index];
						if (this->_iconFont != NULL)
						{
							this->_renderRect = this->_iconFont->makeRenderRectangle(drawRect, area, this->_iconName);
							if (this->_renderRect.src.w > 0.0f && this->_renderRect.src.h > 0.0f && this->_renderRect.dest.w > 0.0f && this->_renderRect.dest.h > 0.0f)
							{
								this->_textSequence.addRenderRectangle(this->_renderRect);
								switch (this->_effectMode)
								{
								case EFFECT_MODE_SHADOW: // shadow
									this->_renderRect.dest += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
									this->_shadowSequence.addRenderRectangle(this->_renderRect);
									break;
								case EFFECT_MODE_BORDER: // border
									if (this->_iconFont->getBorderMode() == Font::BorderMode::Software || !this->_iconFont->hasBorderIcon(this->_iconName, this->_borderFontThickness))
									{
										currentRect = this->_renderRect;
										this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness * sqrt05, -this->_borderThickness * sqrt05);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness * sqrt05, -this->_borderThickness * sqrt05);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness * sqrt05, this->_borderThickness * sqrt05);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness * sqrt05, this->_borderThickness * sqrt05);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(0.0f, -this->_borderThickness);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness, 0.0f);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness, 0.0f);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_renderRect.dest = currentRect.dest + gvec2(0.0f, this->_borderThickness);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_borderSequence.multiplyAlpha = true;
									}
									else
									{
										this->_borderIcon = this->_iconFont->getBorderIcon(this->_iconName, this->_borderFontThickness);
										area = this->_word.rect;
										rectSize = (this->_borderIcon->rect.getSize() - this->_icon->rect.getSize()) * 0.5f * this->_scale;
										area.x += this->_word.charXs[index] - rectSize.x;
										area.y += (this->_lineHeight - this->_height) * 0.5f + this->_iconFontOffsetY * this->_scale - rectSize.y;
										area.w = this->_borderIcon->rect.w * this->_scale;
										area.h = this->_borderIcon->rect.h * this->_scale;
										area.y += this->_lineHeight * (1.0f - this->_textScale) * 0.5f;
										area.y += (this->_height - this->_icon->rect.h * this->_scale) * 0.5f;
										drawRect.x -= rectSize.x;
										drawRect.y -= rectSize.y;
										drawRect.w += rectSize.x * 2.0f;
										drawRect.h += rectSize.y * 2.0f;
										this->_renderRect = this->_iconFont->makeBorderRenderRectangle(drawRect, area, this->_iconName, this->_borderFontThickness);
										this->_borderSequence.addRenderRectangle(this->_renderRect);
										this->_borderSequence.texture = this->_iconFont->getBorderTexture(this->_iconName, this->_borderFontThickness);
										this->_borderSequence.multiplyAlpha = false;
									}
									break;
								default:
									break;
								}
								this->_updateLiningSequenceSwitch();
								if (this->_strikeThroughActive)
								{
									liningRect.x = characterX;
									liningRect.y = this->_word.rect.y + (this->_height - this->_strikeThroughThickness) * 0.5f + this->_strikeThroughOffset;
									liningRect.w = advanceX;
									liningRect.h = this->_strikeThroughThickness;
									this->_textStrikeThroughSequence.addRectangle(liningRect);
									switch (this->_effectMode)
									{
									case EFFECT_MODE_SHADOW: // shadow
										liningRect += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
										this->_shadowStrikeThroughSequence.addRectangle(liningRect);
										break;
									case EFFECT_MODE_BORDER: // border
										liningRect.x -= this->_borderThickness;
										liningRect.y -= this->_borderThickness;
										liningRect.w += this->_borderThickness * 2.0f;
										liningRect.h += this->_borderThickness * 2.0f;
										this->_borderStrikeThroughSequence.addRectangle(liningRect);
										break;
									default:
										break;
									}
								}
								if (this->_underlineActive)
								{
									liningRect.x = characterX;
									liningRect.y = this->_word.rect.y + this->_height + this->_underlineOffset;
									liningRect.w = advanceX;
									liningRect.h = this->_underlineThickness;
									this->_textUnderlineSequence.addRectangle(liningRect);
									switch (this->_effectMode)
									{
									case EFFECT_MODE_SHADOW: // shadow
										liningRect += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
										this->_shadowUnderlineSequence.addRectangle(liningRect);
										break;
									case EFFECT_MODE_BORDER: // border
										liningRect.x -= this->_borderThickness;
										liningRect.y -= this->_borderThickness;
										liningRect.w += this->_borderThickness * 2.0f;
										liningRect.h += this->_borderThickness * 2.0f;
										this->_borderUnderlineSequence.addRectangle(liningRect);
										break;
									default:
										break;
									}
								}
							}
						}
					}
				}
				else
				{
					for_iter_step (i, 0, this->_word.text.size(), byteSize)
					{
						this->_code = this->_word.text.firstUnicodeChar(i, &byteSize);
						// checking first formatting tag changes
						this->_processFormatTags(this->_word.text, i);
						// if character exists in current font
						if (this->_characters.hasKey(this->_code))
						{
							// checking the particular character
							this->_scale = this->_fontScale * this->_textScale;
							this->_character = this->_characters[this->_code];
							this->_shadowOffset = this->shadowOffset * this->_textShadowOffset;
							this->_borderThickness = this->borderThickness * this->_textBorderThickness;
							this->_borderFontThickness = this->_borderThickness / this->_fontBaseScale;
							this->_strikeThroughThickness = this->strikeThroughThickness * this->_textStrikeThroughThickness;
							this->_underlineThickness = this->underlineThickness * this->_textUnderlineThickness;
							area = this->_word.rect;
							if (wordX + bearingX > 0.0f)
							{
								//area.x += bearingX;
							}
							else
							{
								//area.x += bearingX;
								//bearingX += wordX;
								//bearingX = 0.0f;
							}
							characterX = area.x + this->_word.charXs[index];// +this->_word.bearingX;
							area.x += this->_word.charXs[index];// +this->_word.bearingX;
							area.y += (this->_lineHeight - this->_height) * 0.5f + this->_character->offsetY * this->_scale;
							area.w = this->_character->rect.w * this->_scale;
							area.h = this->_character->rect.h * this->_scale;
							area.y += this->_lineHeight * (1.0f - this->_textScale) * 0.5f;
							drawRect = rect;
							advanceX = this->_word.charXs[index] + this->_word.charAdvanceXs[index];
							// optimization, don't even render spaces
							if (this->_font != NULL && this->_code != UNICODE_CHAR_SPACE && this->_code != UNICODE_CHAR_ZERO_WIDTH_SPACE)
							{
								this->_renderRect = this->_font->makeRenderRectangle(drawRect, area, this->_code);
								if (this->_renderRect.src.w > 0.0f && this->_renderRect.src.h > 0.0f && this->_renderRect.dest.w > 0.0f && this->_renderRect.dest.h > 0.0f)
								{
									this->_renderRect.dest.y -= this->_character->bearing.y * this->_scale;
									this->_textSequence.addRenderRectangle(this->_renderRect);
									switch (this->_effectMode)
									{
									case EFFECT_MODE_SHADOW: // shadow
										this->_renderRect.dest += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
										this->_shadowSequence.addRenderRectangle(this->_renderRect);
										break;
									case EFFECT_MODE_BORDER: // border
										if (this->_font->getBorderMode() == Font::BorderMode::Software || !this->_font->hasBorderCharacter(this->_code, this->_borderFontThickness))
										{
											currentRect = this->_renderRect;
											this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness * sqrt05, -this->_borderThickness * sqrt05);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness * sqrt05, -this->_borderThickness * sqrt05);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness * sqrt05, this->_borderThickness * sqrt05);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness * sqrt05, this->_borderThickness * sqrt05);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(0.0f, -this->_borderThickness);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(-this->_borderThickness, 0.0f);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(this->_borderThickness, 0.0f);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_renderRect.dest = currentRect.dest + gvec2(0.0f, this->_borderThickness);
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_borderSequence.multiplyAlpha = true;
										}
										else
										{
											this->_borderCharacter = this->_font->getBorderCharacter(this->_code, this->_borderFontThickness);
											area = this->_word.rect;
											rectSize = (this->_borderCharacter->rect.getSize() - this->_character->rect.getSize()) * 0.5f * this->_scale;
											area.x += this->_word.charXs[index] - rectSize.x;
											area.y += (this->_lineHeight - this->_height) * 0.5f + this->_character->offsetY * this->_scale - rectSize.y;
											area.w = this->_borderCharacter->rect.w * this->_scale;
											area.h = this->_borderCharacter->rect.h * this->_scale;
											area.y += this->_lineHeight * (1.0f - this->_textScale) * 0.5f;
											drawRect.x -= rectSize.x;
											drawRect.y -= rectSize.y;
											drawRect.w += rectSize.x * 2.0f;
											drawRect.h += rectSize.y * 2.0f;
											this->_renderRect = this->_font->makeBorderRenderRectangle(drawRect, area, this->_code, this->_borderFontThickness);
											this->_renderRect.dest.y -= this->_character->bearing.y * this->_scale;
											this->_borderSequence.addRenderRectangle(this->_renderRect);
											this->_borderSequence.texture = this->_font->getBorderTexture(this->_code, this->_borderFontThickness);
											this->_borderSequence.multiplyAlpha = false;
										}
										break;
									default:
										break;
									}
									this->_updateLiningSequenceSwitch();
									if (this->_strikeThroughActive)
									{
										liningRect.x = characterX;
										liningRect.y = this->_word.rect.y + (this->_height - this->_strikeThroughThickness) * 0.5f + this->_strikeThroughOffset;
										liningRect.w = advanceX;
										liningRect.h = this->_strikeThroughThickness;
										liningRect.clip(rect);
										if (liningRect.w > 0.0f && liningRect.h > 0.0f)
										{
											this->_textStrikeThroughSequence.addRectangle(liningRect);
											switch (this->_effectMode)
											{
											case EFFECT_MODE_SHADOW: // shadow
												liningRect += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
												this->_shadowStrikeThroughSequence.addRectangle(liningRect);
												break;
											case EFFECT_MODE_BORDER: // border
												liningRect.x -= this->_borderThickness;
												liningRect.y -= this->_borderThickness;
												liningRect.w += this->_borderThickness * 2.0f;
												liningRect.h += this->_borderThickness * 2.0f;
												this->_borderStrikeThroughSequence.addRectangle(liningRect);
												break;
											default:
												break;
											}
										}
									}
									if (this->_underlineActive)
									{
										liningRect.x = characterX;
										liningRect.y = this->_word.rect.y + this->_height + this->_underlineOffset;
										liningRect.w = advanceX;
										liningRect.h = this->_underlineThickness;
										liningRect.clip(rect);
										if (liningRect.w > 0.0f && liningRect.h > 0.0f)
										{
											this->_textUnderlineSequence.addRectangle(liningRect);
											switch (this->_effectMode)
											{
											case EFFECT_MODE_SHADOW: // shadow
												liningRect += this->_shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
												this->_shadowUnderlineSequence.addRectangle(liningRect);
												break;
											case EFFECT_MODE_BORDER: // border
												liningRect.x -= this->_borderThickness;
												liningRect.y -= this->_borderThickness;
												liningRect.w += this->_borderThickness * 2.0f;
												liningRect.h += this->_borderThickness * 2.0f;
												this->_borderUnderlineSequence.addRectangle(liningRect);
												break;
											default:
												break;
											}
										}
									}
								}
							}
						}
						++index;
					}
				}
			}
		}
		if (this->_textSequence.vertices.size() > 0)
		{
			this->_textSequences += this->_textSequence;
			this->_textSequence.vertices.clear();
		}
		if (this->_shadowSequence.vertices.size() > 0)
		{
			this->_shadowSequences += this->_shadowSequence;
			this->_shadowSequence.vertices.clear();
		}
		if (this->_borderSequence.vertices.size() > 0)
		{
			this->_borderSequences += this->_borderSequence;
			this->_borderSequence.vertices.clear();
		}
		this->_updateLiningSequenceSwitch(true);
		// clear data and optimizations
		this->_lines.clear();
		RenderText result;
		result.textSequences = this->optimizeSequences(this->_textSequences);
		result.shadowSequences = this->optimizeSequences(this->_shadowSequences);
		result.borderSequences = this->optimizeSequences(this->_borderSequences);
		result.textLiningSequences = this->optimizeSequences(this->_textLiningSequences);
		result.shadowLiningSequences = this->optimizeSequences(this->_shadowLiningSequences);
		result.borderLiningSequences = this->optimizeSequences(this->_borderLiningSequences);
		return result;
	}

	harray<RenderSequence> Renderer::optimizeSequences(harray<RenderSequence>& sequences)
	{
		harray<RenderSequence> result;
		RenderSequence current;
		while (sequences.size() > 0)
		{
			current = sequences.removeFirst();
			for_iter (i, 0, sequences.size())
			{
				if (current.texture == sequences[i].texture && current.color.hex(true) == sequences[i].color.hex(true) && current.multiplyAlpha == sequences[i].multiplyAlpha)
				{
					current.vertices += sequences[i].vertices;
					sequences.removeAt(i);
					--i;
				}
			}
			result += current;
		}
		return result;
	}

	harray<RenderLiningSequence> Renderer::optimizeSequences(harray<RenderLiningSequence>& sequences)
	{
		harray<RenderLiningSequence> result;
		RenderLiningSequence current;
		while (sequences.size() > 0)
		{
			current = sequences.removeFirst();
			for_iter (i, 0, sequences.size())
			{
				if (current.color.hex(true) == sequences[i].color.hex(true))
				{
					current.vertices += sequences[i].vertices;
					sequences.removeAt(i);
					--i;
				}
			}
			result += current;
		}
		return result;
	}

	void Renderer::_drawRenderText(RenderText& renderText, const april::Color& color)
	{
		foreach (RenderSequence, it, renderText.shadowSequences)
		{
			this->_drawRenderSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f())));
		}
		foreach (RenderLiningSequence, it, renderText.shadowLiningSequences)
		{
			this->_drawRenderLiningSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f())));
		}
		foreach (RenderSequence, it, renderText.borderSequences)
		{
			if ((*it).multiplyAlpha)
			{
				this->_drawRenderSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f() * color.a_f())));
			}
			else
			{
				this->_drawRenderSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f())));
			}
		}
		foreach (RenderLiningSequence, it, renderText.borderLiningSequences)
		{
			this->_drawRenderLiningSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f())));
		}
		foreach (RenderSequence, it, renderText.textSequences)
		{
#ifdef _DEBUG_RENDER_TEXT
			april::rendersys->setTexture(NULL);
			april::rendersys->setBlendMode(april::BlendMode::Alpha);
			april::rendersys->setColorMode(april::ColorMode::Multiply);
			static harray<april::PlainVertex> v;
			for_iter (i, 0, (*it).vertices.size() / 3)
			{
				v += april::PlainVertex((*it).vertices[i * 3]);
				v.add(april::PlainVertex((*it).vertices[i * 3 + 1]), 2);
				v.add(april::PlainVertex((*it).vertices[i * 3 + 2]), 2);
				v += april::PlainVertex((*it).vertices[i * 3]);
			}
			static april::Color polygonColor(april::Color::Red, 128);
			april::rendersys->render(april::RenderOperation::LineList, (april::PlainVertex*)v, v.size(), polygonColor);
			v.clear();
#endif
			this->_drawRenderSequence((*it), april::Color((*it).color, color.a));
		}
		foreach (RenderLiningSequence, it, renderText.textLiningSequences)
		{
			this->_drawRenderLiningSequence((*it), april::Color((*it).color, color.a));
		}
	}

	void Renderer::_drawRenderSequence(RenderSequence& sequence, const april::Color& color)
	{
		if (sequence.vertices.size() == 0 || sequence.texture == NULL || color.a == 0)
		{
			return;
		}
		april::rendersys->setTexture(sequence.texture);
		april::rendersys->setBlendMode(april::BlendMode::Alpha);
		if (sequence.texture->getFormat() == april::Image::Format::Alpha)
		{
			april::rendersys->setColorMode(april::ColorMode::AlphaMap);
		}
		else
		{
			april::rendersys->setColorMode(april::ColorMode::Multiply);
		}
		april::rendersys->render(april::RenderOperation::TriangleList, (april::TexturedVertex*)sequence.vertices, sequence.vertices.size(), color);
	}

	void Renderer::_drawRenderLiningSequence(RenderLiningSequence& sequence, const april::Color& color)
	{
		if (sequence.vertices.size() == 0 || color.a == 0)
		{
			return;
		}
		april::rendersys->setBlendMode(april::BlendMode::Alpha);
		april::rendersys->setColorMode(april::ColorMode::Multiply);
		april::rendersys->render(april::RenderOperation::TriangleList, (april::PlainVertex*)sequence.vertices, sequence.vertices.size(), color);
	}

	bool Renderer::_checkTextures()
	{
		foreach (RenderSequence, it, this->_cacheEntryText.value.textSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cacheText
				return false;
			}
		}
		foreach (RenderSequence, it, this->_cacheEntryText.value.shadowSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cacheText
				return false;
			}
		}
		foreach (RenderSequence, it, this->_cacheEntryText.value.borderSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cacheText
				return false;
			}
		}
		return true;
	}
	
	void Renderer::drawText(chstr fontName, cgrect rect, chstr text, Horizontal horizontal, Vertical vertical, const april::Color& color,
		cgvec2 offset)
	{
		this->_cacheEntryText.set(text, fontName, rect, horizontal, vertical, color, offset);
		if (!this->cacheText->get(this->_cacheEntryText) || !this->_checkTextures())
		{
			hstr unformattedText = text;
			harray<FormatTag> tags = this->_makeDefaultTags(color, fontName, unformattedText);
			this->_cacheEntryLines.set(text, fontName, rect, horizontal, vertical, april::Color(color, 255), offset);
			if (this->cacheLines->get(this->_cacheEntryLines))
			{
				this->_lines = this->_cacheEntryLines.value;
			}
			else
			{
				this->_lines = this->createRenderLines(rect, unformattedText, tags, horizontal, vertical, offset);
			}
			this->_cacheEntryText.value = this->createRenderText(rect, text, this->_lines, tags);
			this->cacheText->add(this->_cacheEntryText);
			this->cacheText->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
	}

	void Renderer::drawTextUnformatted(chstr fontName, cgrect rect, chstr text, Horizontal horizontal, Vertical vertical,
		const april::Color& color, cgvec2 offset)
	{
		this->_cacheEntryText.set(text, fontName, rect, horizontal, vertical, april::Color(color, 255), offset);
		if (!this->cacheTextUnformatted->get(this->_cacheEntryText) || !this->_checkTextures())
		{
			harray<FormatTag> tags = this->_makeDefaultTagsUnformatted(color, fontName);
			this->_cacheEntryLines.set(text, fontName, rect, horizontal, vertical, april::Color(color, 255), offset);
			if (this->cacheLinesUnformatted->get(this->_cacheEntryLines))
			{
				this->_lines = this->_cacheEntryLines.value;
			}
			else
			{
				this->_lines = this->createRenderLines(rect, text, tags, horizontal, vertical, offset);
			}
			this->_cacheEntryText.value = this->createRenderText(rect, text, this->_lines, tags);
			this->cacheTextUnformatted->add(this->_cacheEntryText);
			this->cacheTextUnformatted->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
	}

	void Renderer::drawText(cgrect rect, chstr text, Horizontal horizontal, Vertical vertical, const april::Color& color, cgvec2 offset)
	{
		this->drawText("", rect, text, horizontal, vertical, color, offset);
	}

	void Renderer::drawTextUnformatted(cgrect rect, chstr text, Horizontal horizontal, Vertical vertical, const april::Color& color, cgvec2 offset)
	{
		this->drawTextUnformatted("", rect, text, horizontal, vertical, color, offset);
	}

	harray<RenderLine> Renderer::makeRenderLines(chstr fontName, cgrect rect, chstr text, Horizontal horizontal, Vertical vertical, const april::Color& color, cgvec2 offset)
	{
		this->_cacheEntryLines.set(text, fontName, rect, horizontal, vertical, color, offset);
		if (!this->cacheLines->get(this->_cacheEntryLines))
		{
			hstr unformattedText = text;
			harray<FormatTag> tags = this->_makeDefaultTags(color, fontName, unformattedText);
			this->_cacheEntryLines.value = this->createRenderLines(rect, unformattedText, tags, horizontal, vertical, offset);
			this->cacheLines->add(this->_cacheEntryLines);
			this->cacheLines->update();
		}
		return this->_cacheEntryLines.value;
	}

	harray<RenderLine> Renderer::makeRenderLinesUnformatted(chstr fontName, cgrect rect, chstr text, Horizontal horizontal, Vertical vertical, const april::Color& color, cgvec2 offset)
	{
		this->_cacheEntryLines.set(text, fontName, rect, horizontal, vertical, april::Color(color, 255), offset);
		if (!this->cacheLinesUnformatted->get(this->_cacheEntryLines))
		{
			harray<FormatTag> tags = this->_makeDefaultTagsUnformatted(color, fontName);
			this->_cacheEntryLines.value = this->createRenderLines(rect, text, tags, horizontal, vertical, offset);
			this->cacheLinesUnformatted->add(this->_cacheEntryLines);
			this->cacheLinesUnformatted->update();
		}
		return this->_cacheEntryLines.value;
	}

	harray<FormatTag> Renderer::_makeDefaultTags(const april::Color& color, chstr fontName, hstr& text)
	{
		harray<FormatTag> tags;
		text = this->analyzeFormatting(text, tags);
		FormatTag tag;
		tag.type = FormatTag::Type::Color;
		tag.data = color.hex();
		tags.addFirst(tag);
		tag.type = FormatTag::Type::Font;
		tag.data = fontName;
		tags.addFirst(tag);
		return tags;
	}

	harray<FormatTag> Renderer::_makeDefaultTagsUnformatted(const april::Color& color, chstr fontName)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = FormatTag::Type::Color;
		tag.data = color.hex();
		tags.addFirst(tag);
		tag.type = FormatTag::Type::Font;
		tag.data = fontName;
		tags.addFirst(tag);
		return tags;
	}

	float Renderer::getTextWidth(chstr fontName, chstr text)
	{
		float result = 0.0f;
		if (text != "")
		{
			static grect defaultRect(0.0f, 0.0f, CHECK_RECT_SIZE, CHECK_RECT_SIZE);
			this->_lines = this->makeRenderLines(fontName, defaultRect, text);
			foreach (RenderLine, it, this->_lines)
			{
				result = hmax(result, (*it).rect.w);
			}
		}
		return result;
	}

	float Renderer::getTextWidth(chstr text)
	{
		return this->getTextWidth("", text);
	}

	float Renderer::getTextWidthUnformatted(chstr fontName, chstr text)
	{
		return this->getTextWidth(fontName, "[-]" + text);
	}

	float Renderer::getTextWidthUnformatted(chstr text)
	{
		return this->getTextWidth("", "[-]" + text);
	}

	float Renderer::getTextAdvanceX(chstr fontName, chstr text)
	{
		float result = 0.0f;
		if (text != "")
		{
			static grect defaultRect(0.0f, 0.0f, CHECK_RECT_SIZE, CHECK_RECT_SIZE);
			this->_lines = this->makeRenderLines(fontName, defaultRect, text);
			foreach (RenderLine, it, this->_lines)
			{
				result = hmax(result, (*it).advanceX);
			}
		}
		return result;
	}

	float Renderer::getTextAdvanceX(chstr text)
	{
		return this->getTextAdvanceX("", text);
	}

	float Renderer::getTextAdvanceXUnformatted(chstr fontName, chstr text)
	{
		return this->getTextAdvanceX(fontName, "[-]" + text);
	}

	float Renderer::getTextAdvanceXUnformatted(chstr text)
	{
		return this->getTextAdvanceX("", "[-]" + text);
	}

	float Renderer::getTextHeight(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			grect defaultRect(0.0f, 0.0f, maxWidth, CHECK_RECT_SIZE);
			this->_lines = this->makeRenderLines(fontName, defaultRect, text, Horizontal::LeftWrapped, Vertical::Top);
			if (this->_lines.size() > 0)
			{
				Font* font = this->getFont(fontName);
				return (this->_lines.size() * font->getLineHeight() + font->getInternalDescender());
			}
		}
		return 0.0f;
	}
	
	float Renderer::getTextHeight(chstr text, float maxWidth)
	{
		return this->getTextHeight("", text, maxWidth);
	}

	float Renderer::getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		return this->getTextHeight(fontName, "[-]" + text, maxWidth);
	}

	float Renderer::getTextHeightUnformatted(chstr text, float maxWidth)
	{
		return this->getTextHeight("", "[-]" + text, maxWidth);
	}

	hstr Renderer::getFittingText(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			grect defaultRect(0.0f, 0.0f, CHECK_RECT_SIZE, CHECK_RECT_SIZE);
			this->_lines = this->makeRenderLines(fontName, defaultRect, text, Horizontal::LeftWrapped, Vertical::Top);
			if (this->_lines.size() > 0)
			{
				float width = 0.0f;
				harray<hstr> result;
				std::ustring ustr;
				foreach (RenderWord, it, this->_lines[0].words)
				{
					if ((*it).rect.right() > maxWidth)
					{
						ustr = (*it).text.uStr();
						for_itert (size_t, i, 0, ustr.size())
						{
							if (width + (*it).segmentWidths[i] > maxWidth)
							{
								break;
							}
							result += hstr::fromUnicode(ustr[i]);
						}
						break;
					}
					width = (*it).rect.right();
					result += (*it).text;
				}
				return result.joined("");
			}
		}
		return "";
	}

	hstr Renderer::getFittingText(chstr text, float maxWidth)
	{
		return this->getFittingText("", text, maxWidth);
	}

	hstr Renderer::getFittingTextUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		return this->getFittingText(fontName, "[-]" + text, maxWidth);
	}

	hstr Renderer::getFittingTextUnformatted(chstr text, float maxWidth)
	{
		return this->getFittingText("", "[-]" + text, maxWidth);
	}

}
