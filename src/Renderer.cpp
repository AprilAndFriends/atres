/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <math.h>
#include <stdio.h>

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "Cache.h"
#include "FontResource.h"

#define ALIGNMENT_IS_WRAPPED(formatting) ((formatting) == LEFT_WRAPPED || (formatting) == CENTER_WRAPPED || (formatting) == RIGHT_WRAPPED || (formatting) == JUSTIFIED)
#define ALIGNMENT_IS_LEFT(formatting) ((formatting) == LEFT || (formatting) == LEFT_WRAPPED)

#define CHECK_UNICODE_LINE_BREAK_CHARS(code) \
	((code) >= 128 && ( (code) == 0x2026 ||	/* ellipsis char */ \
						(code) == 0x3000 ||	/* ideographic space */ \
						(code) == 0x3001 ||	/* ideographic comma */ \
						(code) == 0x3002 ||	/* ideographic full stop/period */ \
						(code) == 0x30FC ||	/* Japanese dash char */ \
						(code) == 0x4E00 ||	/* fullwidth dash char */ \
						(code) == 0xFF01 ||	/* fullwidth exclamation mark */ \
						(code) == 0xFF0C ||	/* fullwidth comma */ \
						(code) == 0xFF1F	/* fullwidth question mark */ \
	))

#define EFFECT_MODE_NORMAL 0
#define EFFECT_MODE_SHADOW 1
#define EFFECT_MODE_BORDER 2

namespace atres
{
	Renderer* renderer;

	Renderer::Renderer()
	{
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
		this->colors["white"] = april::Color::White.hex();
		this->colors["grey"] = april::Color::Grey.hex();
		this->colors["black"] = april::Color::Black.hex();
		// init
		this->shadowOffset.set(1.0f, 1.0f);
		this->shadowColor = april::Color::Black;
		this->borderOffset = 1.0f;
		this->borderColor = april::Color::Black;
		this->globalOffsets = false;
		this->useLegacyLineBreakParsing = false;
		this->defaultFont = NULL;
		// misc init
		this->_fontResource = NULL;
		// cache
		this->cache = new Cache<CacheEntryText>();
		this->cacheUnformatted = new Cache<CacheEntryText>();
		this->cacheLines = new Cache<CacheEntryLine>();
	}

	Renderer::~Renderer()
	{
		this->destroyAllFontResources();
		delete this->cache;
		delete this->cacheUnformatted;
		delete this->cacheLines;
	}

/******* PROPERTIES ****************************************************/

	void Renderer::setShadowOffset(gvec2 value)
	{
		this->shadowOffset = value;
		this->clearCache();
	}
	
	void Renderer::setShadowColor(april::Color value)
	{
		this->shadowColor = value;
		this->clearCache();
	}
	
	void Renderer::setBorderOffset(float value)
	{
		this->borderOffset = value;
		this->clearCache();
	}
	
	void Renderer::setBorderColor(april::Color value)
	{
		this->borderColor = value;
		this->clearCache();
	}
	
	void Renderer::setDefaultFont(chstr name)
	{
		if (!this->fonts.has_key(name))
		{
			throw resource_not_exists("Font", name, "atres");
		}
		this->defaultFont = this->fonts[name];
		this->cache->clear();
	}

	bool Renderer::hasFont(chstr name)
	{
		return (name == "" && this->defaultFont != NULL || this->fonts.has_key(name));
	}
	
	void Renderer::setCacheSize(int value)
	{
		this->cache->setMaxSize(value);
		this->cacheUnformatted->setMaxSize(value);
		this->cacheLines->setMaxSize(value);
	}
	
/******* FONT **********************************************************/

	void Renderer::registerFontResource(FontResource* fontResource)
	{
		hstr name = fontResource->getName();
		hlog::write(atres::logTag, "Registering font resource: " + name);
		if (this->fonts.has_key(name))
		{
			throw resource_already_exists("font resource", name, "atres");
		}
		this->fonts[name] = fontResource;
		if (this->defaultFont == NULL)
		{
			this->defaultFont = fontResource;
		}
	}
	
	void Renderer::unregisterFontResource(FontResource* fontResource)
	{
		if (!this->fonts.has_value(fontResource))
		{
			throw resource_not_exists("font resource", fontResource->getName(), "atres");
		}
		harray<hstr> keys = this->fonts.keys();
		foreach (hstr, it, keys)
		{
			if (this->fonts[*it] == fontResource)
			{
				this->fonts.remove_key(*it);
			}
		}
		if (this->defaultFont == fontResource)
		{
			this->defaultFont = (this->fonts.size() > 0 ? this->fonts.values().first() : NULL);
		}
		this->cache->clear();
	}

	void Renderer::registerFontResourceAlias(chstr name, chstr alias)
	{
		if (this->fonts.has_key(alias))
		{
			throw resource_already_exists("font resource", alias, "atres");
		}
		FontResource* fontResource = this->getFontResource(name);
		hlog::writef(atres::logTag, "Registering font resource alias '%s' for '%s'.", alias.c_str(), fontResource->getName().c_str());
		this->fonts[alias] = fontResource;
	}
	
	void Renderer::destroyAllFontResources()
	{
		this->defaultFont = NULL;
		this->clearCache();
		FontResource* fontResource;
		harray<hstr> keys;
		while (this->fonts.size() > 0)
		{
			keys = this->fonts.keys();
			fontResource = this->fonts[keys.first()];
			foreach (hstr, it, keys)
			{
				if (this->fonts[*it] == fontResource)
				{
					this->fonts.remove_key(*it);
				}
			}
			delete fontResource;
		}
	}
	
	void Renderer::destroyFontResource(FontResource* fontResource)
	{
		this->unregisterFontResource(fontResource);
		delete fontResource;
	}

	FontResource* Renderer::getFontResource(chstr name)
	{
		if (name == "" && this->defaultFont != NULL)
		{
			this->defaultFont->setScale(1.0f);
			return this->defaultFont;
		}
		FontResource* fontResource;
		if (this->fonts.has_key(name))
		{
			fontResource = this->fonts[name];
			fontResource->setScale(1.0f);
			return fontResource;
		}
		int position = name.find(":");
		if (position < 0)
		{
			throw resource_not_exists("font resource", name, "atres");
		}
		fontResource = this->getFontResource(name(0, position));
		position++;
		fontResource->setScale((float)(name(position, name.size() - position)));
		return fontResource;
	}
	
/******* MISC **********************************************************/

	void Renderer::addColor(chstr key, chstr value)
	{
		this->colors[key.lower()] = value.upper();
	}
	
	void Renderer::_updateCache()
	{
		this->cache->update();
		this->cacheUnformatted->update();
		this->cacheLines->update();
	}
	
	void Renderer::clearCache()
	{
		if (this->cache->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d text cache entries...", this->cache->size());
			this->cache->clear();
		}
		if (this->cacheUnformatted->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d unformatted text cache entries...", this->cacheUnformatted->size());
			this->cacheUnformatted->clear();
		}
		if (this->cacheLines->size() > 0)
		{
			hlog::writef(atres::logTag, "Clearing %d line calculation cache entries...", this->cacheLines->size());
			this->cacheLines->clear();
		}
	}
	
/******* ANALYZE TEXT **************************************************/

	void Renderer::analyzeText(chstr text)
	{
		// makes sure dynamically allocated characters are loaded
		std::basic_string<unsigned int> chars = utf8_to_unicode(text);
		foreach_m (FontResource*, it, this->fonts)
		{
			for_itert (unsigned int, i, 0, chars.size())
			{
				it->second->hasChar(chars[i]);
			}
		}
	}

	hstr Renderer::analyzeFormatting(chstr text, harray<FormatTag>& tags)
	{
		const char* str = text.c_str();
		int start = 0;
		int end = 0;
		harray<char> stack;
		FormatTag tag;
		harray<FormatTag> foundTags;
		while (true)
		{
			start = text.find('[', start);
			if (start < 0)
			{
				break;
			}
			end = text.find(']', start);
			if (end < 0)
			{
				break;
			}
			end++;
			tag.data = "";
			tag.start = start;
			tag.count = end - start;
			if (tag.count == 2) // empty command
			{
				tag.type = TAG_TYPE_ESCAPE;
			}
			else if (str[start + 1] == '/') // closing command
			{
				if (stack.size() > 0 && stack.last() != str[start + 2]) // interleaving, ignore the tag
				{
					start = end;
					hlog::warnf(atres::logTag, "Closing tag that was not opened ('[/%c]' in '%s')!", str[start + 2], str);
					continue;
				}
				stack.pop_last();
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
				default: // command not supported, regard as normal text
					start = end;
					continue;
				}
				stack += str[start + 1];
				if (tag.count > 4)
				{
					tag.data = text(start + 3, tag.count - 4);
				}
			}
			foundTags += tag;
			start = end;
		}
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
				count--;
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
	
	harray<RenderLine> Renderer::verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float y, float lineHeight, float correctedHeight)
	{
		harray<RenderLine> result;
		// vertical correction
		switch (vertical)
		{
		case CENTER:
			y += ((lines.size() - 1) * lineHeight + correctedHeight - rect.h) * 0.5f;
			break;
		case BOTTOM:
			y += (lines.size() - 1) * lineHeight + correctedHeight - rect.h;
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
		if (horizontal != JUSTIFIED)
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
			for_iter (i, 0, lines.size())
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
								width += (*it).spaces * widthPerSpace;
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
		this->_fontResource = NULL;
		this->_texture = NULL;
		this->_characters.clear();
		this->_height = 0.0f;
		this->_lineHeight = 0.0f;
		this->_correctedHeight = 0.0f;
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
		this->_char = RenderChar();
		this->_line = RenderLine();
		this->_word = RenderWord();
	}

	void Renderer::_checkFormatTags(chstr text, int index)
	{
		while (this->_tags.size() > 0 && index >= this->_nextTag.start)
		{
			if (this->_nextTag.type == TAG_TYPE_CLOSE)
			{
				this->_currentTag = this->_stack.pop_last();
				if (this->_currentTag.type == TAG_TYPE_FONT)
				{
					this->_fontName = this->_currentTag.data;
					this->_fontResource = this->getFontResource(this->_fontName);
					this->_characters = this->_fontResource->getCharacters();
					this->_fontScale = this->_fontResource->getScale();
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
				try
				{
					if (this->_fontResource == NULL) // if there is no previous font, the height values have to be obtained as well
					{
						this->_fontResource = this->getFontResource(this->_nextTag.data);
						this->_height = this->_fontResource->getHeight();
						this->_lineHeight = this->_fontResource->getLineHeight();
						this->_correctedHeight = this->_fontResource->getCorrectedHeight();
					}
					else
					{
						this->_fontResource = this->getFontResource(this->_nextTag.data);
					}
					this->_fontName = this->_nextTag.data;
					this->_characters = this->_fontResource->getCharacters();
					this->_fontScale = this->_fontResource->getScale();
				}
				catch (hltypes::_resource_not_exists e)
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
			this->_tags.pop_first();
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
				this->_currentTag = this->_stack.pop_last();
				switch (this->_currentTag.type)
				{
				case TAG_TYPE_FONT:
					this->_fontName = this->_currentTag.data;
					this->_fontResource = this->getFontResource(this->_fontName);
					this->_characters = this->_fontResource->getCharacters();
					this->_fontScale = this->_fontResource->getScale();
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
					try
					{
						if (this->_fontResource == NULL)
						{
							this->_fontResource = this->getFontResource(this->_nextTag.data);
							this->_height = this->_fontResource->getHeight();
							this->_lineHeight = this->_fontResource->getLineHeight();
							this->_correctedHeight = this->_fontResource->getCorrectedHeight();
						}
						else
						{
							this->_fontResource = this->getFontResource(this->_nextTag.data);
						}
						this->_fontName = this->_nextTag.data;
						this->_characters = this->_fontResource->getCharacters();
						this->_fontScale = this->_fontResource->getScale();
					}
					catch (hltypes::_resource_not_exists e)
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
			this->_tags.pop_first();
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
			this->_texture = this->_fontResource->getTexture(this->_char.code);
			this->_checkSequenceSwitch();
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
		this->_texture = this->_fontResource->getTexture(this->_char.code);
		this->_checkSequenceSwitch();
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

	harray<RenderChar> Renderer::createRenderChars(chstr text)
	{
		int actualSize = text.find_first_of('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
		else if (actualSize < text.size())
		{
			hlog::warnf(atres::logTag, "Text '%s' has \\0 character before the actual end!", text.c_str());
		}
		harray<RenderChar> result;
		const char* str = text.c_str();
		int i = 0;
		while (i < actualSize) // checking all words
		{
			this->_char.code = utf8_to_uint(&str[i], &this->_char.byteSize);
			this->_char.text = text(i, this->_char.byteSize);
			result += this->_char;
			i += this->_char.byteSize;
		}
		return result;
	}

	harray<RenderWord> Renderer::createRenderWords(grect rect, chstr text, harray<FormatTag> tags)
	{
		this->_initializeFormatTags(tags);
		harray<RenderChar> chars = this->createRenderChars(text);
		harray<RenderWord> result;
		RenderWord word;
		float ax = 0.0f;
		float aw = 0.0f;
		float wordX = 0.0f;
		float wordW = 0.0f;
		float addW = 0.0f;
		int iStart = 0;
		int i = 0;
		int jStart = 0;
		int j = 0;
		bool checkingSpaces = true;
		bool tooLong = false;
		word.rect.x = rect.x;
		word.rect.y = rect.y;
		word.rect.h = this->_height;
		
		while (j < chars.size()) // checking all words
		{
			iStart = i;
			jStart = j;
			wordX = 0.0f;
			wordW = 0.0f;
			while (j < chars.size()) // checking a whole word
			{
				this->_char = chars[j];
				if (this->_char.code == '\n')
				{
					if (i == iStart)
					{
						i += this->_char.byteSize;
						j++;
					}
					break;
				}
				if ((this->_char.code == 0x20) != checkingSpaces)
				{
					break;
				}
				this->_checkFormatTags(text, i);
				this->_character = &this->_characters[this->_char.code];
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
				i += this->_char.byteSize;
				j++;
				if (!checkingSpaces)
				{
					if (!this->useLegacyLineBreakParsing)
					{
						if (j - jStart >= 2 && CHECK_UNICODE_LINE_BREAK_CHARS(this->_char.code))
						{
							break;
						}
						if (j < chars.size() - 1)
						{
							RenderChar nextChar = chars[j + 1];
							if (CHECK_UNICODE_LINE_BREAK_CHARS(nextChar.code))
							{
								break;
							}
						}
					}
					else if (CHECK_UNICODE_LINE_BREAK_CHARS(this->_char.code))
					{
						break;
					}
				}
			}
			if (i > iStart)
			{
				word.text = text(iStart, i - iStart);
				word.chars = chars(jStart, j - jStart);
				word.rect.w = wordX;
				word.start = iStart;
				word.spaces = (checkingSpaces ? i - iStart : 0);
				word.fullWidth = wordW;
				result += word;
			}
			else if (tooLong) // this prevents an infinite loop if not at least one character fits in the line
			{
				break;
			}
			tooLong = false;
			checkingSpaces = !checkingSpaces;
		}
		return result;
	}

	harray<RenderLine> Renderer::createRenderLines(grect rect, chstr text, harray<FormatTag> tags,
		Alignment horizontal, Alignment vertical, gvec2 offset)
	{
		this->analyzeText(text);
		harray<RenderWord> words = this->createRenderWords(rect, text, tags);
		this->_initializeLineProcessing();

		bool wrapped = ALIGNMENT_IS_WRAPPED(horizontal);
		bool left = ALIGNMENT_IS_LEFT(horizontal);
		float maxWidth = 0.0f;
		float lineWidth = 0.0f;
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
					i--;
				}
				// else the whole word is the only one in the line and doesn't fit, so just chop it off
				nextLine = true;
			}
			if (addWord)
			{
				words[i].rect.y += this->_lines.size() * this->_lineHeight;
				lineWidth += words[i].rect.w;
				this->_line.words += words[i];
			}
			if (nextLine)
			{
				// remove spaces at beginning and end in wrapped formatting styles
				if (wrapped)
				{
					while (this->_line.words.size() > 0 && this->_line.words.first().spaces > 0)
					{
						this->_line.words.pop_first();
					}
					while (this->_line.words.size() > 0 && this->_line.words.last().spaces > 0)
					{
						this->_line.words.pop_last();
					}
				}
				if (this->_line.words.size() > 0)
				{
					this->_line.words.last().rect.w = hmax(this->_line.words.last().rect.w, this->_line.words.last().fullWidth);
					foreach (RenderWord, it, this->_line.words)
					{
						this->_line.text += (*it).text;
						this->_line.spaces += (*it).spaces;
						this->_line.rect.w += (*it).rect.w;
					}
				}
				maxWidth = hmax(maxWidth, this->_line.rect.w);
				this->_line.rect.y = rect.y + this->_lines.size() * this->_lineHeight;
				this->_line.terminated = forcedNextLine;
				if (left && !wrapped && this->_line.rect.w > rect.w)
				{
					this->_line.rect.w = rect.w;
					this->_lines += this->_line;
					break;
				}
				this->_lines += this->_line;
				this->_line.text = "";
				this->_line.spaces = 0;
				this->_line.rect.w = 0.0f;
				this->_line.words.clear();
				lineWidth = 0.0f;
			}
		}
		maxWidth = hmin(maxWidth, rect.w);
		if (this->_lines.size() > 0)
		{
			this->_lines.last().terminated = true; // last line is regarded as terminated with \n
			this->_lines = this->verticalCorrection(rect, vertical, this->_lines, offset.y, this->_lineHeight, this->_correctedHeight);
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
		float width = 0.0f;
		grect destination;
		grect area;
		grect drawRect;
		int i = 0;
		
		while (this->_lines.size() > 0)
		{
			this->_line = this->_lines.pop_first();
			width = 0.0f;
			foreach (RenderWord, it, this->_line.words)
			{
				this->_word = (*it);
				i = 0;
				for_iter (j, 0, this->_word.chars.size())
				{
					this->_char = this->_word.chars[j];
					// checking first formatting tag changes
					this->_processFormatTags(this->_word.text, i);
					// if character exists in current font
					if (this->_characters.has_key(this->_char.code))
					{
						// checking the particular character
						this->_scale = this->_fontScale * this->_textScale;
						this->_character = &this->_characters[this->_char.code];
						area = this->_word.rect;
						area.x += hmax(0.0f, width + this->_character->bx * this->_scale);
						area.y += (this->_lineHeight - this->_height) * 0.5f;
						area.w = this->_character->w * this->_scale;
						area.h = this->_character->h * this->_scale;
						area.y += area.h * (1.0f - this->_textScale) * 0.5f;
						drawRect = rect;
						drawRect.h += this->_character->by * this->_scale;
						this->_renderRect = this->_fontResource->makeRenderRectangle(drawRect, area, this->_char.code);
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
						width += (width < -this->_character->bx * this->_scale ? (this->_character->aw - this->_character->bx) : this->_character->aw) * this->_scale;
					}
					i += this->_char.byteSize;
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
			current = sequences.pop_first();
			for_iter (i, 0, sequences.size())
			{
				if (current.texture == sequences[i].texture && current.color.hex(true) == sequences[i].color.hex(true))
				{
					current.vertices += sequences[i].vertices;
					sequences.remove_at(i);
					i--;
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
		if (sequence.texture->getFormat() == april::Texture::FORMAT_ALPHA)
		{
			april::rendersys->setTextureColorMode(april::ALPHA_MAP);
		}
		else
		{
			april::rendersys->setTextureColorMode(april::NORMAL);
		}
		if (color == april::Color::White)
		{
			april::rendersys->render(april::TriangleList, &sequence.vertices[0], sequence.vertices.size());
		}
		else
		{
			april::rendersys->render(april::TriangleList, &sequence.vertices[0], sequence.vertices.size(), color);
		}
		april::rendersys->setTextureColorMode(april::NORMAL);
	}

	bool Renderer::_checkTextures()
	{
		foreach (RenderSequence, it, this->_cacheEntryText.value.textSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cache
				return false;
			}
		}
		foreach (RenderSequence, it, this->_cacheEntryText.value.shadowSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cache
				return false;
			}
		}
		foreach (RenderSequence, it, this->_cacheEntryText.value.borderSequences)
		{
			if (!(*it).texture->isLoaded())
			{
				this->clearCache(); // font textures were deleted somewhere for some reason (e.g. Android's onPause), clear the cache
				return false;
			}
		}
		return true;
	}
	
	void Renderer::drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
	{
		this->_cacheEntryText.set(text, fontName, rect, horizontal, vertical, color, offset);
		bool found = this->cache->get(this->_cacheEntryText);
		if (found)
		{
			found = this->_checkTextures();
		}
		if (!found)
		{
			harray<FormatTag> tags;
			hstr unformattedText = this->analyzeFormatting(text, tags);
			FormatTag tag;
			tag.type = TAG_TYPE_COLOR;
			tag.data = color.hex();
			tags.push_front(tag);
			tag.type = TAG_TYPE_FONT;
			tag.data = fontName;
			tags.push_front(tag);
			this->_lines = this->createRenderLines(rect, unformattedText, tags, horizontal, vertical, offset);
			this->_cacheEntryText.value = this->createRenderText(rect, this->_lines, tags);
			this->cache->add(this->_cacheEntryText);
			this->cache->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
	}

	void Renderer::drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		this->_cacheEntryText.set(text, fontName, rect, horizontal, vertical, april::Color(color, 255), offset);
		bool found = this->cacheUnformatted->get(this->_cacheEntryText);
		if (found)
		{
			found = this->_checkTextures();
		}
		if (!found)
		{
			harray<FormatTag> tags;
			FormatTag tag;
			tag.type = TAG_TYPE_COLOR;
			tag.data = color.hex();
			tags.push_front(tag);
			tag.type = TAG_TYPE_FONT;
			tag.data = fontName;
			tags.push_front(tag);
			this->_lines = this->createRenderLines(rect, text, tags, horizontal, vertical, offset);
			this->_cacheEntryText.value = this->createRenderText(rect, this->_lines, tags);
			this->cacheUnformatted->add(this->_cacheEntryText);
			this->cacheUnformatted->update();
		}
		this->_drawRenderText(this->_cacheEntryText.value, color);
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
	
	float Renderer::getFontHeight(chstr fontName)
	{
		return this->getFontResource(fontName)->getHeight();
	}
	
	float Renderer::getFontLineHeight(chstr fontName)
	{
		return this->getFontResource(fontName)->getLineHeight();
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
					w = hmax(w, this->getFittingLine(fontName, grect(0.0f, 0.0f, 100000.0f, 1.0f), (*it), tags).rect.w);
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
				harray<RenderLine> lines = this->createRenderLines(grect(0.0f, 0.0f, maxWidth, 100000.0f), unformattedText, tags, LEFT_WRAPPED, TOP);
				FontResource* font = this->getFontResource(fontName);
				return ((lines.size() - 1) * font->getLineHeight() + font->getCorrectedHeight());
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
			foreach (hstr, it, lines)
			{
				w = hmax(w, this->getFittingLine(fontName, grect(0.0f, 0.0f, 100000.0f, 1.0f), (*it), tags).rect.w);
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
			harray<RenderLine> lines = this->createRenderLines(grect(0.0f, 0.0f, maxWidth, 100000.0f), text, tags, LEFT_WRAPPED, TOP);
			FontResource* font = this->getFontResource(fontName);
			return ((lines.size() - 1) * font->getLineHeight() + font->getCorrectedHeight());
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
		tags.push_front(tag);
		return unformattedText;
	}
	
	harray<FormatTag> Renderer::prepareTags(chstr fontName)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = TAG_TYPE_FONT;
		tag.data = fontName;
		tags.push_front(tag);
		return tags;
	}
	
	RenderLine Renderer::getFittingLine(chstr fontName, grect rect, chstr text, harray<FormatTag> tags)
	{
		this->_cacheEntryLine.set(text, fontName, rect.getSize());
		if (!this->cacheLines->get(this->_cacheEntryLine))
		{
			this->_cacheEntryLine.value = this->_calculateFittingLine(rect, text, tags);
			this->cacheLines->add(this->_cacheEntryLine);
			this->cacheLines->update();
		}
		return this->_cacheEntryLine.value;
	}

	RenderLine Renderer::_calculateFittingLine(grect rect, chstr text, harray<FormatTag> tags)
	{
		this->analyzeText(text);
		this->_initializeFormatTags(tags);
		harray<RenderChar> chars = this->createRenderChars(text);
		int actualSize = text.find_first_of('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
		else if (actualSize < text.size())
		{
			hlog::warnf(atres::logTag, "Text '%s' has \\0 character before the actual end!", text.c_str());
		}

		float ax = 0.0f;
		float aw = 0.0f;
		float lineX = 0.0f;
		float lineW = 0.0f;
		float addW = 0.0f;
		int i = 0;
		RenderLine result;
		result.rect.h = this->_height;
		
		while (i < actualSize) // checking all characters
		{
			this->_char = chars.first();
			if (this->_char.code == '\n')
			{
				break;
			}
			this->_checkFormatTags(text, i);
			this->_character = &this->_characters[this->_char.code];
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
			i += this->_char.byteSize;
			chars.pop_first();
		}
		result.text = text(0, i);
		result.rect.w = lineW;
		return result;
	}

}
