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

namespace Atres
{
    hmap<hstr,Font*> fonts;
	Font* default_font = NULL;
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

	harray<FormatOperation> verticalCorrection(grect rect, Alignment vertical, harray<FormatOperation> operations, float y, float lineHeight)
	{
		harray<FormatOperation> result;
		int lines = round((operations[operations.size() - 1].rect.y - operations[0].rect.y) / lineHeight) + 1;
		// vertical correction
		switch (vertical)
		{
		case CENTER:
			y += (lines * lineHeight - rect.h) / 2;
			break;
		case BOTTOM:
			y += lines * lineHeight - rect.h;
			break;
		}
		// remove lines that cannot be seen anyway
		foreach (FormatOperation, it, operations)
		{
			if ((*it).type == TEXT)
			{
				(*it).rect.y -= y;
				if ((*it).rect.intersects(rect))
				{
					result += (*it);
				}
			}
			else
			{
				result += (*it);
			}
		}
		return result;
	}
	
	harray<FormatOperation> horizontalCorrection(grect rect, Alignment horizontal, harray<FormatOperation> operations, float x, float lineWidth)
	{
		// horizontal correction not necessary when left aligned
		if (horizontal == LEFT || horizontal == LEFT_WRAPPED)
		{
			foreach (FormatOperation, it, operations)
			{
				(*it).rect.x -= x;
			}
			return operations;
		}
		harray<FormatOperation> lineParts;
		harray<float> widths;
		float y = operations[0].rect.y;
		float width = 0.0f;
		// find all lines from format operations
		foreach (FormatOperation, it, operations)
		{
			if ((*it).rect.y > y)
			{
				foreach (FormatOperation, it2, lineParts)
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
		foreach (FormatOperation, it2, lineParts)
		{
			width += (*it2).rect.w;
		}
		widths.push_back(width, lineParts.size());
		// horizontal correction
		for (int i = 0; i < operations.size(); i++)
		{
			switch (horizontal)
			{
			case CENTER:
			case CENTER_WRAPPED:
				operations[i].rect.x += -x + (rect.w - widths[i]) / 2;
				break;
			case RIGHT:
			case RIGHT_WRAPPED:
				operations[i].rect.x += -x + rect.w - widths[i];
				break;
			}
		}
		return operations;
	}
	
	harray<FormatOperation> analyzeText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		hstr font = fontName;
		harray<FormatOperation> result;
		Font* f = getFont(fontName);
		float lineHeight = f->getLineHeight();
		hmap<unsigned int, FontCharDef> characters = f->getCharacters();
		float scale = f->getScale();
		bool wrapped = (horizontal == LEFT_WRAPPED || horizontal == RIGHT_WRAPPED || horizontal == CENTER_WRAPPED);
		const char* str = text.c_str();
		int byteLength;
		bool checkingSpaces;
		float width;
		float advance;
		unsigned int code = 0;
		int i = 0;
		int start = 0;
		int current = 0;
		FormatOperation operation;
		float lineWidth = 0.0f;
		float y = 0.0f;
		while (i < text.size())
		{
			i = start + current;
			operation.data = "";
			operation.size = current;
			operation.type = TEXT;
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
			operation.rect = grect(rect.x, rect.y + y, width, lineHeight);
			if (current > 0)
			{
				operation.data += text(start, current);
			}
			operation.data = operation.data.trim();
			operation.size += current;
			result += operation;
			y += lineHeight;
		}
		if (result.size() > 0)
		{
			result = verticalCorrection(rect, vertical, result, offset.y, lineHeight);
			if (result.size() > 0)
			{
				result = horizontalCorrection(rect, horizontal, result, offset.x, lineWidth);
			}
		}
		return result;
	}

	harray<FormatOperation> calculateTextPositions(grect rect, chstr text, harray<FormatTag> tags,
		Alignment horizontal, Alignment vertical, gvec2 offset)
	{
		Font* f = getFont(tags.pop_front().data);
		harray<FormatOperation> result;
		float lineHeight = f->getLineHeight();
		hmap<unsigned int, FontCharDef> characters = f->getCharacters();
		float scale = f->getScale();
		bool wrapped = (horizontal == LEFT_WRAPPED || horizontal == RIGHT_WRAPPED || horizontal == CENTER_WRAPPED);
		const char* str = text.c_str();
		int byteLength;
		bool checkingSpaces;
		float width;
		float advance;
		unsigned int code = 0;
		int i = 0;
		int start = 0;
		int current = 0;
		FormatOperation operation;
		float lineWidth = 0.0f;
		float y = 0.0f;
		while (i < text.size())
		{
			i = start + current;
			operation.data = "";
			operation.size = current;
			operation.type = TEXT;
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
			operation.rect = grect(rect.x, rect.y + y, width, lineHeight);
			if (current > 0)
			{
				operation.data += text(start, current);
			}
			operation.data = operation.data.trim();
			operation.size += current;
			result += operation;
			y += lineHeight;
		}
		if (result.size() > 0)
		{
			result = verticalCorrection(rect, vertical, result, offset.y, lineHeight);
			result = horizontalCorrection(rect, horizontal, result, offset.x, lineWidth);
		}
		return result;
	}

/******* DRAW TEXT *****************************************************/

	void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		harray<FormatTag> tags;
		//hstr unformattedText = removeFormatting(text);
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
		harray<FormatOperation> operationsDepr = calculateTextPositions(rect, unformattedText, tags, horizontal, vertical, offset);
		harray<grect> areas;
		harray<hstr> lines;
		foreach (FormatOperation, it, operationsDepr)
		{
			areas += (*it).rect;
			lines += (*it).data;
		}
		getFont(fontName)->renderRaw(rect, lines, areas, color);
		flushRenderOperations();
	}

	void drawTextShadowed(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		//drawText(fontName, rect, "[s]" + text, horizontal, vertical, color, offset);
		///*
		harray<FormatTag> operations;
		hstr unformattedText = analyzeFormatting(text, operations);
		harray<FormatOperation> operationsDepr = analyzeText(fontName, rect, unformattedText, horizontal, vertical, color, offset);
		Font* f = getFont(fontName);
		harray<grect> areas;
		harray<hstr> lines;
		foreach (FormatOperation, it, operationsDepr)
		{
			areas += (*it).rect;
			lines += (*it).data;
		}
		f->renderRaw(rect + shadowOffset, lines, areas, shadowColor);
		flushRenderOperations();
		f->renderRaw(rect, lines, areas, color);
		flushRenderOperations();
		//*/
	}

	void drawTextBordered(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		//drawText(fontName, rect, "[b]" + text, horizontal, vertical, color, offset);
		///*
		harray<FormatTag> operations;
		hstr unformattedText = analyzeFormatting(text, operations);
		harray<FormatOperation> operationsDepr = analyzeText(fontName, rect, unformattedText, horizontal, vertical, color, offset);
		Font* f = getFont(fontName);
		harray<grect> areas;
		harray<hstr> lines;
		foreach (FormatOperation, it, operationsDepr)
		{
			areas += (*it).rect;
			lines += (*it).data;
		}
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(0.0f, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, 0.0f) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, 0.0f) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(0.0f, borderOffset) * f->getScale());
		flushRenderOperations();
		f->renderRaw(rect, lines, areas, color);
		flushRenderOperations();
		//*/
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
		f->testRender(grect(0, 0, maxWidth, 100000), text, LEFT_WRAPPED, TOP, lines, areas);
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
		default_font=fonts[name];
	}
    
    void loadFont(chstr filename)
    {
		logMessage(hsprintf("loading font %s", filename.c_str()));
        Font* f=new Font(filename);
        fonts[f->getName()]=f;
		if (default_font == 0) default_font=f;
    }
    
    Font* getFont(chstr name)
    {
		if (name == "" && default_font != 0)
		{
			default_font->setScale(0);
			return default_font;
		}
		
        Font* f;
        if (fonts.find(name) == fonts.end())
		{
			int pos=name.find(":");
			if (pos != -1)
			{
				f=getFont(name(0,pos));
				f->setScale((float)atof(name(pos+1,10).c_str()));
			}
			else throw resource_error("Font",name,"Atres");
		}
		else
		{
			f=fonts[name];
			f->setScale(0);
		}
        return f;
    }
	
}
