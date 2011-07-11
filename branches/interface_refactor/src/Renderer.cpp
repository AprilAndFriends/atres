/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <math.h>
#include <stdio.h>
#include <hltypes/util.h>

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "atres.h"
#include "FontResource.h"
#include "Util.h"

namespace atres
{
    hmap<hstr, FontResource*> fonts;
	FontResource* defaultFont = NULL;
	int cacheSize = 100;
	int cacheIndex = 0;
	gvec2 shadowOffset(1.0f, 1.0f);
	april::Color shadowColor = APRIL_COLOR_BLACK;
	float borderOffset = 1.0f;
	april::Color borderColor = APRIL_COLOR_BLACK;
	hmap<hstr, CacheEntry> cache;
	hmap<hstr, hstr> colors;
	bool globalOffsets = false;
	april::TexturedVertex vertices[BUFFER_MAX_CHARACTERS * 6];

	Renderer::Renderer()
	{
		colors["red"] = APRIL_COLOR_RED.hex();
		colors["green"] = APRIL_COLOR_GREEN.hex();
		colors["blue"] = APRIL_COLOR_BLUE.hex();
		colors["yellow"] = APRIL_COLOR_YELLOW.hex();
		colors["mangenta"] = APRIL_COLOR_MANGENTA.hex();
		colors["cyan"] = APRIL_COLOR_CYAN.hex();
		colors["orange"] = APRIL_COLOR_ORANGE.hex();
		colors["pink"] = APRIL_COLOR_PINK.hex();
		colors["teal"] = APRIL_COLOR_TEAL.hex();
		colors["neon"] = APRIL_COLOR_NEON.hex();
		colors["purple"] = APRIL_COLOR_PURPLE.hex();
		colors["aqua"] = APRIL_COLOR_AQUA.hex();
		colors["white"] = APRIL_COLOR_WHITE.hex();
		colors["grey"] = APRIL_COLOR_GREY.hex();
		colors["black"] = APRIL_COLOR_BLACK.hex();
	}

	Renderer::~Renderer()
	{
		foreach_m (FontResource*, it, fonts)
		{
			delete it->second;
		}
	}

    void Renderer::registerFontResource(FontResource* fontResource)
    {
		atres::log(hsprintf("registering font resource %s", fontResource->getName().c_str()));
        fonts[fontResource->getName()] = fontResource;
		if (defaultFont == NULL)
		{
			defaultFont = fontResource;
		}
    }
    
/******* ANALYZE TEXT **************************************************/

	hstr Renderer::analyzeFormatting(chstr text, harray<FormatTag>& tags)
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
					atres::log(hsprintf("Warning: closing tag that was not opened (\"[/%c]\" in \"%s\")", str[start + 2], str));
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

	harray<RenderLine> Renderer::verticalCorrection(grect rect, Alignment vertical, harray<RenderLine> lines, float y, float lineHeight)
	{
		harray<RenderLine> result;
		int count = hround((lines[lines.size() - 1].rect.y - lines[0].rect.y) / lineHeight) + 1;
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
	
	harray<RenderLine> Renderer::horizontalCorrection(grect rect, Alignment horizontal, harray<RenderLine> lines, float x, float lineWidth)
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

	harray<RenderLine> Renderer::createRenderLines(grect rect, chstr text, harray<FormatTag> tags,
		Alignment horizontal, Alignment vertical, gvec2 offset)
	{
		harray<RenderLine> lines;
		RenderLine line;
		harray<FormatTag> stack;
		FormatTag tag;
		FormatTag nextTag = tags.front();
		
		hstr fontName;
		FontResource* fontResource = NULL;
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
							fontResource = getFontResource(fontName);
							characters = fontResource->getCharacters();
							scale = fontResource->getScale();
						}
					}
					else if (nextTag.type == FORMAT_FONT)
					{
						tag.type = FORMAT_FONT;
						tag.data = fontName;
						stack += tag;
						try
						{
							if (fontResource == NULL)
							{
								fontResource = getFontResource(nextTag.data);
								lineHeight = fontResource->getLineHeight();
							}
							else
							{
								fontResource = getFontResource(nextTag.data);
							}
							fontName = nextTag.data;
							characters = fontResource->getCharacters();
							scale = fontResource->getScale();
						}
						catch (hltypes::_resource_error e)
						{
#ifdef _DEBUG
							atres::log(hsprintf("Warning: font \"%s\" does not exist", nextTag.data.c_str()));
#endif
						}
					}
					else
					{
						tag.type = FORMAT_NORMAL;
						stack += tag;
					}
					tags.pop_front();
					if (tags.size() > 0)
					{
						nextTag = tags.front();
					}
					else
					{
						nextTag.start = text.size() + 1;
					}
				}
				code = utf8_to_uint(&str[i], &byteLength);
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
						if (current == 0) // word doesn't fit into line at all
						{
							i = text.size();
						}
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
		return lines;
	}
	
	harray<RenderSequence> Renderer::createRenderSequences(grect rect, harray<RenderLine> lines, harray<FormatTag> tags)
	{
		harray<RenderSequence> sequences;
		RenderSequence sequence;
		harray<RenderSequence> shadowSequences;
		RenderSequence shadowSequence;
		shadowSequence.color = shadowColor;
		harray<RenderSequence> borderSequences;
		RenderSequence borderSequence;
		borderSequence.color = borderColor;
		borderSequence.color.a = (unsigned char)(borderColor.a * borderColor.a_f() * borderColor.a_f());
		RenderRectangle renderRect;
		harray<FormatTag> stack;
		RenderLine line;
		FormatTag tag;
		FormatTag nextTag = tags.front();
		grect area;
		
		hstr fontName;
		FontResource* fontResource = NULL;
		hmap<unsigned int, CharacterDefinition> characters;
		float lineHeight;
		float scale;
		april::Color color;
		hstr hex;
		int effectMode = 0;
		int alpha = -1;
		
		int byteLength;
		unsigned int code;
		float width;
		grect destination;
		bool colorChanged;
		
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			width = 0.0f;
			for (int i = 0; i < line.text.size(); i += byteLength)
			{
				// checking first formatting tag changes
				code = utf8_to_uint(&line.text[i], &byteLength);
				while (tags.size() > 0 && line.start + i >= nextTag.start)
				{
					if (nextTag.type == CLOSE)
					{
						tag = stack.pop_back();
						switch (tag.type)
						{
						case FORMAT_FONT:
							fontName = tag.data;
							fontResource = getFontResource(fontName);
							characters = fontResource->getCharacters();
							scale = fontResource->getScale();
							break;
						case FORMAT_COLOR:
							hex = (colors.has_key(tag.data) ? colors[tag.data] : tag.data);
							if ((hex.size() == 6 || hex.size() == 8) && is_hexstr(hex))
							{
								color.set(hex);
							}
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
							try
							{
								if (fontResource == NULL)
								{
									fontResource = getFontResource(nextTag.data);
									lineHeight = fontResource->getLineHeight();
								}
								else
								{
									fontResource = getFontResource(nextTag.data);
								}
								fontName = nextTag.data;
								characters = fontResource->getCharacters();
								scale = fontResource->getScale();
							}
							catch (hltypes::_resource_error e)
							{
#ifdef _DEBUG
								atres::log(hsprintf("Warning: font \"%s\" does not exist", nextTag.data.c_str()));
#endif
							}
							break;
						case FORMAT_COLOR:
							tag.type = FORMAT_COLOR;
							tag.data = color.hex();
							stack += tag;
							hex = (colors.has_key(nextTag.data) ? colors[nextTag.data] : nextTag.data);
							if ((hex.size() == 6 || hex.size() == 8) && is_hexstr(hex))
							{
								color.set(hex);
								alpha == -1 ? alpha = color.a : color.a = (unsigned char)(alpha * color.a_f());
							}
#ifdef _DEBUG
							else
							{
								atres::log(hsprintf("Warning: color \"%s\" does not exist", hex.c_str()));
							}
#endif
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
					if (tags.size() > 0)
					{
						nextTag = tags.front();
					}
					else if (lines.size() > 0)
					{
						nextTag.start = lines.back().start + lines.back().text.size() + 1;
					}
					else
					{
						nextTag.start = line.start + line.text.size() + 1;
					}
					colorChanged = sequence.color != color;
					if (sequence.texture != fontResource->getTexture(code) || colorChanged)
					{
						if (sequence.rectangles.size() > 0)
						{
							sequences += sequence;
							sequence.rectangles.clear();
						}
						sequence.texture = fontResource->getTexture(code);
						sequence.color = color;
					}
					if (shadowSequence.texture != fontResource->getTexture(code) || colorChanged)
					{
						if (shadowSequence.rectangles.size() > 0)
						{
							shadowSequences += shadowSequence;
							shadowSequence.rectangles.clear();
						}
						shadowSequence.texture = fontResource->getTexture(code);
						shadowSequence.color = shadowColor;
						shadowSequence.color.a = (unsigned char)(shadowSequence.color.a * color.a_f());
					}
					if (borderSequence.texture != fontResource->getTexture(code) || colorChanged)
					{
						if (borderSequence.rectangles.size() > 0)
						{
							borderSequences += borderSequence;
							borderSequence.rectangles.clear();
						}
						borderSequence.texture = fontResource->getTexture(code);
						borderSequence.color = borderColor;
						borderSequence.color.a = (unsigned char)(borderSequence.color.a * color.a_f() * color.a_f());
					}
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
				// checking the particular character
				area = line.rect;
				area.x += width;
				area.w = characters[code].w * scale;
				area.y += (lineHeight - fontResource->getHeight()) / 2;
				area.h = fontResource->getHeight();
				renderRect = fontResource->makeRenderRectangle(rect, area, code);
				sequence.rectangles += renderRect;
				destination = renderRect.dest;
				switch (effectMode)
				{
				case 1: // shadow
					renderRect.dest = destination + shadowOffset * (globalOffsets ? 1 : scale);
					shadowSequence.rectangles += renderRect;
					break;
				case 2: // border
					renderRect.dest = destination + gvec2(-borderOffset, -borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(borderOffset, -borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(-borderOffset, borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(borderOffset, borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(0.0f, -borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(-borderOffset, 0.0f) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(borderOffset, 0.0f) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					renderRect.dest = destination + gvec2(0.0f, borderOffset) * (globalOffsets ? 1 : scale);
					borderSequence.rectangles += renderRect;
					break;
				}
				width += characters[code].aw * scale;
			}
		}
		if (sequence.rectangles.size() > 0)
		{
			sequences += sequence;
		}
		if (shadowSequence.rectangles.size() > 0)
		{
			shadowSequences += shadowSequence;
		}
		if (borderSequence.rectangles.size() > 0)
		{
			borderSequences += borderSequence;
		}
		borderSequences = optimizeSequences(borderSequences);
		shadowSequences = optimizeSequences(shadowSequences);
		return optimizeSequences(shadowSequences + borderSequences + sequences);
	}

	harray<RenderSequence> Renderer::optimizeSequences(harray<RenderSequence> sequences)
	{
		harray<RenderSequence> result;
		RenderSequence current;
		int i;
		while (sequences.size() > 0)
		{
			current = sequences.pop_front();
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

	void Renderer::drawRenderSequence(RenderSequence& sequence)
	{
		if (sequence.rectangles.size() == 0)
		{
			return;
		}
		float w = (float)sequence.texture->getWidth();
		float h = (float)sequence.texture->getHeight();
		int i = 0;
		foreach (RenderRectangle, it, sequence.rectangles)
		{
			vertices[i].x = (*it).dest.x;                vertices[i].y = (*it).dest.y;                vertices[i].z = 0; vertices[i].u = (*it).src.x / w;                 vertices[i].v = (*it).src.y / h;                 i++;
			vertices[i].x = (*it).dest.x + (*it).dest.w; vertices[i].y = (*it).dest.y;                vertices[i].z = 0; vertices[i].u = ((*it).src.x + (*it).src.w) / w; vertices[i].v = (*it).src.y / h;                 i++;
			vertices[i].x = (*it).dest.x;                vertices[i].y = (*it).dest.y + (*it).dest.h; vertices[i].z = 0; vertices[i].u = (*it).src.x / w;                 vertices[i].v = ((*it).src.y + (*it).src.h) / h; i++;
			vertices[i].x = (*it).dest.x + (*it).dest.w; vertices[i].y = (*it).dest.y;                vertices[i].z = 0; vertices[i].u = ((*it).src.x + (*it).src.w) / w; vertices[i].v = (*it).src.y / h;                 i++;
			vertices[i].x = (*it).dest.x + (*it).dest.w; vertices[i].y = (*it).dest.y + (*it).dest.h; vertices[i].z = 0; vertices[i].u = ((*it).src.x + (*it).src.w) / w; vertices[i].v = ((*it).src.y + (*it).src.h) / h; i++;
			vertices[i].x = (*it).dest.x;                vertices[i].y = (*it).dest.y + (*it).dest.h; vertices[i].z = 0; vertices[i].u = (*it).src.x / w;                 vertices[i].v = ((*it).src.y + (*it).src.h) / h; i++;
		}
		april::rendersys->setTexture(sequence.texture);
		april::rendersys->render(april::TriangleList, vertices, i, sequence.color);
	}

	void Renderer::drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
	{
		bool needCache = !cache.has_key(text);
		CacheEntry entry;
		grect drawRect = grect(0.0f, 0.0f, rect.getSize());
		if (!needCache)
		{
			entry = cache[text];
			needCache = (entry.fontName != fontName || entry.size.x != drawRect.w || entry.size.y != drawRect.h ||
				entry.horizontal != horizontal || entry.vertical != vertical || entry.color != color || entry.offset != offset);
		}
		else
		{
			updateCache();
			cacheIndex++;
		}
		if (needCache)
		{
			harray<FormatTag> tags;
			hstr unformattedText = analyzeFormatting(text, tags);
			FormatTag tag;
			tag.type = FORMAT_COLOR;
			tag.data = color.hex();
			tags.push_front(tag);
			tag.type = FORMAT_FONT;
			tag.data = fontName;
			tags.push_front(tag);
			harray<RenderLine> lines = createRenderLines(drawRect, unformattedText, tags, horizontal, vertical, offset);
			entry.fontName = fontName;
			entry.index = cacheIndex;
			entry.size = rect.getSize();
			entry.horizontal = horizontal;
			entry.vertical = vertical;
			entry.color = color;
			entry.offset = offset;
			entry.sequences = createRenderSequences(drawRect, lines, tags);
			cache[text] = entry;
		}
		harray<RenderSequence> sequences = cache[text].sequences;
		foreach (RenderSequence, it, sequences)
		{
			foreach (RenderRectangle, it2, (*it).rectangles)
			{
				(*it2).dest.x += rect.x;
				(*it2).dest.y += rect.y;
			}
			drawRenderSequence(*it);
		}
	}

	void Renderer::drawTextUnformatted(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		harray<FormatTag> tags;
		FormatTag tag;
		tag.type = FORMAT_COLOR;
		tag.data = color.hex();
		tags.push_front(tag);
		tag.type = FORMAT_FONT;
		tag.data = fontName;
		tags.push_front(tag);
		harray<RenderLine> lines = createRenderLines(rect, text, tags, horizontal, vertical, offset);
		harray<RenderSequence> sequences = createRenderSequences(rect, lines, tags);
		foreach (RenderSequence, it, sequences)
		{
			drawRenderSequence(*it);
		}
	}

/******* DRAW TEXT OVERLOADS *******************************************/

	void Renderer::drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		drawText("", rect, text, horizontal, vertical, color, offset);
	}

	void Renderer::drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		drawText("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText("", grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawTextUnformatted(grect rect, chstr text, Alignment horizontal, Alignment vertical, april::Color color,
		gvec2 offset)
	{
		drawTextUnformatted("", rect, text, horizontal, vertical, color, offset);
	}

	void Renderer::drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal,
		Alignment vertical, april::Color color, gvec2 offset)
	{
		drawTextUnformatted(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		april::Color color, gvec2 offset)
	{
		drawTextUnformatted("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void Renderer::drawTextUnformatted(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal,
		Alignment vertical, unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextUnformatted(fontName, grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
	void Renderer::drawTextUnformatted(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextUnformatted("", grect(x, y, w, h), text, horizontal, vertical, april::Color(r, g, b, a), offset);
	}
	
/******* PROPERTIES ****************************************************/

	void Renderer::setDefaultFont(chstr name)
	{
		if (!fonts.has_key(name))
		{
			throw resource_error("Font", name, "atres");
		}
		defaultFont = fonts[name];
		cache.clear();
	}
    
    FontResource* Renderer::getFontResource(chstr name)
    {
		if (name == "" && defaultFont != NULL)
		{
			defaultFont->setScale(1.0f);
			return defaultFont;
		}
        FontResource* fontResource;
		if (fonts.has_key(name))
		{
			fontResource = fonts[name];
			fontResource->setScale(1.0f);
			return fontResource;
		}
		int position = name.find(":");
		if (position < 0)
		{
			throw resource_error("Font", name, "atres");
		}
		fontResource = getFontResource(name(0, position));
		position++;
		fontResource->setScale((float)(name(position, name.size() - position)));
        return fontResource;
    }
	
    bool Renderer::hasFont(chstr name)
    {
		return fonts.has_key(name);
    }
	
	void Renderer::setCacheSize(int value)
	{
		cacheSize = value;
		updateCache();
	}
	
	void Renderer::updateCache()
	{
		hstr minKey;
		int minIndex;
		while (cache.size() >= cacheSize)
		{
			minKey = "";
			minIndex = -1;
			foreach_m (CacheEntry, it, cache)
			{
				if (minKey == "" || it->second.index < minIndex)
				{
					minKey = it->first;
					minIndex = it->second.index;
				}
			}
			cache.remove_key(minKey);
		}
	}
	
	void Renderer::setGlobalOffsets(bool value)
	{
		globalOffsets = value;
	}
	
	gvec2 Renderer::getShadowOffset()
	{
		return shadowOffset;
	}
	
	void Renderer::setShadowOffset(gvec2 value)
	{
		shadowOffset = value;
		cache.clear();
	}
	
	april::Color Renderer::getShadowColor()
	{
		return shadowColor;
	}
	
	void Renderer::setShadowColor(april::Color value)
	{
		shadowColor = value;
		cache.clear();
	}
	
	float Renderer::getBorderOffset()
	{
		return borderOffset;
	}
	
	void Renderer::setBorderOffset(float value)
	{
		borderOffset = value;
		cache.clear();
	}
	
	april::Color Renderer::getBorderColor()
	{
		return borderColor;
	}
	
	void Renderer::setBorderColor(april::Color value)
	{
		borderColor = value;
		cache.clear();
	}
	
	void Renderer::addColor(chstr key, chstr value)
	{
		colors[key.lower()] = value.upper();
	}
	
/******* OTHER *********************************************************/
	
	float Renderer::getFontHeight(chstr fontName)
	{
		return getFontResource(fontName)->getHeight();
	}
	
	float Renderer::getTextWidth(chstr fontName, chstr text)
	{
		if (text != "")
		{
			harray<FormatTag> tags;
			hstr unformattedText = prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				return getFittingLine(grect(0, 0, 100000, 1), unformattedText, tags).rect.w;
			}
		}
		return 0.0f;
	}

	float Renderer::getTextHeight(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "")
		{
			harray<FormatTag> tags;
			hstr unformattedText = prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				harray<RenderLine> lines = createRenderLines(grect(0, 0, maxWidth, 100000), unformattedText, tags, LEFT_WRAPPED, TOP);
				return (lines.size() * getFontResource(fontName)->getLineHeight());
			}
		}
		return 0.0f;
	}
	
	int Renderer::getTextCount(chstr fontName, chstr text, float maxWidth)
	{
		if (text != "")
		{
			harray<FormatTag> tags;
			hstr unformattedText = prepareFormatting(fontName, text, tags);
			if (unformattedText != "")
			{
				return getFittingLine(grect(0, 0, maxWidth, 1), unformattedText, tags).text.size();
			}
		}
		return 0;
	}
	
	float Renderer::getTextWidthUnformatted(chstr fontName, chstr text)
	{
		if (text == "")
		{
			return 0.0f;
		}
		harray<FormatTag> tags = prepareTags(fontName);
		return getFittingLine(grect(0, 0, 100000, 1), text, tags).rect.w;
	}

	float Renderer::getTextHeightUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		if (text == "")
		{
			return 0.0f;
		}
		harray<FormatTag> tags = prepareTags(fontName);
		harray<RenderLine> lines = createRenderLines(grect(0, 0, maxWidth, 100000), text, tags, LEFT_WRAPPED, TOP);
		return (lines.size() * getFontResource(fontName)->getLineHeight());
	}
	
	int Renderer::getTextCountUnformatted(chstr fontName, chstr text, float maxWidth)
	{
		if (text == "")
		{
			return 0;
		}
		harray<FormatTag> tags = prepareTags(fontName);
		return getFittingLine(grect(0, 0, maxWidth, 1), text, tags).text.size();
	}
	
	hstr Renderer::prepareFormatting(chstr fontName, chstr text, harray<FormatTag>& tags)
	{
		hstr unformattedText = analyzeFormatting(text, tags);
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
	
	RenderLine Renderer::getFittingLine(grect rect, chstr text, harray<FormatTag> tags)
	{
		harray<FormatTag> stack;
		FormatTag tag;
		FormatTag nextTag = tags.front();
		
		hstr fontName;
		FontResource* fontResource = NULL;
		hmap<unsigned int, CharacterDefinition> characters;
		float lineHeight;
		float scale;
		
		const char* str = text.c_str();
		int i = 0;
		int byteLength;
		unsigned int code;
		float width = 0.0f;
		
		while (i < text.size())
		{
			while (tags.size() > 0 && i >= nextTag.start)
			{
				if (nextTag.type == CLOSE)
				{
					tag = stack.pop_back();
					if (tag.type == FORMAT_FONT)
					{
						fontName = tag.data;
						fontResource = getFontResource(fontName);
						characters = fontResource->getCharacters();
						scale = fontResource->getScale();
					}
				}
				else if (nextTag.type == FORMAT_FONT)
				{
					tag.type = FORMAT_FONT;
					tag.data = fontName;
					stack += tag;
					fontName = nextTag.data;
					if (fontResource == NULL)
					{
						fontResource = getFontResource(fontName);
						lineHeight = fontResource->getLineHeight();
					}
					else
					{
						fontResource = getFontResource(fontName);
					}
					characters = fontResource->getCharacters();
					scale = fontResource->getScale();
				}
				else
				{
					tag.type = FORMAT_NORMAL;
					stack += tag;
				}
				tags.pop_front();
				if (tags.size() > 0)
				{
					nextTag = tags.front();
				}
				else
				{
					nextTag.start = text.size() + 1;
				}
			}
			if (tags.size() == 0)
			{
				nextTag.start = text.size() + 1;
			}
			code = utf8_to_uint(&str[i], &byteLength);
			if (code == '\0' || code == '\n')
			{
				break;
			}
			width += characters[code].aw * scale;
			if (width > rect.w) // line is full
			{
				width -= characters[code].aw * scale;
				break;
			}
			i += byteLength;
		}
		RenderLine result;
		result.start = 0;
		result.text = (i == 0 ? "" : text(0, i));
		result.rect = grect(0, 0, width, lineHeight);
		return result;
	}
	


}
