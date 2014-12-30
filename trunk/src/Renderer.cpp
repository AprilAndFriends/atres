/// @file
/// @version 3.43
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <math.h>
#include <stdio.h>

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

#define ALIGNMENT_IS_WRAPPED(formatting) ((formatting) == LEFT_WRAPPED || (formatting) == CENTER_WRAPPED || (formatting) == RIGHT_WRAPPED || (formatting) == JUSTIFIED)
#define ALIGNMENT_IS_LEFT(formatting) ((formatting) == LEFT || (formatting) == LEFT_WRAPPED || (formatting) == JUSTIFIED && !this->justifiedEnabled)

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

#define EFFECT_MODE_NORMAL 0
#define EFFECT_MODE_SHADOW 1
#define EFFECT_MODE_BORDER 2

#define CHECK_RECT_SIZE 100000.0f

namespace atres
{
	Renderer* renderer;

	Renderer::Renderer()
	{
		this->colors["white"] = april::Color::White.hex();
		this->colors["black"] = april::Color::Black.hex();
		this->colors["grey"] = april::Color::Grey.hex();
		this->colors["red"] = april::Color::Red.hex();
		this->colors["green"] = april::Color::Green.hex();
		this->colors["blue"] = april::Color::Blue.hex();
		this->colors["yellow"] = april::Color::Yellow.hex();
		this->colors["magenta"] = april::Color::Magenta.hex();
		this->colors["cyan"] = april::Color::Cyan.hex();
		this->colors["orange"] = april::Color::Orange.hex();
		this->colors["pink"] = april::Color::Pink.hex();
		this->colors["teal"] = april::Color::Teal.hex();
		this->colors["neon"] = april::Color::Neon.hex();
		this->colors["purple"] = april::Color::Purple.hex();
		this->colors["aqua"] = april::Color::Aqua.hex();
		this->colors["light_grey"] = april::Color::LightGrey.hex();
		this->colors["light_red"] = april::Color::LightRed.hex();
		this->colors["light_green"] = april::Color::LightGreen.hex();
		this->colors["light_blue"] = april::Color::LightBlue.hex();
		this->colors["light_yellow"] = april::Color::LightYellow.hex();
		this->colors["light_magenta"] = april::Color::LightMagenta.hex();
		this->colors["light_cyan"] = april::Color::LightCyan.hex();
		this->colors["light_orange"] = april::Color::LightOrange.hex();
		this->colors["light_pink"] = april::Color::LightPink.hex();
		this->colors["light_teal"] = april::Color::LightTeal.hex();
		this->colors["light_neon"] = april::Color::LightNeon.hex();
		this->colors["light_purple"] = april::Color::LightPurple.hex();
		this->colors["light_aqua"] = april::Color::LightAqua.hex();
		this->colors["dark_grey"] = april::Color::DarkGrey.hex();
		this->colors["dark_red"] = april::Color::DarkRed.hex();
		this->colors["dark_green"] = april::Color::DarkGreen.hex();
		this->colors["dark_blue"] = april::Color::DarkBlue.hex();
		this->colors["dark_yellow"] = april::Color::DarkYellow.hex();
		this->colors["dark_magenta"] = april::Color::DarkMagenta.hex();
		this->colors["dark_cyan"] = april::Color::DarkCyan.hex();
		this->colors["dark_orange"] = april::Color::DarkOrange.hex();
		this->colors["dark_pink"] = april::Color::DarkPink.hex();
		this->colors["dark_teal"] = april::Color::DarkTeal.hex();
		this->colors["dark_neon"] = april::Color::DarkNeon.hex();
		this->colors["dark_purple"] = april::Color::DarkPurple.hex();
		this->colors["dark_aqua"] = april::Color::DarkAqua.hex();
		// init
		this->shadowOffset.set(1.0f, 1.0f);
		this->shadowColor = april::Color::Black;
		this->borderOffset = 1.0f;
		this->borderColor = april::Color::Black;
		this->globalOffsets = false;
		this->useLegacyLineBreakParsing = false;
		this->useIdeographWords = false;
		this->justifiedEnabled = true;
		this->defaultFont = NULL;
		// misc init
		this->_font = NULL;
		// cache
		this->cacheText = new Cache<CacheEntryText>();
		this->cacheTextUnformatted = new Cache<CacheEntryText>();
		this->cacheLines = new Cache<CacheEntryLines>();
		this->cacheLinesUnformatted = new Cache<CacheEntryLines>();
		this->cacheLine = new Cache<CacheEntryLine>();
	}

	Renderer::~Renderer()
	{
		this->destroyAllFonts();
		delete this->cacheText;
		delete this->cacheTextUnformatted;
		delete this->cacheLines;
		delete this->cacheLinesUnformatted;
		delete this->cacheLine;
	}

/******* PROPERTIES ****************************************************/

	void Renderer::setShadowOffset(gvec2 value)
	{
		if (this->shadowOffset != value)
		{
			this->shadowOffset = value;
			this->clearCache();
		}
	}
	
	void Renderer::setShadowColor(april::Color value)
	{
		if (this->shadowColor != value)
		{
			this->shadowColor = value;
			this->clearCache();
		}
	}
	
	void Renderer::setBorderOffset(float value)
	{
		if (this->borderOffset != value)
		{
			this->borderOffset = value;
			this->clearCache();
		}
	}
	
	void Renderer::setBorderColor(april::Color value)
	{
		if (this->borderColor != value)
		{
			this->borderColor = value;
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

	void Renderer::setJustifiedEnabled(bool value)
	{
		if (this->justifiedEnabled != value)
		{
			this->justifiedEnabled = value;
			this->clearCache();
		}
	}

	void Renderer::setDefaultFont(chstr name)
	{
		if (name == "")
		{
			this->defaultFont = NULL;
			this->clearCache();
			return;
		}
		if (!this->fonts.has_key(name))
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
	
	hstr Renderer::getDefaultFont()
	{
		if (this->defaultFont == NULL) return "";
		return this->defaultFont->getName();
	}

	bool Renderer::hasFont(chstr name)
	{
		return (name == "" && this->defaultFont != NULL || this->fonts.has_key(name));
	}
	
	void Renderer::setCacheSize(int value)
	{
		this->cacheText->setMaxSize(value);
		this->cacheTextUnformatted->setMaxSize(value);
		this->cacheLines->setMaxSize(value);
		this->cacheLinesUnformatted->setMaxSize(value);
		this->cacheLine->setMaxSize(value);
	}
	
/******* FONT **********************************************************/

	void Renderer::registerFont(Font* font, bool allowDefault)
	{
		hstr name = font->getName();
		hlog::write(atres::logTag, "Registering font: " + name);
		if (this->fonts.has_key(name))
		{
			throw ResourceAlreadyExistsException("font", name, "atres");
		}
		this->fonts[name] = font;
		if (this->defaultFont == NULL && allowDefault)
		{
			this->defaultFont = font;
		}
		this->clearCache(); // there may be old cached definitions, they must be removed
	}
	
	void Renderer::unregisterFont(Font* font)
	{
		if (!this->fonts.has_value(font))
		{
			throw ResourceNotExistsException("font", font->getName(), "atres");
		}
		harray<hstr> keys = this->fonts.keys();
		foreach (hstr, it, keys) // removing aliases
		{
			if (this->fonts[*it] == font)
			{
				this->fonts.remove_key(*it);
			}
		}
		if (this->defaultFont == font)
		{
			this->defaultFont = (this->fonts.size() > 0 ? this->fonts.values().first() : NULL);
		}
		this->clearCache(); // there may be old cached definitions, they must be removed
	}

	void Renderer::registerFontAlias(chstr name, chstr alias)
	{
		if (this->fonts.has_key(alias))
		{
			throw ResourceAlreadyExistsException("font", alias, "atres");
		}
		Font* font = this->getFont(name);
		if (font != NULL)
		{
			hlog::writef(atres::logTag, "Registering font alias '%s' for '%s'.", alias.c_str(), font->getName().c_str());
			this->fonts[alias] = font;
		}
		else
		{
			hlog::errorf(atres::logTag, "Could not register alias '%s' for font '%s'. The font does not exist.", alias.c_str(), name.c_str());
		}
	}
	
	void Renderer::destroyAllFonts()
	{
		this->defaultFont = NULL;
		this->clearCache();
		harray<Font*> fonts = this->fonts.values().removed_duplicates();
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
		if (this->fonts.has_key(name))
		{
			font = this->fonts[name];
			font->setScale(1.0f);
			return font;
		}
		int position = name.find(":");
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
	
/******* MISC **********************************************************/

	void Renderer::addColor(chstr key, chstr value)
	{
		this->colors[key.lower()] = value.upper();
	}
	
	void Renderer::_updateCache()
	{
		this->cacheText->update();
		this->cacheTextUnformatted->update();
		this->cacheLines->update();
		this->cacheLinesUnformatted->update();
		this->cacheLine->update();
	}
	
	void Renderer::clearCache()
	{
		if (this->cacheText->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d text cache entries...", this->cacheText->size());
			this->cacheText->clear();
		}
		if (this->cacheTextUnformatted->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d unformatted text cache entries...", this->cacheTextUnformatted->size());
			this->cacheTextUnformatted->clear();
		}
		if (this->cacheLines->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d lines cache entries...", this->cacheLines->size());
			this->cacheLines->clear();
		}
		if (this->cacheLinesUnformatted->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d unformatted lines cache entries...", this->cacheLinesUnformatted->size());
			this->cacheLinesUnformatted->clear();
		}
		if (this->cacheLine->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d line calculation cache entries...", this->cacheLine->size());
			this->cacheLine->clear();
		}
	}
	
/******* ANALYZE TEXT **************************************************/

	void Renderer::analyzeText(chstr fontName, chstr text)
	{
		// makes sure dynamically allocated characters are loaded
		std::basic_string<unsigned int> chars = text.u_str();
		Font* font = this->getFont(fontName);
		if (font != NULL)
		{
			for_itert (unsigned int, i, 0, chars.size())
			{
				font->hasChar(chars[i]);
			}
		}
	}

	hstr Renderer::analyzeFormatting(chstr text, harray<FormatTag>& tags)
	{
		std::basic_string<unsigned int> uText = text.u_str();
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
			start = uText.find_first_of('[', start);
			if (start < 0)
			{
				break;
			}
			end = uText.find_first_of(']', start);
			if (end < 0)
			{
				break;
			}
			++end;
			tag.data = "";
			tag.start = text.utf8_substr(0, start).size();
			tag.count = text.utf8_substr(start, end - start).size();
			if (ignoreFormatting)
			{
				if (str[start + 1] != '/' || str[start + 2] != '-')
				{
					start = end;
					continue;
				}
				ignoreFormatting = false;
				stack.remove_last();
				tag.type = TAG_TYPE_CLOSE;
			}
			else if (end - start == 2) // empty command
			{
				tag.type = TAG_TYPE_ESCAPE;
			}
			else if (str[start + 1] == '/') // closing command
			{
				if (stack.size() == 0 || stack.last() != str[start + 2]) // interleaving, ignore the tag
				{
					start = end;
					hlog::warnf(atres::logTag, "Closing tag that was not opened ('[/%c]' in '%s')!", str[start + 2], str);
					continue;
				}
				stack.remove_last();
				tag.type = TAG_TYPE_CLOSE;
			}
			else // opening new tag
			{
				switch (str[start + 1])
				{
				case 'f':
					tag.type = TAG_TYPE_FONT;
					break;
				case 'c':
					tag.type = TAG_TYPE_COLOR;
					break;
				case 'x':
					tag.type = TAG_TYPE_SCALE;
					break;
				case 'n':
					tag.type = TAG_TYPE_NORMAL;
					break;
				case 's':
					tag.type = TAG_TYPE_SHADOW;
					break;
				case 'b':
					tag.type = TAG_TYPE_BORDER;
					break;
				case '-': // ignore formattting from here on
					tag.type = TAG_TYPE_IGNORE_FORMATTING;
					ignoreFormatting = true;
					//start = end;
					break;
				default: // command not supported, regard as normal text
					start = end;
					continue;
				}
				stack += str[start + 1];
				if (end - start > 4)
				{
					tag.data = text.utf8_substr(start + 3, end - start - 4);
				}
			}
			foundTags += tag;
			start = end;
		}
		// add closing tags where missing
		hstr result;
		int index = 0;
		int count = 0;
		foreach (FormatTag, it, foundTags)
		{
			result += text(index, (*it).start - index);
			index = (*it).start + (*it).count;
			if ((*it).type != TAG_TYPE_ESCAPE)
			{
				(*it).start -= count;
				tags += (*it);
			}
			else
			{
				--count;
				result += '[';
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

	harray<RenderLine> Renderer::removeOutOfBoundLines(grect rect, harray<RenderLine> lines)
	{
		harray<RenderLine> result;
		foreach (RenderLine, it, lines)
		{
			// zero-length rectangles should be included
			if ((*it).rect.w == 0.0f || (*it).rect.h == 0.0f || (*it).rect.intersects(rect))
			{
				result += (*it);
			}
		}
		return result;
	}
	
	harray<RenderLine> Renderer::verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float y, float lineHeight, float descender)
	{
		harray<RenderLine> result;
		int lineCount = lines.size();
		if (lines.last().terminated)
		{
			++lineCount;
		}
		// vertical correction
		switch (vertical)
		{
		case CENTER:
			y += (lineCount * lineHeight - rect.h + descender) * 0.5f;
			break;
		case BOTTOM:
			y += lineCount * lineHeight - rect.h + descender;
			break;
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
		return result;
	}
	
	harray<RenderLine> Renderer::horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float x, float lineWidth)
	{
		// horizontal correction not necessary when left aligned
		if (ALIGNMENT_IS_LEFT(horizontal))
		{
			foreach (RenderLine, it, lines)
			{
				(*it).rect.x -= x;
				foreach (RenderWord, it2, (*it).words)
				{
					(*it2).rect.x -= x;
				}
			}
			return lines;
		}
		float ox = 0.0f;
		if (horizontal != JUSTIFIED || !this->justifiedEnabled)
		{
			// horizontal correction
			foreach (RenderLine, it, lines)
			{
				switch (horizontal)
				{
				case CENTER:
				case CENTER_WRAPPED:
					ox = -x + (rect.w - (*it).rect.w) * 0.5f;
					break;
				case RIGHT:
				case RIGHT_WRAPPED:
					ox = -x + rect.w - (*it).rect.w;
					break;
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
			float width;
			float widthPerSpace;
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
								width += (*it2).rect.w;
							}
						}
						widthPerSpace = (rect.w - width) / lines[i].spaces;
						width = 0.0f;
						words.clear();
						foreach (RenderWord, it, lines[i].words)
						{
							if ((*it).spaces == 0)
							{
								(*it).rect.x += hroundf(width);
								words += (*it);
							}
							else
							{
								width += (*it).spaces * (widthPerSpace - (*it).rect.w);
							}
						}
						lines[i].words = words;
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
		return lines;
	}

	void Renderer::_initializeFormatTags(harray<FormatTag>& tags)
	{
		this->_tags = tags;
		this->_stack.clear();
		this->_currentTag = FormatTag();
		this->_nextTag = this->_tags.first();
		this->_fontName = "";
		this->_font = NULL;
		this->_texture = NULL;
		this->_characters.clear();
		this->_height = 0.0f;
		this->_lineHeight = 0.0f;
		this->_descender = 0.0f;
		this->_fontScale = 1.0f;
		this->_textScale = 1.0f;
		this->_scale = 1.0f;
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
		this->_borderSequence.color.a = (unsigned char)(this->borderColor.a * this->borderColor.a_f() * this->borderColor.a_f());
		this->_renderRect = RenderRectangle();
		this->_textColor = april::Color::White;
		this->_shadowColor = this->shadowColor;
		this->_borderColor = this->borderColor;
		this->_hex = "";
		this->_effectMode = 0;
		this->_alpha = -1;
	}

	void Renderer::_initializeLineProcessing(harray<RenderLine> lines)
	{
		this->_lines = lines;
		this->_line = RenderLine();
		this->_word = RenderWord();
	}

	void Renderer::_checkFormatTags(chstr text, int index)
	{
		while (this->_tags.size() > 0 && index >= this->_nextTag.start)
		{
			if (this->_nextTag.type == TAG_TYPE_CLOSE)
			{
				this->_currentTag = this->_stack.remove_last();
				if (this->_currentTag.type == TAG_TYPE_FONT)
				{
					this->_fontName = this->_currentTag.data;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_fontScale = this->_font->getScale();
				}
				else if (this->_currentTag.type == TAG_TYPE_SCALE)
				{
					this->_textScale = this->_currentTag.data;
				}
			}
			else if (this->_nextTag.type == TAG_TYPE_FONT)
			{
				this->_currentTag.type = TAG_TYPE_FONT;
				this->_currentTag.data = this->_fontName;
				this->_stack += this->_currentTag;
				if (this->_font == NULL) // if there is no previous font, the height values have to be obtained as well
				{
					this->_font = this->getFont(this->_nextTag.data);
					if (this->_font != NULL)
					{
						this->_height = this->_font->getHeight();
						this->_lineHeight = this->_font->getLineHeight();
						this->_descender = this->_font->getDescender();
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
				}
				else
				{
					hlog::warnf(atres::logTag, "Font '%s' does not exist!", this->_nextTag.data.c_str());
				}
			}
			else if (this->_nextTag.type == TAG_TYPE_COLOR)
			{
				this->_currentTag.type = TAG_TYPE_COLOR;
				this->_stack += this->_currentTag;
			}
			else if (this->_nextTag.type == TAG_TYPE_SCALE)
			{
				this->_currentTag.type = TAG_TYPE_SCALE;
				this->_currentTag.data = this->_textScale;
				this->_stack += this->_currentTag;
				this->_textScale = this->_nextTag.data;
			}
			else
			{
				this->_currentTag.type = TAG_TYPE_NORMAL;
				this->_stack += this->_currentTag;
			}
			this->_tags.remove_first();
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
			if (this->_nextTag.type == TAG_TYPE_CLOSE)
			{
				this->_currentTag = this->_stack.remove_last();
				switch (this->_currentTag.type)
				{
				case TAG_TYPE_FONT:
					this->_fontName = this->_currentTag.data;
					this->_font = this->getFont(this->_fontName);
					this->_characters = this->_font->getCharacters();
					this->_fontScale = this->_font->getScale();
					break;
				case TAG_TYPE_COLOR:
					this->_hex = (this->colors.has_key(this->_currentTag.data) ? this->colors[this->_currentTag.data] : this->_currentTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_textColor.set(this->_hex);
					}
					break;
				case TAG_TYPE_SCALE:
					this->_textScale = this->_currentTag.data;
					break;
				case TAG_TYPE_NORMAL:
					this->_effectMode = EFFECT_MODE_NORMAL;
					break;
				case TAG_TYPE_SHADOW:
					this->_effectMode = EFFECT_MODE_SHADOW;
					this->_hex = (this->colors.has_key(this->_currentTag.data) ? this->colors[this->_currentTag.data] : this->_currentTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_shadowColor.set(this->_hex);
					}
					break;
				case TAG_TYPE_BORDER:
					this->_effectMode = EFFECT_MODE_BORDER;
					this->_hex = (this->colors.has_key(this->_currentTag.data) ? this->colors[this->_currentTag.data] : this->_currentTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_borderColor.set(this->_hex);
					}
					break;
				}
			}
			else
			{
				switch (this->_nextTag.type)
				{
				case TAG_TYPE_FONT:
					this->_currentTag.type = TAG_TYPE_FONT;
					this->_currentTag.data = this->_fontName;
					this->_stack += this->_currentTag;
					if (this->_font == NULL)
					{
						this->_font = this->getFont(this->_nextTag.data);
						if (this->_font != NULL)
						{
							this->_height = this->_font->getHeight();
							this->_lineHeight = this->_font->getLineHeight();
							this->_descender = this->_font->getDescender();
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
					}
					else
					{
						hlog::warnf(atres::logTag, "Font '%s' does not exist!", this->_nextTag.data.c_str());
					}
					break;
				case TAG_TYPE_COLOR:
					this->_currentTag.type = TAG_TYPE_COLOR;
					this->_currentTag.data = this->_textColor.hex();
					this->_stack += this->_currentTag;
					this->_hex = (this->colors.has_key(this->_nextTag.data) ? this->colors[this->_nextTag.data] : this->_nextTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_textColor.set(this->_hex);
						this->_alpha == -1 ? this->_alpha = this->_textColor.a : this->_textColor.a = (unsigned char)(this->_alpha * this->_textColor.a_f());
					}
					else
					{
						hlog::warnf(atres::logTag, "Color '%s' does not exist!", this->_hex.c_str());
					}
					break;
				case TAG_TYPE_SCALE:
					this->_currentTag.type = TAG_TYPE_SCALE;
					this->_currentTag.data = this->_textScale;
					this->_stack += this->_currentTag;
					this->_textScale = this->_nextTag.data;
					break;
				case TAG_TYPE_NORMAL:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? TAG_TYPE_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? TAG_TYPE_SHADOW : TAG_TYPE_NORMAL));
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_NORMAL;
					break;
				case TAG_TYPE_SHADOW:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? TAG_TYPE_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? TAG_TYPE_SHADOW : TAG_TYPE_NORMAL));
					this->_currentTag.data = this->_shadowColor.hex();
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_SHADOW;
					this->_shadowColor = this->shadowColor;
					if (this->_nextTag.data != "")
					{
						this->_hex = (this->colors.has_key(this->_nextTag.data) ? this->colors[this->_nextTag.data] : this->_nextTag.data);
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
						{
							this->_shadowColor.set(this->_hex);
						}
						else
						{
							hlog::warnf(atres::logTag, "Color '%s' does not exist!", this->_hex.c_str());
						}
					}
					break;
				case TAG_TYPE_BORDER:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? TAG_TYPE_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? TAG_TYPE_SHADOW : TAG_TYPE_NORMAL));
					this->_currentTag.data = this->_borderColor.hex();
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_BORDER;
					this->_borderColor = this->borderColor;
					if (this->_nextTag.data != "")
					{
						this->_hex = (this->colors.has_key(this->_nextTag.data) ? this->colors[this->_nextTag.data] : this->_nextTag.data);
						if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
						{
							this->_borderColor.set(this->_hex);
						}
						else
						{
							hlog::warnf(atres::logTag, "Color '%s' does not exist!", this->_hex.c_str());
						}
					}
					break;
				}
			}
			this->_tags.remove_first();
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
			if (this->_font != NULL)
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
		if (this->_font != NULL)
		{
			// this additional check is required in case the texture had to be changed
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
	}

	harray<RenderWord> Renderer::createRenderWords(grect rect, chstr text, harray<FormatTag> tags)
	{
		this->_initializeFormatTags(tags);
		int actualSize = text.find_first_of('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
		else if (actualSize < text.size())
		{
			hlog::warnf(atres::logTag, "Text '%s' has \\0 character before the actual end!", text.c_str());
		}
		harray<RenderWord> result;
		RenderWord word;
		unsigned int code = 0;
		float ax = 0.0f;
		float aw = 0.0f;
		float wordX = 0.0f;
		float wordW = 0.0f;
		float addW = 0.0f;
		int start = 0;
		int i = 0;
		int chars = 0;
		int byteSize = 0;
		bool checkingSpaces = true;
		bool tooLong = false;
		harray<float> charWidths;
		word.rect.x = rect.x;
		word.rect.y = rect.y;
		word.rect.h = this->_height;
		
		while (i < actualSize) // checking all words
		{
			start = i;
			chars = 0;
			wordX = 0.0f;
			wordW = 0.0f;
			while (i < actualSize) // checking a whole word
			{
				code = text.first_unicode_char(i, &byteSize);
				this->_checkFormatTags(text, i);
				if (code == '\n')
				{
					if (i == start)
					{
						i += byteSize;
						++chars;
					}
					break;
				}
				if ((code == 0x20) != checkingSpaces)
				{
					break;
				}
				this->_character = &this->_characters[code];
				this->_scale = this->_fontScale * this->_textScale;
				if (wordX < -this->_character->bx * this->_scale)
				{
					ax = (this->_character->aw - this->_character->bx) * this->_scale;
					aw = this->_character->w * this->_scale;
				}
				else
				{
					ax = this->_character->aw * this->_scale;
					aw = (this->_character->w + this->_character->bx) * this->_scale;
				}
				addW = hmax(ax, aw);
				if (wordW + addW > rect.w) // word too long for line
				{
					if (!checkingSpaces)
					{
						tooLong = true;
					}
					break;
				}
				wordW = wordX + addW;
				wordX += ax;
				charWidths += ax;
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
							unsigned int nextCode = text.first_unicode_char(i);
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
				word.text = text(start, i - start);
				word.rect.w = wordX;
				word.start = start;
				word.count = i - start;
				word.spaces = (checkingSpaces ? i - start : 0);
				word.fullWidth = wordW;
				word.charWidths = charWidths;
				result += word;
				charWidths.clear();
			}
			else if (tooLong) // this prevents an infinite loop if not at least one character fits in the line
			{
				hlog::warn(atres::logTag, "String does not fit in rect: " + text);
				break;
			}
			tooLong = false;
			checkingSpaces = !checkingSpaces;
		}
		return result;
	}

	harray<RenderLine> Renderer::createRenderLines(grect rect, chstr text, harray<FormatTag> tags,
		Alignment horizontal, Alignment vertical, gvec2 offset, bool keepWrappedSpaces)
	{
		this->analyzeText(tags.first().data, text); // by convention, the first tag is the font name
		harray<RenderWord> words = this->createRenderWords(rect, text, tags);
		this->_initializeLineProcessing();

		bool wrapped = ALIGNMENT_IS_WRAPPED(horizontal);
		float maxWidth = 0.0f;
		float lineWidth = 0.0f;
		float x = 0.0f;
		bool nextLine = false;
		bool forcedNextLine = false;
		bool addWord = false;
		this->_line.rect.x = rect.x;
		this->_line.rect.h = this->_height;

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
			else if (lineWidth + words[i].fullWidth > rect.w && wrapped)
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
				lineWidth += words[i].rect.w;
				this->_line.words += words[i];
				this->_line.count += words[i].count;
			}
			if (nextLine)
			{
				// remove spaces at beginning and end in wrapped formatting styles
				if (wrapped && !keepWrappedSpaces)
				{
					while (this->_line.words.size() > 0 && this->_line.words.first().spaces > 0)
					{
						this->_line.words.remove_first();
					}
					while (this->_line.words.size() > 0 && this->_line.words.last().spaces > 0)
					{
						this->_line.words.remove_last();
					}
				}
				if (this->_line.words.size() > 0)
				{
					this->_line.words.last().rect.w = hmax(this->_line.words.last().rect.w, this->_line.words.last().fullWidth);
					x = this->_line.words.first().rect.x;
					foreach (RenderWord, it, this->_line.words)
					{
						this->_line.text += (*it).text;
						this->_line.spaces += (*it).spaces;
						this->_line.rect.w += (*it).rect.w;
						(*it).rect.x = x;
						x += (*it).rect.w;
					}
				}
				maxWidth = hmax(maxWidth, this->_line.rect.w);
				this->_line.rect.y = rect.y + this->_lines.size() * this->_lineHeight;
				this->_line.terminated = forcedNextLine;
				this->_lines += this->_line;
				// reset
				this->_line.text = "";
				this->_line.start = 0;
				this->_line.count = 0;
				this->_line.spaces = 0;
				this->_line.terminated = false;
				this->_line.rect.w = 0.0f;
				this->_line.words.clear();
				lineWidth = 0.0f;
			}
		}
		maxWidth = hmin(maxWidth, rect.w);
		if (this->_lines.size() > 0)
		{
			this->_lines = this->verticalCorrection(rect, vertical, this->_lines, offset.y, this->_lineHeight, this->_descender);
			this->_lines = this->removeOutOfBoundLines(rect, this->_lines);
			if (this->_lines.size() > 0)
			{
				this->_lines = this->horizontalCorrection(rect, horizontal, this->_lines, offset.x, maxWidth);
			}
		}
		return this->_lines;
	}
	
	RenderText Renderer::createRenderText(grect rect, harray<RenderLine> lines, harray<FormatTag> tags)
	{
		this->_initializeFormatTags(tags);
		this->_initializeRenderSequences();
		this->_initializeLineProcessing(lines);
		int byteSize = 0;
		float width = 0.0f;
		grect destination;
		grect area;
		grect drawRect;
		
		while (this->_lines.size() > 0)
		{
			this->_line = this->_lines.remove_first();
			foreach (RenderWord, it, this->_line.words)
			{
				width = 0.0f;
				this->_word = (*it);
				for_iter_step (i, 0, this->_word.text.size(), byteSize)
				{
					this->_code = this->_word.text.first_unicode_char(i, &byteSize);
					// checking first formatting tag changes
					this->_processFormatTags(this->_word.text, i);
					// if character exists in current font
					if (this->_characters.has_key(this->_code))
					{
						// checking the particular character
						this->_scale = this->_fontScale * this->_textScale;
						this->_character = &this->_characters[this->_code];
						area = this->_word.rect;
						area.x += hmax(0.0f, width + this->_character->bx * this->_scale);
						area.y += (this->_lineHeight - this->_height) * 0.5f;
						area.w = this->_character->w * this->_scale;
						area.h = this->_character->h * this->_scale;
						area.y += this->_lineHeight * (1.0f - this->_textScale) * 0.5f;
						drawRect = rect;
						drawRect.h += this->_character->by * this->_scale;
						if (this->_font != NULL)
						{
							this->_renderRect = this->_font->makeRenderRectangle(drawRect, area, this->_code);
							this->_renderRect.dest.y -= this->_character->by * this->_scale;
							this->_textSequence.addRenderRectangle(this->_renderRect);
							destination = this->_renderRect.dest;
							switch (this->_effectMode)
							{
							case EFFECT_MODE_SHADOW: // shadow
								this->_renderRect.dest = destination + this->shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
								this->_shadowSequence.addRenderRectangle(this->_renderRect);
								break;
							case EFFECT_MODE_BORDER: // border
								this->_renderRect.dest = destination + gvec2(-this->borderOffset, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(this->borderOffset, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(-this->borderOffset, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(this->borderOffset, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(0.0f, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(-this->borderOffset, 0.0f) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(this->borderOffset, 0.0f) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								this->_renderRect.dest = destination + gvec2(0.0f, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
								this->_borderSequence.addRenderRectangle(this->_renderRect);
								break;
							}
						}
						width += (width < -this->_character->bx * this->_scale ? (this->_character->aw - this->_character->bx) : this->_character->aw) * this->_scale;
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
		RenderText result;
		result.textSequences = this->optimizeSequences(this->_textSequences);
		result.shadowSequences = this->optimizeSequences(this->_shadowSequences);
		result.borderSequences = this->optimizeSequences(this->_borderSequences);
		return result;
	}

	harray<RenderSequence> Renderer::optimizeSequences(harray<RenderSequence>& sequences)
	{
		harray<RenderSequence> result;
		RenderSequence current;
		while (sequences.size() > 0)
		{
			current = sequences.remove_first();
			for_iter (i, 0, sequences.size())
			{
				if (current.texture == sequences[i].texture && current.color.hex(true) == sequences[i].color.hex(true))
				{
					current.vertices += sequences[i].vertices;
					sequences.remove_at(i);
					--i;
				}
			}
			result += current;
		}
		return result;
	}

/******* DRAW TEXT *****************************************************/

	void Renderer::_drawRenderText(RenderText& renderText, april::Color color)
	{
		foreach (RenderSequence, it, renderText.shadowSequences)
		{
			this->_drawRenderSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f())));
		}
		foreach (RenderSequence, it, renderText.borderSequences)
		{
			this->_drawRenderSequence((*it), april::Color((*it).color, (unsigned char)((*it).color.a * color.a_f() * color.a_f())));
		}
		foreach (RenderSequence, it, renderText.textSequences)
		{
			this->_drawRenderSequence((*it), april::Color((*it).color, color.a));
		}
	}

	void Renderer::_drawRenderSequence(RenderSequence& sequence, april::Color color)
	{
		if (sequence.vertices.size() == 0 || sequence.texture == NULL || color.a == 0)
		{
			return;
		}
		april::rendersys->setTexture(sequence.texture);
		if (sequence.texture->getFormat() == april::Image::FORMAT_ALPHA)
		{
			april::rendersys->setTextureColorMode(april::CM_ALPHA_MAP);
		}
		else
		{
			april::rendersys->setTextureColorMode(april::CM_DEFAULT);
		}
		if (color == april::Color::White)
		{
			april::rendersys->render(april::RO_TRIANGLE_LIST, &sequence.vertices[0], sequence.vertices.size());
		}
		else
		{
			april::rendersys->render(april::RO_TRIANGLE_LIST, &sequence.vertices[0], sequence.vertices.size(), color);
		}
		april::rendersys->setTextureColorMode(april::CM_DEFAULT);
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
	
	void Renderer::drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
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
			this->_cacheEntryText.value = this->createRenderText(rect, this->_lines, tags);
			this->cacheText->add(this->_cacheEntryText);
			this->cacheText->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
	}

	void Renderer::drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
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
			this->_cacheEntryText.value = this->createRenderText(rect, this->_lines, tags);
			this->cacheTextUnformatted->add(this->_cacheEntryText);
			this->cacheTextUnformatted->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
	}

	harray<RenderLine> Renderer::makeRenderLines(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
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

	harray<RenderLine> Renderer::makeRenderLinesUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
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

	harray<FormatTag> Renderer::_makeDefaultTags(april::Color color, chstr fontName, hstr& text)
	{
		harray<FormatTag> tags;
		text = this->analyzeFormatting(text, tags);
		FormatTag tag;
		tag.type = TAG_TYPE_COLOR;
		tag.data = color.hex();
		tags.push_first(tag);
		tag.type = TAG_TYPE_FONT;
		tag.data = fontName;
		tags.push_first(tag);
		return tags;
	}

	harray<FormatTag> Renderer::_makeDefaultTagsUnformatted(april::Color color, chstr fontName)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = TAG_TYPE_COLOR;
		tag.data = color.hex();
		tags.push_first(tag);
		tag.type = TAG_TYPE_FONT;
		tag.data = fontName;
		tags.push_first(tag);
		return tags;
	}

	/******* DRAW TEXT OVERLOADS *******************************************/

	void Renderer::drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		this->drawText("", rect, text, horizontal, vertical, color, offset);
	}

	void Renderer::drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		this->drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		this->drawText("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		this->drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		this->drawText("", grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawTextUnformatted(grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
	{
		this->drawTextUnformatted("", rect, text, horizontal, vertical, color, offset);
	}

	void Renderer::drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal,
		Alignment vertical, april::Color color, gvec2 offset)
	{
		this->drawTextUnformatted(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		this->drawTextUnformatted("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal,
		Alignment vertical, unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		this->drawTextUnformatted(fontName, grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		this->drawTextUnformatted("", grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
/******* MISC **********************************************************/
	
	float Renderer::getFontHeight(chstr fontName) // DEPRECATED
	{
		return this->getFont(fontName)->getHeight();
	}
	
	float Renderer::getFontLineHeight(chstr fontName) // DEPRECATED
	{
		return this->getFont(fontName)->getLineHeight();
	}
	
	float Renderer::getTextWidth(chstr fontName, chstr text)
	{
		if (text != "")
		{
			harray<FormatTag> tags;
			hstr unformattedText = this->prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				float w = 0.0f;
				harray<hstr> lines = unformattedText.split('\n', -1, true);
				foreach (hstr, it, lines)
				{
					w = hmax(w, this->getFittingLine(fontName, grect(0.0f, 0.0f, CHECK_RECT_SIZE, 1.0f), (*it), tags).rect.w);
				}
				return w;
			}
		}
		return 0.0f;
	}

	float Renderer::getTextHeight(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			harray<FormatTag> tags;
			hstr unformattedText = this->prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				harray<RenderLine> lines = this->createRenderLines(grect(0.0f, 0.0f, maxWidth, CHECK_RECT_SIZE), unformattedText, tags, LEFT_WRAPPED, TOP);
				Font* font = this->getFont(fontName);
				return (lines.size() * font->getLineHeight() + font->getDescender());
			}
		}
		return 0.0f;
	}
	
	int Renderer::getTextCount(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			harray<FormatTag> tags;
			hstr unformattedText = this->prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				return this->getFittingLine(fontName, grect(0.0f, 0.0f, maxWidth, 1.0f), unformattedText, tags).text.size();
			}
		}
		return 0;
	}
	
	float Renderer::getTextWidthUnformatted(chstr fontName, chstr text)
	{
		if (text != "")
		{
			harray<FormatTag> tags = this->prepareTags(fontName);
			float w = 0.0f;
			harray<hstr> lines = text.split('\n', -1, true);
			grect rect(0.0f, 0.0f, CHECK_RECT_SIZE, 1.0f);
			foreach (hstr, it, lines)
			{
				w = hmax(w, this->getFittingLine(fontName, rect, (*it), tags).rect.w);
			}
			return w;
		}
		return 0.0f;
	}

	float Renderer::getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			harray<FormatTag> tags = this->prepareTags(fontName);
			harray<RenderLine> lines = this->createRenderLines(grect(0.0f, 0.0f, maxWidth, CHECK_RECT_SIZE), text, tags, LEFT_WRAPPED, TOP);
			Font* font = this->getFont(fontName);
			return (lines.size() * font->getLineHeight() + font->getDescender());
		}
		return 0.0f;
	}
	
	int Renderer::getTextCountUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "" && maxWidth > 0.0f)
		{
			harray<FormatTag> tags = this->prepareTags(fontName);
			return this->getFittingLine(fontName, grect(0.0f, 0.0f, maxWidth, 1.0f), text, tags).text.size();
		}
		return 0;
	}
	
	hstr Renderer::prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags)
	{
		hstr unformattedText = this->analyzeFormatting(text, tags);
		FormatTag tag;
		tag.type = TAG_TYPE_FONT;
		tag.data = fontName;
		tags.push_first(tag);
		return unformattedText;
	}
	
	harray<FormatTag> Renderer::prepareTags(chstr fontName)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = TAG_TYPE_FONT;
		tag.data = fontName;
		tags.push_first(tag);
		return tags;
	}
	
	RenderLine Renderer::getFittingLine(chstr fontName, grect rect, chstr text, harray<FormatTag> tags)
	{
		this->_cacheEntryLine.set(text, fontName, rect.getSize());
		if (!this->cacheLine->get(this->_cacheEntryLine))
		{
			this->_cacheEntryLine.value = this->_calculateFittingLine(rect, text, tags);
			this->cacheLine->add(this->_cacheEntryLine);
			this->cacheLine->update();
		}
		return this->_cacheEntryLine.value;
	}

	RenderLine Renderer::_calculateFittingLine(grect rect, chstr text, harray<FormatTag> tags)
	{
		this->analyzeText(tags.first().data, text);
		this->_initializeFormatTags(tags);
		int actualSize = text.find_first_of('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
		else if (actualSize < text.size())
		{
			hlog::warnf(atres::logTag, "Text '%s' has \\0 character before the actual end!", text.c_str());
		}

		unsigned int code = 0;
		float ax = 0.0f;
		float aw = 0.0f;
		float lineX = 0.0f;
		float lineW = 0.0f;
		float addW = 0.0f;
		int i = 0;
		int byteSize = 0;
		RenderLine result;
		result.rect.h = this->_height;
		
		while (i < actualSize) // checking all characters
		{
			code = text.first_unicode_char(i, &byteSize);
			if (code == '\n')
			{
				break;
			}
			this->_checkFormatTags(text, i);
			this->_character = &this->_characters[code];
			this->_scale = this->_fontScale * this->_textScale;
			if (lineX < -this->_character->bx * this->_scale)
			{
				ax = (this->_character->aw - this->_character->bx) * this->_scale;
				aw = this->_character->w * this->_scale;
			}
			else
			{
				ax = this->_character->aw * this->_scale;
				aw = (this->_character->w + this->_character->bx) * this->_scale;
			}
			addW = hmax(ax, aw);
			if (lineX + addW > rect.w) // doesn't fit any more in this line
			{
				break;
			}
			lineW = lineX + addW;
			lineX += ax;
			i += byteSize;
		}
		result.text = text(0, i);
		result.rect.w = lineW;
		return result;
	}

}
