/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <math.h>
#include <stdio.h>

// VS compilers' math.h does not have round defined
#ifndef round
float round(float f)
{
	return floor(f + 0.5f);
}
#endif

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Atres.h"
#include "Font.h"
#include "Util.h"

namespace Atres
{
    hmap<hstr,Font*> fonts;
	Font* defaultFont = NULL;
	void (*g_logFunction)(chstr) = atres_writelog;
	gvec2 shadowOffset(1.0f, 1.0f);
	April::Color shadowColor(255, 0, 0, 0);
	float borderOffset = 1.0f;
	April::Color borderColor(255, 0, 0, 0);

    void init()
    {
    }
    
    void destroy()
    {
		foreach_m (Font*, it, fonts)
		{
			delete it->second;
		}
    }

	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction = fnptr;
	}
	
	void logMessage(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
	void atres_writelog(chstr message)
	{
		printf("%s\n", message.c_str());		
	}
	
/******* ANALYZE TEXT **************************************************/

	unsigned int getCharUtf8(const char* s, int* char_len_out)
	{
		if (*s < 0)
		{
			const unsigned char* u = (const unsigned char*)s;
			const unsigned char first = *u;
			if ((first & 0xE0) == 0xC0)
			{
				*char_len_out = 2;
				return ((first & 0x1F) << 6) | (u[1] & 0x3F);
			}
			if ((first & 0xF0) == 0xE0)
			{
				*char_len_out = 3;
				return ((((first & 0xF) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F);
			}
			*char_len_out = 4;
			return ((((((first & 7) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F)) << 6) | (u[3] & 0x3F);
		}
		*char_len_out = 1;
		return *s;
	}

	harray<RenderLine> verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float y, float lineHeight)
	{
		harray<RenderLine> result;
		int count = round((lines[lines.size() - 1].rect.y - lines[0].rect.y) / lineHeight) + 1;
		// vertical correction
		switch (vertical)
		{
		case CENTER:
			y += (count * lineHeight - rect.h) / 2;
			break;
		case BOTTOM:
			y += count * lineHeight - rect.h;
			break;
		}
		// remove lines that cannot be seen anyway
		foreach (RenderLine, it, lines)
		{
			(*it).rect.y -= y;
			if ((*it).rect.intersects(rect))
			{
				result += (*it);
			}
		}
		return result;
	}
	
	harray<RenderLine> horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float x, float lineWidth)
	{
		// horizontal correction not necessary when left aligned
		if (horizontal == LEFT || horizontal == LEFT_WRAPPED)
		{
			foreach (RenderLine, it, lines)
			{
				(*it).rect.x -= x;
			}
			return lines;
		}
		harray<RenderLine> lineParts;
		harray<float> widths;
		float y = lines[0].rect.y;
		float width = 0.0f;
		// find all lines
		foreach (RenderLine, it, lines)
		{
			if ((*it).rect.y > y)
			{
				foreach (RenderLine, it2, lineParts)
				{
					width += (*it2).rect.w;
				}
				widths.push_back(width, lineParts.size());
				y = (*it).rect.y;
				width = 0.0f;
				lineParts.clear();
			}
			lineParts += (*it);
		}
		foreach (RenderLine, it, lineParts)
		{
			width += (*it).rect.w;
		}
		widths.push_back(width, lineParts.size());
		// horizontal correction
		for (int i = 0; i < lines.size(); i++)
		{
			switch (horizontal)
			{
			case CENTER:
			case CENTER_WRAPPED:
				lines[i].rect.x += -x + (rect.w - widths[i]) / 2;
				break;
			case RIGHT:
			case RIGHT_WRAPPED:
				lines[i].rect.x += -x + rect.w - widths[i];
				break;
			}
		}
		return lines;
	}

	harray<RenderSequence> createRenderSequences(grect rect, chstr text, harray<FormatTag> tags,
		Alignment horizontal, Alignment vertical, gvec2 offset)
	{
		harray<RenderLine> lines;
		RenderLine line;
		harray<FormatTag> stack;
		harray<FormatTag> formatTags(tags);
		FormatTag tag;
		FormatTag nextTag = tags.front();
		
		hstr fontName;
		Font* font = NULL;
		hmap<unsigned int, CharacterDefinition> characters;
		float lineHeight;
		float scale;
		
		const char* str = text.c_str();
		bool wrapped = (horizontal == LEFT_WRAPPED || horizontal == RIGHT_WRAPPED || horizontal == CENTER_WRAPPED);
		bool checkingSpaces = false;
		int i = 0;
		int start = 0;
		int current = 0;
		int byteLength;
		unsigned int code;
		float width;
		float advance;
		float lineWidth = 0.0f;
		float y = 0.0f;
		
		
		while (i < text.size())
		{
			i = start + current;
			line.text = "";
			while (i < text.size() && str[i] == ' ') // skip initial spaces in the line
			{
				i++;
			}
			start = i;
			current = 0;
			width = 0.0f;
			advance = 0.0f;
			checkingSpaces = false;
			while (true) // checking how much fits into this line
			{
				while (tags.size() > 0 && i >= nextTag.start)
				{
					if (nextTag.type == CLOSE)
					{
						tag = stack.pop_back();
						if (tag.type == FORMAT_FONT)
						{
							fontName = tag.data;
							font = getFont(fontName);
							characters = font->getCharacters();
							scale = font->getScale();
						}
					}
					else if (nextTag.type == FORMAT_FONT)
					{
						tag.type = FORMAT_FONT;
						tag.data = fontName;
						stack += tag;
						fontName = nextTag.data;
						if (font == NULL)
						{
							font = getFont(fontName);
							lineHeight = font->getLineHeight();
						}
						else
						{
							font = getFont(fontName);
						}
						characters = font->getCharacters();
						scale = font->getScale();
					}
					else
					{
						tag.type = FORMAT_NORMAL;
						stack += tag;
					}
					tags.pop_front();
					nextTag = tags.front();
				}
				if (tags.size() == 0)
				{
					nextTag.start = text.size() + 1;
				}
				code = getCharUtf8(&str[i], &byteLength);
				if (code == ' ' || code == '\0')
				{
					if (!checkingSpaces)
					{
						width = advance;
						current = i - start;
					}
					checkingSpaces = true;
					if (code == '\0')
					{
						i += byteLength;
						break;
					}
				}
				else
				{
					checkingSpaces = false;
				}
				if (code == '\n')
				{
					width = advance;
					i += byteLength;
					current = i - start;
					break;
				}
				advance += characters[code].aw * scale;
				if (wrapped && advance > rect.w) // current word doesn't fit anymore
				{
					if (current == 0) // whole word doesn't fit into a line, just chop it off
					{
						width = advance - characters[code].aw * scale;
						current = i - start;
					}
					break;
				}
				i += byteLength;
			}
			lineWidth = hmax(lineWidth, width);
			line.text = text(start, current);
			line.rect = grect(rect.x, rect.y + y, width, lineHeight);
			line.start = start;
			lines += line;
			y += lineHeight;
		}
		if (lines.size() > 0)
		{
			lines = verticalCorrection(rect, vertical, lines, offset.y, lineHeight);
			if (lines.size() > 0)
			{
				lines = horizontalCorrection(rect, horizontal, lines, offset.x, lineWidth);
			}
		}
		if (lines.size() == 0)
		{
			return harray<RenderSequence>();
		}
		return analyzeLines(rect, lines, formatTags);
	}
	
	harray<RenderSequence> analyzeLines(grect rect, harray<RenderLine> lines, harray<FormatTag> tags)
	{
		harray<RenderSequence> effectSequences;
		harray<RenderSequence> sequences;
		RenderSequence sequence;
		RenderRectangle renderRect;
		harray<FormatTag> stack;
		RenderLine line;
		FormatTag tag;
		FormatTag nextTag = tags.front();
		grect area;
		
		hstr fontName;
		Font* font = NULL;
		hmap<unsigned int, CharacterDefinition> characters;
		float lineHeight;
		float scale;
		April::Color color;
		int effectMode = 0;
		
		int byteLength;
		unsigned int code;
		float width;
		
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			width = 0.0f;
			for (int i = 0; i < line.text.size(); i += byteLength)
			{
				while (tags.size() > 0 && line.start + i >= nextTag.start)
				{
					if (nextTag.type == CLOSE)
					{
						tag = stack.pop_back();
						switch (tag.type)
						{
						case FORMAT_FONT:
							fontName = tag.data;
							font = getFont(fontName);
							characters = font->getCharacters();
							scale = font->getScale();
							break;
						case FORMAT_COLOR:
							color = hexstr_to_color(tag.data);
							break;
						case FORMAT_NORMAL:
							effectMode = 0;
							break;
						case FORMAT_SHADOW:
							effectMode = 1;
							break;
						case FORMAT_BORDER:
							effectMode = 2;
							break;
						}
					}
					else
					{
						switch (nextTag.type)
						{
						case FORMAT_FONT:
							tag.type = FORMAT_FONT;
							tag.data = fontName;
							stack += tag;
							fontName = nextTag.data;
							if (font == NULL)
							{
								font = getFont(fontName);
								lineHeight = font->getLineHeight();
							}
							else
							{
								font = getFont(fontName);
							}
							characters = font->getCharacters();
							scale = font->getScale();
							break;
						case FORMAT_COLOR:
							tag.type = FORMAT_COLOR;
							tag.data = hsprintf("%02x%02x%02x%02x", color.a, color.r, color.g, color.b);
							stack += tag;
							color = hexstr_to_color(nextTag.data);
							break;
						case FORMAT_NORMAL:
							tag.type = (effectMode == 2 ? FORMAT_BORDER : (effectMode == 1 ? FORMAT_SHADOW : FORMAT_NORMAL));
							stack += tag;
							effectMode = 0;
							break;
						case FORMAT_SHADOW:
							tag.type = (effectMode == 2 ? FORMAT_BORDER : (effectMode == 1 ? FORMAT_SHADOW : FORMAT_NORMAL));
							stack += tag;
							effectMode = 1;
							break;
						case FORMAT_BORDER:
							tag.type = (effectMode == 2 ? FORMAT_BORDER : (effectMode == 1 ? FORMAT_SHADOW : FORMAT_NORMAL));
							stack += tag;
							effectMode = 2;
							break;
						}
					}
					tags.pop_front();
					nextTag = tags.front();
					if (sequence.rectangles.size() > 0)
					{
						sequences += sequence;
					}
					sequence.texture = font->getTexture();
					sequence.color = color;
					sequence.rectangles.clear();
				}
				if (tags.size() == 0)
				{
					if (lines.size() > 0)
					{
						nextTag.start = lines.back().start + lines.back().text.size() + 1;
					}
					else
					{
						nextTag.start = line.start + line.text.size() + 1;
					}
				}
				code = getCharUtf8(&line.text[i], &byteLength);
				area = line.rect;
				area.x += width;
				area.w = characters[code].aw * scale;
				area.y += (lineHeight - font->getHeight()) / 2;
				area.h = font->getHeight();
				renderRect = font->makeRenderRectangle(rect, area, code);
				sequence.rectangles += renderRect;
				width += characters[code].aw * scale;
			}
		}
		if (sequence.rectangles.size() > 0)
		{
			sequences += sequence;
		}
		return sequences;
	}

/******* DRAW TEXT *****************************************************/

	void drawRenderSequence(RenderSequence& sequence)
	{
		if (sequence.rectangles.size() > 0)
		{
			April::TexturedVertex v[BUFFER_MAX_CHARACTERS];
			float w = (float)sequence.texture->getWidth();
			float h = (float)sequence.texture->getHeight();
			int i = 0;
			foreach (RenderRectangle, it, sequence.rectangles)
			{
				v[i].x = (*it).dest.x;                v[i].y = (*it).dest.y;                v[i].z = 0; v[i].u = (*it).src.x / w;                 v[i].v = (*it).src.y / h;                 i++;
				v[i].x = (*it).dest.x + (*it).dest.w; v[i].y = (*it).dest.y;                v[i].z = 0; v[i].u = ((*it).src.x + (*it).src.w) / w; v[i].v = (*it).src.y / h;                 i++;
				v[i].x = (*it).dest.x;                v[i].y = (*it).dest.y + (*it).dest.h; v[i].z = 0; v[i].u = (*it).src.x / w;                 v[i].v = ((*it).src.y + (*it).src.h) / h; i++;
				v[i].x = (*it).dest.x + (*it).dest.w; v[i].y = (*it).dest.y;                v[i].z = 0; v[i].u = ((*it).src.x + (*it).src.w) / w; v[i].v = (*it).src.y / h;                 i++;
				v[i].x = (*it).dest.x + (*it).dest.w; v[i].y = (*it).dest.y + (*it).dest.h; v[i].z = 0; v[i].u = ((*it).src.x + (*it).src.w) / w; v[i].v = ((*it).src.y + (*it).src.h) / h; i++;
				v[i].x = (*it).dest.x;                v[i].y = (*it).dest.y + (*it).dest.h; v[i].z = 0; v[i].u = (*it).src.x / w;                 v[i].v = ((*it).src.y + (*it).src.h) / h; i++;
			}
			April::Color color = sequence.color;
			April::rendersys->setTexture(sequence.texture);
			April::rendersys->render(April::TriangleList, v, i, color.r_float(), color.g_float(), color.b_float(), color.a_float());
		}
	}

	void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		harray<FormatTag> tags;
		hstr unformattedText = analyzeFormatting(text, tags);
		FormatTag tag;
		tag.type = FORMAT_COLOR;
		tag.data = hsprintf("%02x%02x%02x%02x", color.a, color.r, color.g, color.b);
		tag.start = 0;
		tag.count = 0;
		tags.push_front(tag);
		tag.type = FORMAT_FONT;
		tag.data = fontName;
		tags.push_front(tag);
		harray<RenderSequence> sequences = createRenderSequences(rect, unformattedText, tags, horizontal, vertical, offset);
		foreach (RenderSequence, it, sequences)
		{
			drawRenderSequence(*it);
		}
	}

	void drawTextShadowed(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawText(fontName, rect, "[s]" + text, horizontal, vertical, color, offset);
	}

	void drawTextBordered(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawText(fontName, rect, "[b]" + text, horizontal, vertical, color, offset);
	}

/******* DRAW TEXT OVERLOADS *******************************************/

	void drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawText("", rect, text, horizontal, vertical, color, offset);
	}

	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawText("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
	void drawTextShadowed(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawTextShadowed("", rect, text, horizontal, vertical, color, offset);
	}

	void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextShadowed(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextShadowed("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextShadowed(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextShadowed("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
	void drawTextBordered(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawTextBordered("", rect, text, horizontal, vertical, color, offset);
	}

	void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextBordered(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextBordered("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextBordered(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextBordered("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
/******* PROPERTIES ****************************************************/

	gvec2 getShadowOffset()
	{
		return shadowOffset;
	}
	
	void setShadowOffset(gvec2 value)
	{
		shadowOffset = value;
	}
	
	April::Color getShadowColor()
	{
		return shadowColor;
	}
	
	void setShadowColor(April::Color value)
	{
		shadowColor = value;
	}
	
	float getBorderOffset()
	{
		return borderOffset;
	}
	
	void setBorderOffset(float value)
	{
		borderOffset = value;
	}
	
	April::Color getBorderColor()
	{
		return borderColor;
	}
	
	void setBorderColor(April::Color value)
	{
		borderColor = value;
	}
	
/******* OTHER *********************************************************/
	
	hstr analyzeFormatting(chstr text, harray<FormatTag>& tags)
	{
		const char* str = text.c_str();
		int start = 0;
		int end;
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
				if (stack.size() > 0 && stack.back() != str[start + 2]) // interleaving, ignore the tag
				{
					start = end;
#ifdef _DEBUG
					logMessage(hsprintf("Warning: closing tag that was not opened (\"[/%c]\" in \"%s\")", str[start + 2], str));
#endif
					continue;
				}
				stack.pop_back();
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

	float getFontHeight(chstr fontName)
	{
		return getFont(fontName)->getHeight();
	}
	
	float getTextWidth(chstr fontName, chstr text)
	{
		//2DO - make it work with formatted text properly
		return getFont(fontName)->getTextWidth(text);
	}

	float getTextHeight(chstr fontName, chstr text, float maxWidth)
	{
		Font* f = getFont(fontName);
		harray<grect> areas;
		harray<hstr> lines;
		//f->testRender(grect(0, 0, maxWidth, 100000), text, LEFT_WRAPPED, TOP, lines, areas);
		//2DO - make it work with formatted text properly
		return (lines.size() * f->getLineHeight());
	}
	
	int getTextCount(chstr fontName, chstr text, float maxWidth)
	{
		//return (text != "" ? getFont(fontName)->getTextCount(text, maxWidth) : 0);
		harray<FormatTag> tags;
		hstr unformattedText = analyzeFormatting(text, tags);
		//2DO - make it work with formatted text properly
		return (unformattedText != "" ? getFont(fontName)->getTextCount(unformattedText, maxWidth) : 0);
	}
	
	void setDefaultFont(chstr name)
	{
		if (!fonts.has_key(name))
		{
			throw resource_error("Font", name, "Atres");
		}
		defaultFont = fonts[name];
	}
    
    void loadFont(chstr filename)
    {
		logMessage(hsprintf("loading font %s", filename.c_str()));
        Font* font = new Font(filename);
        fonts[font->getName()] = font;
		if (defaultFont == NULL)
		{
			defaultFont = font;
		}
    }
    
    Font* getFont(chstr name)
    {
		if (name == "" && defaultFont != NULL)
		{
			defaultFont->setScale(1.0f);
			return defaultFont;
		}
        Font* font;
		if (fonts.has_key(name))
		{
			font = fonts[name];
			font->setScale(1.0f);
			return font;
		}
		int position = name.find(":");
		if (position < 0)
		{
			throw resource_error("Font", name, "Atres");
		}
		font = getFont(name(0, position));
		position++;
		font->setScale((float)(name(position, name.size() - position)));
        return font;
    }
	
}
