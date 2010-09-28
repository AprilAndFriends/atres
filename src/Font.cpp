/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Font.h"

#define CHR_BUFFER_MAX 32768

namespace Atres
{
	CharacterRenderOperation operations[CHR_BUFFER_MAX];
	int operationsCount = 0;
	
	//2DO
	void flushRenderOperations()
	{
		if (operationsCount > 0)
		{
			April::TexturedVertex v[CHR_BUFFER_MAX];
			April::Texture* t = operations[0].texture;
			int i = 0;
			float w = (float)t->getWidth();
			float h = (float)t->getHeight();
			for (Atres::CharacterRenderOperation* op = operations; op < operations + operationsCount; op++)
			{
				v[i].x = op->dest.x;              v[i].y = op->dest.y;              v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = op->src.y / h;               i++;
				v[i].x = op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x = op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x = op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x = op->dest.x + op->dest.w; v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x = op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
			}
			April::rendersys->setTexture(t);
			April::Color color = operations[0].color;
			April::rendersys->render(April::TriangleList, v, i, color.r_float(), color.g_float(), color.b_float(), color.a_float());
			operationsCount = 0;
		}
	}

	Font::Font(chstr filename)
	{
		this->scale = 1.0f;
		this->defaultScale = 1.0f;
		this->lineHeight = 0.0f;
		harray<hstr> lines = hfile::hread(filename).split("\n");
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			if (line.starts_with("Name="))
			{
				this->name = line.replace("Name=", "");
			}
			else if (line.starts_with("Resource="))
			{
				this->texture = April::rendersys->loadTexture(line.replace("Resource=", ""));
				Atres::logMessage(hsprintf("\"Resource=\" is deprecated. Use \"Texture=\" instead. (File: %s)", filename.c_str()));
			}
			else if (line.starts_with("Texture="))
			{
				this->texture = April::rendersys->loadTexture(line.replace("Texture=", ""));
			}
			else if (line.starts_with("LineHeight="))
			{
				this->lineHeight = (float)line.replace("LineHeight=", "");
			}
			else if (line.starts_with("Height="))
			{
				this->height = (float)line.replace("Height=", "");
			}
			else if (line.starts_with("Scale="))
			{
				this->scale = (float)line.replace("Scale=", "");
				this->defaultScale = this->scale;
			}
			else if (line.starts_with("#"))
			{
				continue;
			}
			else if (line.starts_with("-"))
			{
				break;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		FontCharDef c;
		unsigned int code;
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			data = (*it).split(" ");
			if (data.size() == 5)
			{
				code = (unsigned int)data.pop_front();
				c.x = (float)data.pop_front();
				c.y = (float)data.pop_front();
				c.w = (float)data.pop_front();
				c.aw = (float)data.pop_front();
				if (c.aw == 0.0f)
				{
					c.aw = c.w;
				}
				this->characters[code] = c;
			}
		}
	}

	Font::Font(Font& f, float scale)
	{
		// todo: copy constructor
	}

	Font::~Font()
	{
		this->characters.clear();
	}
	
	bool Font::hasChar(unsigned int charcode)
	{
		return this->characters.has_key(charcode);
	}
	
	void Font::setScale(float value)
	{
		this->scale = (value == 0.0f ? this->defaultScale : value);
	}
	
	float Font::getTextWidth(chstr text)
	{
		const char* str = text.c_str();
		float result = 0.0f;
		unsigned int code;
		int byteLength;
		int i = 0;
		while (i < text.size())
		{
			code = getCharUtf8(&str[i], &byteLength);
			result += this->characters[code].aw * this->scale;
			i += byteLength;
		}
		return result;
	}
	
	int Font::getTextCount(chstr text, float maxWidth)
	{
		const char* str = text.c_str();
		unsigned int code;
		float width = 0.0f;
		int byteLength;
		int i = 0;
		while (i < text.size())
		{
			code = getCharUtf8(&str[i], &byteLength);
			width += this->characters[code].aw * this->scale;
			if (width > maxWidth)
			{
				break;
			}
			i += byteLength;
		}
		return i;
	}
	
	int Font::testRender(grect rect, chstr text, Alignment horizontal, Alignment vertical, harray<hstr>& lines, harray<grect>& areas, gvec2 offset)
	{
		bool wrapped = (horizontal == LEFT_WRAPPED || horizontal == RIGHT_WRAPPED || horizontal == CENTER_WRAPPED);
		const char* str = text.c_str();
		int byteLength;
		bool checkingSpaces;
		float lineHeight = this->getLineHeight();
		float width;
		float advance;
		harray<int> counts;
		unsigned int code = 0;
		int i = 0;
		int start = 0;
		int current = 0;
		// extraction of all lines
		while (i < text.size())
		{
			i = start + current;
			counts += current;
			while (i < text.size() && str[i] == ' ') // skip initial spaces in the line
			{
				i++;
			}
			//counts += i - start;
			start = i;
			current = 0;
			width = 0.0f;
			advance = 0.0f;
			checkingSpaces = false;
			while (true) // checking how much fits into this line
			{
				code = getCharUtf8(&str[i], &byteLength);
				if (code == '\n')
				{
					width = advance;
					i += byteLength;
					current = i - start;
					break;
				}
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
				advance += this->characters[code].aw * this->scale;
				if (wrapped && advance > rect.w) // current word doesn't fit anymore
				{
					if (current == 0) // whole word doesn't fit into line, just chop it off
					{
						width = advance - this->characters[code].aw * this->scale;
						current = i - start;
					}
					break;
				}
				i += byteLength;
			}
			counts[counts.size() - 1] += current;
			// horizontal offset
			areas += grect(rect.x, rect.y + lines.size() * lineHeight, width, lineHeight);
			lines += (current > 0 ? text(start, current).trim() : "");
		}
		int count = 0;
		//int count = this->verticalCorrection(rect, vertical, lines, areas, offset);
		//this->horizontalCorrection(rect, horizontal, lines, areas, offset);
		if (count > 0)
		{
			counts.pop_front(count);
		}
		count = counts.size() - lines.size();
		if (count > 0)
		{
			counts.pop_back(count);
		}
		count = 0;
		foreach (int, it, counts)
		{
			count += (*it);
		}
		return hmin(count, text.size());
	}
	
	void Font::render(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		harray<grect> areas;
		harray<hstr> lines;
		this->testRender(rect, text, horizontal, vertical, lines, areas, offset);
		this->renderRaw(rect, lines, areas, color);
	}
	
	void Font::renderRaw(grect rect, harray<hstr> lines, harray<grect> areas, April::Color color, gvec2 offset)
	{
		if (lines.size() == 0)
		{
			return;
		}
		CharacterRenderOperation* op = operations + operationsCount;
		float height = this->getHeight();
		float lineHeight = this->getLineHeight();
		FontCharDef chr;
		unsigned int code;
		int byteLength;
		float width;
		float ratioTop;
		float ratioBottom;
		float ratioLeft;
		float ratioRight;
		rect += offset;
		foreach (grect, it, areas)
		{
			(*it) += offset;
		}
		for (int i = 0; i < lines.size(); i++)
		{
			width = 0.0f;
			// vertical cutoff of destination rectangle
			ratioTop = (areas[i].y < rect.y ? (areas[i].y + areas[i].h - rect.y) / lineHeight : 1.0f);
			ratioBottom = (rect.y + rect.h < areas[i].y + areas[i].h ? (rect.y + rect.h - areas[i].y) / lineHeight : 1.0f);
			for (int j = 0; j < lines[i].size(); j += byteLength)
			{
				code = getCharUtf8(&lines[i][j], &byteLength);
				chr = this->characters[code];
				// destination rectangle
				op->dest.x = areas[i].x + width;
				op->dest.y = areas[i].y + height * (1.0f - ratioTop);
				op->dest.w = chr.w * this->scale;
				op->dest.h = height * (ratioTop + ratioBottom - 1.0f);
				if (rect.intersects(op->dest)) // if destination rectangle inside drawing area
				{
					// horizontal cutoff of destination rectangle
					ratioLeft = (op->dest.x < rect.x ? (op->dest.x + op->dest.w - rect.x) / op->dest.w : 1.0f);
					ratioRight = (rect.x + rect.w < op->dest.x + op->dest.w ? (rect.x + rect.w - op->dest.x) / op->dest.w : 1.0f);
					op->dest.x = op->dest.x + op->dest.w * (1.0f - ratioLeft);
					op->dest.w = op->dest.w * (ratioLeft + ratioRight - 1.0f);
					// source rectangle
					op->src.x = chr.x + chr.w * (1.0f - ratioLeft);
					op->src.y = chr.y + this->height * (1.0f - ratioTop);
					op->src.w = chr.w * (ratioLeft + ratioRight - 1.0f);
					op->src.h = this->height * (ratioTop + ratioBottom - 1.0f);
					// everything else
					op->texture = this->texture;
					op->color = color;
					op++;
					operationsCount++;
				}
				width += chr.aw * this->scale;
			}
		}
	}
}
