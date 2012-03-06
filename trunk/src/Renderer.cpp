/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.41
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
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "Cache.h"
#include "FontResource.h"

#define ALIGNMENT_IS_WRAPPED(formatting) ((formatting) == LEFT_WRAPPED || (formatting) == CENTER_WRAPPED || (formatting) == RIGHT_WRAPPED || (formatting) == JUSTIFIED)
#define ALIGNMENT_IS_LEFT(formatting) ((formatting) == LEFT || (formatting) == LEFT_WRAPPED)

#define CHECK_UNICODE_LINE_BREAK_CHARS(code) \
	((code) >= 128 && (((code) >= 0x3000 && (code) <= 0x3002) ||	/* ideographic chars like the chinese dot */ \
						(code) == 0xFF0C ||							/* fullwith comma */ \
						(code) == 0x4E00 ||							/* fullwidth '-' char */ \
						(code) == 0x30FC							/* japanese '-' */ \
	))

#define EFFECT_MODE_NORMAL 0
#define EFFECT_MODE_SHADOW 1
#define EFFECT_MODE_BORDER 2
#define BUFFER_MAX_SIZE BUFFER_MAX_CHARACTERS * 6

namespace atres
{
	Renderer* renderer;

	Cache<CacheKeySequence, harray<RenderSequence> > cache;
	Cache<CacheKeySequence, harray<RenderSequence> > cacheUnformatted;
	Cache<CacheKeyLine, RenderLine> cacheLines;
	
	april::TexturedVertex vertices[BUFFER_MAX_SIZE];

	Renderer::Renderer()
	{
		this->colors["red"] = APRIL_COLOR_RED.hex();
		this->colors["green"] = APRIL_COLOR_GREEN.hex();
		this->colors["blue"] = APRIL_COLOR_BLUE.hex();
		this->colors["yellow"] = APRIL_COLOR_YELLOW.hex();
		this->colors["mangenta"] = APRIL_COLOR_MANGENTA.hex();
		this->colors["cyan"] = APRIL_COLOR_CYAN.hex();
		this->colors["orange"] = APRIL_COLOR_ORANGE.hex();
		this->colors["pink"] = APRIL_COLOR_PINK.hex();
		this->colors["teal"] = APRIL_COLOR_TEAL.hex();
		this->colors["neon"] = APRIL_COLOR_NEON.hex();
		this->colors["purple"] = APRIL_COLOR_PURPLE.hex();
		this->colors["aqua"] = APRIL_COLOR_AQUA.hex();
		this->colors["white"] = APRIL_COLOR_WHITE.hex();
		this->colors["grey"] = APRIL_COLOR_GREY.hex();
		this->colors["black"] = APRIL_COLOR_BLACK.hex();
		// init
		this->shadowOffset.set(1.0f, 1.0f);
		this->shadowColor = APRIL_COLOR_BLACK;
		this->borderOffset = 1.0f;
		this->borderColor = APRIL_COLOR_BLACK;
		this->globalOffsets = false;
		this->defaultFont = NULL;
		// misc init
		this->_fontResource = NULL;
	}

	Renderer::~Renderer()
	{
		foreach_m (FontResource*, it, this->fonts)
		{
			delete it->second;
		}
	}

/******* PROPERTIES ****************************************************/

	void Renderer::setShadowOffset(gvec2 value)
	{
		this->shadowOffset = value;
		this->_clearCache();
	}
	
	void Renderer::setShadowColor(april::Color value)
	{
		this->shadowColor = value;
		this->_clearCache();
	}
	
	void Renderer::setBorderOffset(float value)
	{
		this->borderOffset = value;
		this->_clearCache();
	}
	
	void Renderer::setBorderColor(april::Color value)
	{
		this->borderColor = value;
		this->_clearCache();
	}
	
	void Renderer::setDefaultFont(chstr name)
	{
		if (!this->fonts.has_key(name))
		{
			throw resource_error("Font", name, "atres");
		}
		this->defaultFont = this->fonts[name];
		cache.clear();
	}

    bool Renderer::hasFont(chstr name)
    {
		return this->fonts.has_key(name);
    }
	
	void Renderer::setCacheSize(int value)
	{
		cache.setMaxSize(value);
		cacheUnformatted.setMaxSize(value);
		cacheLines.setMaxSize(value);
	}
	
/******* FONT **********************************************************/

    void Renderer::registerFontResource(FontResource* fontResource)
    {
		atres::log(hsprintf("registering font resource %s", fontResource->getName().c_str()));
        this->fonts[fontResource->getName()] = fontResource;
		if (this->defaultFont == NULL)
		{
			this->defaultFont = fontResource;
		}
    }
    
	void Renderer::unregisterFontResource(FontResource* resource)
	{
		this->fonts.remove_key(resource->getName());
		if (this->defaultFont == resource)
		{
			this->defaultFont = NULL;
		}
	}

	void Renderer::destroyFontResource(FontResource* resource)
	{
		this->unregisterFontResource(resource);
		delete resource;
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
			throw resource_error("Font", name, "atres");
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
		cache.update();
		cacheUnformatted.update();
		cacheLines.update();
	}
	
	void Renderer::_clearCache()
	{
		cache.clear();
		cacheUnformatted.clear();
		cacheLines.clear();
	}
	
/******* ANALYZE TEXT **************************************************/

	void Renderer::analyzeText(chstr text)
	{
		// makes sure dynamically allocated characters are loaded
		int size;
		unsigned int* chars = utf8_to_unicode(text, &size);
		foreach_m (FontResource*, it, this->fonts)
		{
			for (int i = 0; i < size; i++)
			{
				it->second->hasChar(chars[i]);
			}
		}
		delete [] chars;
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
				tag.type = ESCAPE;
			}
			else if (str[start + 1] == '/') // closing command
			{
				if (stack.size() > 0 && stack.last() != str[start + 2]) // interleaving, ignore the tag
				{
					start = end;
#ifdef _DEBUG
					atres::log(hsprintf("Warning: closing tag that was not opened (\"[/%c]\" in \"%s\")", str[start + 2], str));
#endif
					continue;
				}
				stack.pop_last();
				tag.type = CLOSE;
			}
			else // opening new tag
			{
				switch (str[start + 1])
				{
				case 'n':
					tag.type = FORMAT_NORMAL;
					break;
				case 's':
					tag.type = FORMAT_SHADOW;
					break;
				case 'b':
					tag.type = FORMAT_BORDER;
					break;
				case 'c':
					tag.type = FORMAT_COLOR;
					break;
				case 'f':
					tag.type = FORMAT_FONT;
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
			if ((*it).type != ESCAPE)
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

	harray<RenderLine> Renderer::verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float y, float lineHeight, float correctedHeight)
	{
		harray<RenderLine> result;
		// vertical correction
		switch (vertical)
		{
		case CENTER:
			y += ((lines.size() - 1) * lineHeight + correctedHeight - rect.h) / 2;
			break;
		case BOTTOM:
			y += (lines.size() - 1) * lineHeight + correctedHeight - rect.h;
			break;
		}
		// remove lines that cannot be seen anyway
		foreach (RenderLine, it, lines)
		{
			(*it).rect.y -= y;
			if ((*it).rect.intersects(rect))
			{
				foreach (RenderWord, it2, (*it).words)
				{
					(*it2).rect.y -= y;
				}
				result += (*it);
			}
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
					ox = -x + (rect.w - (*it).rect.w) / 2;
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
			for (int i = 0; i < lines.size(); i++)
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
								(*it).rect.x = hroundf(width);
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
						ox = -x + (rect.w - lines[i].rect.w) / 2;
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
		this->_characters.clear();
		this->_height = 0.0f;
		this->_lineHeight = 0.0f;
		this->_correctedHeight = 0.0f;
		this->_scale = 1.0f;
	}

	void Renderer::_initializeRenderSequences()
	{
		this->_sequences.clear();
		this->_sequence = RenderSequence();
		this->_shadowSequences.clear();
		this->_shadowSequence = RenderSequence();
		this->_shadowSequence.color = this->shadowColor;
		this->_borderSequences.clear();
		this->_borderSequence = RenderSequence();
		this->_borderSequence.color = this->borderColor;
		this->_borderSequence.color.a = (unsigned char)(this->borderColor.a * this->borderColor.a_f() * this->borderColor.a_f());
		this->_renderRect = RenderRectangle();
		this->_color = APRIL_COLOR_WHITE;
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
			if (this->_nextTag.type == CLOSE)
			{
				this->_currentTag = this->_stack.pop_last();
				if (this->_currentTag.type == FORMAT_FONT)
				{
					this->_fontName = this->_currentTag.data;
					this->_fontResource = this->getFontResource(this->_fontName);
					this->_characters = this->_fontResource->getCharacters();
					this->_scale = this->_fontResource->getScale();
				}
			}
			else if (this->_nextTag.type == FORMAT_FONT)
			{
				this->_currentTag.type = FORMAT_FONT;
				this->_currentTag.data = this->_fontName;
				this->_stack += this->_currentTag;
				try
				{
					if (this->_fontResource == NULL) // if there is no previous font, lineHeight has to be obtained as well
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
					this->_scale = this->_fontResource->getScale();
				}
				catch (hltypes::_resource_error e)
				{
#ifdef _DEBUG
					atres::log(hsprintf("Warning: font \"%s\" does not exist", this->_nextTag.data.c_str()));
#endif
				}
			}
			else
			{
				this->_currentTag.type = FORMAT_NORMAL;
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
			if (this->_nextTag.type == CLOSE)
			{
				this->_currentTag = this->_stack.pop_last();
				switch (this->_currentTag.type)
				{
				case FORMAT_FONT:
					this->_fontName = this->_currentTag.data;
					this->_fontResource = this->getFontResource(this->_fontName);
					this->_characters = this->_fontResource->getCharacters();
					this->_scale = this->_fontResource->getScale();
					break;
				case FORMAT_COLOR:
					this->_hex = (this->colors.has_key(this->_currentTag.data) ? this->colors[this->_currentTag.data] : this->_currentTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_color.set(this->_hex);
					}
					break;
				case FORMAT_NORMAL:
					this->_effectMode = EFFECT_MODE_NORMAL;
					break;
				case FORMAT_SHADOW:
					this->_effectMode = EFFECT_MODE_SHADOW;
					break;
				case FORMAT_BORDER:
					this->_effectMode = EFFECT_MODE_BORDER;
					break;
				}
			}
			else
			{
				switch (this->_nextTag.type)
				{
				case FORMAT_FONT:
					this->_currentTag.type = FORMAT_FONT;
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
						this->_scale = this->_fontResource->getScale();
					}
					catch (hltypes::_resource_error e)
					{
#ifdef _DEBUG
						atres::log(hsprintf("Warning: font \"%s\" does not exist", this->_nextTag.data.c_str()));
#endif
					}
					break;
				case FORMAT_COLOR:
					this->_currentTag.type = FORMAT_COLOR;
					this->_currentTag.data = this->_color.hex();
					this->_stack += this->_currentTag;
					this->_hex = (this->colors.has_key(this->_nextTag.data) ? this->colors[this->_nextTag.data] : this->_nextTag.data);
					if ((this->_hex.size() == 6 || this->_hex.size() == 8) && this->_hex.is_hex())
					{
						this->_color.set(this->_hex);
						this->_alpha == -1 ? this->_alpha = this->_color.a : this->_color.a = (unsigned char)(this->_alpha * this->_color.a_f());
					}
#ifdef _DEBUG
					else
					{
						atres::log(hsprintf("Warning: color \"%s\" does not exist", this->_hex.c_str()));
					}
#endif
					break;
				case FORMAT_NORMAL:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FORMAT_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? FORMAT_SHADOW : FORMAT_NORMAL));
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_NORMAL;
					break;
				case FORMAT_SHADOW:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FORMAT_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? FORMAT_SHADOW : FORMAT_NORMAL));
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_SHADOW;
					break;
				case FORMAT_BORDER:
					this->_currentTag.type = (this->_effectMode == EFFECT_MODE_BORDER ? FORMAT_BORDER : (this->_effectMode == EFFECT_MODE_SHADOW ? FORMAT_SHADOW : FORMAT_NORMAL));
					this->_stack += this->_currentTag;
					this->_effectMode = EFFECT_MODE_BORDER;
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
			this->_colorChanged = (this->_sequence.color != this->_color);
			this->_texture = this->_fontResource->getTexture(this->_code);
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
		this->_colorChanged = false;
		this->_texture = this->_fontResource->getTexture(this->_code);
		this->_checkSequenceSwitch();
	}

	void Renderer::_checkSequenceSwitch()
	{
		if (this->_sequence.texture != this->_texture || this->_colorChanged)
		{
			if (this->_sequence.rectangles.size() > 0)
			{
				this->_sequences += this->_sequence;
				this->_sequence.rectangles.clear();
			}
			this->_sequence.texture = this->_texture;
			this->_sequence.color = this->_color;
		}
		if (this->_shadowSequence.texture != this->_texture || this->_colorChanged)
		{
			if (this->_shadowSequence.rectangles.size() > 0)
			{
				this->_shadowSequences += this->_shadowSequence;
				this->_shadowSequence.rectangles.clear();
			}
			this->_shadowSequence.texture = this->_texture;
			this->_shadowSequence.color = this->shadowColor;
			this->_shadowSequence.color.a = (unsigned char)(this->_shadowSequence.color.a * this->_color.a_f());
		}
		if (this->_borderSequence.texture != this->_texture || this->_colorChanged)
		{
			if (this->_borderSequence.rectangles.size() > 0)
			{
				this->_borderSequences += this->_borderSequence;
				this->_borderSequence.rectangles.clear();
			}
			this->_borderSequence.texture = this->_texture;
			this->_borderSequence.color = this->borderColor;
			this->_borderSequence.color.a = (unsigned char)(this->_borderSequence.color.a * this->_color.a_f() * this->_color.a_f());
		}
	}

	harray<RenderWord> Renderer::createRenderWords(grect rect, chstr text, harray<FormatTag> tags, bool limitWidth)
	{
		this->_initializeFormatTags(tags);
		int actualSize = text.find_first_of('\0');
		if (actualSize < 0)
		{
			actualSize = text.size();
		}
#ifdef _DEBUG
		else if (actualSize < text.size())
		{
			atres::log(hsprintf("Warning: Text \"%s\" has \\0 character before the actual end", text.c_str()));
		}
#endif
		harray<RenderWord> result;
		RenderWord word;

		const char* str = text.c_str();

		unsigned int code = 0;
		float ax = 0.0f;
		float aw = 0.0f;
		float wordX = 0.0f;
		float wordW = 0.0f;
		float addW = 0.0f;
		int start = 0;
		int i = 0;
		int byteLength = 0;
		bool checkingSpaces = true;
		word.rect.h = this->_height;
		
		while (i < actualSize) // checking all words
		{
			start = i;
			wordX = 0.0f;
			wordW = 0.0f;
			while (i < actualSize) // checking a whole word
			{
				code = utf8_to_uint(&str[i], &byteLength);
				if (code == '\n')
				{
					if (i == start)
					{
						i += byteLength;
					}
					break;
				}
				if ((code == ' ') != checkingSpaces)
				{
					break;
				}
				this->_checkFormatTags(text, i);
				this->_character = &this->_characters[code];
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
				if (!limitWidth && wordW + addW > rect.w) // word too long for line
				{
					break;
				}
				wordW = wordX + addW;
				wordX += ax;
				if (limitWidth && wordW > rect.w)
				{
					break;
				}
				i += byteLength;
				if (!checkingSpaces && CHECK_UNICODE_LINE_BREAK_CHARS(code))
				{
					break;
				}
			}
			if (i > start)
			{
				word.text = text(start, i - start);
				word.rect.w = wordX;
				word.start = start;
				word.spaces = (checkingSpaces ? i - start : 0);
				word.fullWidth = wordW;
				result += word;
			}
			if (limitWidth && wordW > rect.w) // makes sure to prevent an infinite loop
			{
				break;
			}
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
		this->_line.rect.h = this->_height;
		for (int i = 0; i < words.size(); i++)
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
				words[i].rect.y = this->_lines.size() * this->_lineHeight;
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
				this->_line.rect.y = this->_lines.size() * this->_lineHeight;
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
			if (this->_lines.size() > 0)
			{
				this->_lines = this->horizontalCorrection(rect, horizontal, this->_lines, offset.x, maxWidth);
			}
		}
		return this->_lines;
	}
	
	harray<RenderSequence> Renderer::createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags)
	{
		this->_initializeFormatTags(tags);
		this->_initializeRenderSequences();
		this->_initializeLineProcessing(lines);
		int byteLength = 0;
		float width = 0.0f;
		grect destination;
		grect area;
		
		while (this->_lines.size() > 0)
		{
			this->_line = this->_lines.pop_first();
			width = 0.0f;
			foreach (RenderWord, it, this->_line.words)
			{
				this->_word = (*it);
				for (int i = 0; i < this->_word.text.size(); i += byteLength)
				{
					this->_code = utf8_to_uint(&this->_word.text[i], &byteLength);
					// checking first formatting tag changes
					this->_processFormatTags(this->_word.text, i);
					// checking the particular character
					this->_character = &this->_characters[this->_code];
					area = this->_word.rect;
					area.x += hmax(0.0f, width + this->_character->bx * this->_scale);
					area.w = this->_character->w * this->_scale;
					area.h = this->_height;
					area.y += (this->_lineHeight - this->_height) / 2;
					this->_renderRect = this->_fontResource->makeRenderRectangle(rect, area, this->_code);
					this->_sequence.rectangles += this->_renderRect;
					destination = this->_renderRect.dest;
					switch (this->_effectMode)
					{
					case EFFECT_MODE_SHADOW: // shadow
						this->_renderRect.dest = destination + this->shadowOffset * (this->globalOffsets ? 1.0f : this->_scale);
						this->_shadowSequence.rectangles += this->_renderRect;
						break;
					case EFFECT_MODE_BORDER: // border
						this->_renderRect.dest = destination + gvec2(-this->borderOffset, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(this->borderOffset, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(-this->borderOffset, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(this->borderOffset, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(0.0f, -this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(-this->borderOffset, 0.0f) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(this->borderOffset, 0.0f) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						this->_renderRect.dest = destination + gvec2(0.0f, this->borderOffset) * (this->globalOffsets ? 1.0f : this->_scale);
						this->_borderSequence.rectangles += this->_renderRect;
						break;
					}
					width += (width < -this->_character->bx * this->_scale ? (this->_character->aw - this->_character->bx) : this->_character->aw) * this->_scale;
				}
			}
		}
		if (this->_sequence.rectangles.size() > 0)
		{
			this->_sequences += this->_sequence;
		}
		if (this->_shadowSequence.rectangles.size() > 0)
		{
			this->_shadowSequences += this->_shadowSequence;
		}
		if (this->_borderSequence.rectangles.size() > 0)
		{
			this->_borderSequences += this->_borderSequence;
		}
		this->_borderSequences = this->optimizeSequences(this->_borderSequences);
		this->_shadowSequences = this->optimizeSequences(this->_shadowSequences);
		this->_sequences = this->_shadowSequences + this->_borderSequences + this->_sequences;
		return this->optimizeSequences(this->_sequences);
	}

	harray<RenderSequence> Renderer::optimizeSequences(harray<RenderSequence>& sequences)
	{
		harray<RenderSequence> result;
		RenderSequence current;
		int i = 0;
		while (sequences.size() > 0)
		{
			current = sequences.pop_first();
			for (i = 0; i < sequences.size(); i++)
			{
				if (current.texture == sequences[i].texture && current.color == sequences[i].color)
				{
					current.rectangles += sequences[i].rectangles;
					sequences.remove_at(i);
					i--;
				}
			}
			result += current;
		}
		return result;
	}
	
/******* DRAW TEXT *****************************************************/

	void Renderer::_drawRenderSequence(RenderSequence& sequence, gvec2 offset)
	{
		if (sequence.rectangles.size() == 0 || sequence.texture == NULL)
		{
			return;
		}
		this->_tw = 1.0f / sequence.texture->getWidth();
		this->_th = 1.0f / sequence.texture->getHeight();
		this->_rectangles.clear();
		this->_i = 0;
		this->_j = 0;
		april::rendersys->setTexture(sequence.texture);
		while (this->_j < sequence.rectangles.size())
		{
			this->_i = 0;
			this->_rectangles = sequence.rectangles(this->_j, hmin(BUFFER_MAX_CHARACTERS, sequence.rectangles.size() - this->_j));
			this->_j += BUFFER_MAX_CHARACTERS;
			foreach (RenderRectangle, it, this->_rectangles)
			{
				vertices[this->_i].x = (*it).dest.x + offset.x;					vertices[this->_i].y = (*it).dest.y + offset.y;					vertices[this->_i].u = (*it).src.x * this->_tw;					vertices[this->_i].v = (*it).src.y * this->_th;					this->_i++;
				vertices[this->_i].x = (*it).dest.x + offset.x + (*it).dest.w;	vertices[this->_i].y = (*it).dest.y + offset.y;					vertices[this->_i].u = ((*it).src.x + (*it).src.w) * this->_tw;	vertices[this->_i].v = (*it).src.y * this->_th;					this->_i++;
				vertices[this->_i].x = (*it).dest.x + offset.x;					vertices[this->_i].y = (*it).dest.y + offset.y + (*it).dest.h;	vertices[this->_i].u = (*it).src.x * this->_tw;					vertices[this->_i].v = ((*it).src.y + (*it).src.h) * this->_th;	this->_i++;
				vertices[this->_i].x = (*it).dest.x + offset.x + (*it).dest.w;	vertices[this->_i].y = (*it).dest.y + offset.y;					vertices[this->_i].u = ((*it).src.x + (*it).src.w) * this->_tw;	vertices[this->_i].v = (*it).src.y * this->_th;					this->_i++;
				vertices[this->_i].x = (*it).dest.x + offset.x + (*it).dest.w;	vertices[this->_i].y = (*it).dest.y + offset.y + (*it).dest.h;	vertices[this->_i].u = ((*it).src.x + (*it).src.w) * this->_tw;	vertices[this->_i].v = ((*it).src.y + (*it).src.h) * this->_th;	this->_i++;
				vertices[this->_i].x = (*it).dest.x + offset.x;					vertices[this->_i].y = (*it).dest.y + offset.y + (*it).dest.h;	vertices[this->_i].u = (*it).src.x * this->_tw;					vertices[this->_i].v = ((*it).src.y + (*it).src.h) * this->_th;	this->_i++;
			}
			if (sequence.color == APRIL_COLOR_WHITE)
			{
				april::rendersys->render(april::TriangleList, vertices, this->_i);
			}
			else
			{
				april::rendersys->render(april::TriangleList, vertices, this->_i, sequence.color);
			}
		}
	}

	void Renderer::drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
	{
		this->_cacheKeySequence.set(text, fontName, rect.getSize(), horizontal, vertical, color, offset);
		this->_drawRect = grect(0.0f, 0.0f, rect.getSize());
		if (!cache.get(this->_cacheKeySequence, &this->_currentSequences))
		{
			harray<FormatTag> tags;
			hstr unformattedText = this->analyzeFormatting(text, tags);
			FormatTag tag;
			tag.type = FORMAT_COLOR;
			tag.data = color.hex();
			tags.push_front(tag);
			tag.type = FORMAT_FONT;
			tag.data = fontName;
			tags.push_front(tag);
			this->_lines = this->createRenderLines(this->_drawRect, unformattedText, tags, horizontal, vertical, offset);
			this->_currentSequences = this->createRenderSequences(this->_drawRect, this->_lines, tags);
			cache.set(this->_cacheKeySequence, this->_currentSequences);
			cache.update();
		}
		foreach (RenderSequence, it, this->_currentSequences)
		{
			this->_drawRenderSequence((*it), rect.getPosition());
		}
	}

	void Renderer::drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		this->_cacheKeySequence.set(text, fontName, rect.getSize(), horizontal, vertical, color, offset);
		this->_drawRect = grect(0.0f, 0.0f, rect.getSize());
		if (!cacheUnformatted.get(this->_cacheKeySequence, &this->_currentSequences))
		{
			harray<FormatTag> tags;
			FormatTag tag;
			tag.type = FORMAT_COLOR;
			tag.data = color.hex();
			tags.push_front(tag);
			tag.type = FORMAT_FONT;
			tag.data = fontName;
			tags.push_front(tag);
			this->_lines = this->createRenderLines(this->_drawRect, text, tags, horizontal, vertical, offset);
			this->_currentSequences = this->createRenderSequences(this->_drawRect, this->_lines, tags);
			cacheUnformatted.set(this->_cacheKeySequence, this->_currentSequences);
			cacheUnformatted.update();
		}
		foreach (RenderSequence, it, this->_currentSequences)
		{
			this->_drawRenderSequence((*it), rect.getPosition());
		}
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
		if (text != "")
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
		if (text != "")
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
		if (text != "")
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
		if (text != "")
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
		tag.type = FORMAT_FONT;
		tag.data = fontName;
		tags.push_front(tag);
		return unformattedText;
	}
	
	harray<FormatTag> Renderer::prepareTags(chstr fontName)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = FORMAT_FONT;
		tag.data = fontName;
		tags.push_front(tag);
		return tags;
	}
	
	RenderLine Renderer::getFittingLine(chstr fontName, grect rect, chstr text, harray<FormatTag> tags)
	{
		this->_cacheKeyLine.set(text, fontName, rect.getSize());
		if (!cacheLines.get(this->_cacheKeyLine, &this->_currentLine))
		{
			this->_currentLine = this->_calculateFittingLine(rect, text, tags);
			cacheLines.set(this->_cacheKeyLine, this->_currentLine);
			cacheLines.update();
		}
		return this->_currentLine;
	}

	RenderLine Renderer::_calculateFittingLine(grect rect, chstr text, harray<FormatTag> tags)
	{
		harray<RenderWord> words = this->createRenderWords(rect, text, tags, true);
		RenderLine line;

		float lineWidth = 0.0f;
		bool nextLine = false;
		bool addWord = false;
		line.rect.h = this->_height;
		for (int i = 0; i < words.size(); i++)
		{
			nextLine = (i == words.size() - 1);
			addWord = true;
			if (words[i].text == "\n")
			{
				addWord = false;
				nextLine = true;
			}
			else if (lineWidth + words[i].rect.w > rect.w)
			{
				if (line.words.size() > 0)
				{
					addWord = false;
					i--;
				}
				// else the whole word is the only one in the line and doesn't fit, so just chop it off
				nextLine = true;
			}
			if (addWord)
			{
				lineWidth += words[i].rect.w;
				line.words += words[i];
			}
			if (nextLine)
			{
				float width = 0.0f;
				foreach (RenderWord, it, line.words)
				{
					width += (*it).rect.w;
				}

				foreach (RenderWord, it, line.words)
				{
					line.text += (*it).text;
					line.spaces += (*it).spaces;
					line.rect.w += (*it).rect.w;
				}
				break;
			}
		}
		return line;
	}

}