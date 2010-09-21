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
#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "Font.h"

#define CHR_BUFFER_MAX 32768

namespace Atres
{
	CharacterRenderOp rops[CHR_BUFFER_MAX];
	int nOps=0;
	
	//2DO
	void flushRenderOperations()
	{
		if (nOps)
		{
			April::TexturedVertex v[CHR_BUFFER_MAX];
			//April::Texture* t = rendersys->getTe g_font_textures[rops[0].resource];
			April::Texture* t = rops[0].texture;
			int i=0;
			float w=(float)t->getWidth(),h=(float)t->getHeight();
			for (Atres::CharacterRenderOp* op=rops;op < rops+nOps;op++)
			{
				v[i].x=op->dest.x;              v[i].y = op->dest.y;              v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x=op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
			}
			April::rendersys->setTexture(t);
			April::rendersys->render(April::TriangleList,v,i,rops[0].color.r_float(),rops[0].color.g_float(),rops[0].color.b_float(),rops[0].color.a_float());
			/*
			rendersys->render()
			getRenderInterface()->render(rops,nOps);
			*/
			nOps=0;
		}
	}
	
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
	
	void Font::aligmentCorrection(grect rect, Alignment horizontal, Alignment vertical, harray<hstr>* lines, harray<grect>* areas)
	{
		float lineHeight = this->getLineHeight();
		// vertical correction
		if (vertical != Atres::TOP)
		{
			float difference = lines->size() * lineHeight - rect.h;
			if (vertical == Atres::CENTER)
			{
				difference /= 2;
				while (lines->size() > 0 && (lines->size() - 2) * lineHeight >= rect.h)
				{
					lines->pop_front();
					areas->pop_front();
					if (lines->size() > 0)
					{
						lines->pop_back();
						areas->pop_back();
					}
				}
			}
			else if (vertical == Atres::BOTTOM)
			{
				while (lines->size() > 0 && (lines->size() - 1) * lineHeight >= rect.h)
				{
					lines->pop_front();
					areas->pop_front();
				}
			}
			foreach (grect, it, *areas)
			{
				(*it).y -= difference;
			}
		}
		// horizontal correction
		if (areas != NULL && horizontal != LEFT && horizontal != LEFT_WRAPPED)
		{
			if (horizontal == RIGHT || horizontal == RIGHT_WRAPPED)
			{
				foreach (grect, it, *areas)
				{
					(*it).x += rect.w - (*it).w;
				}
			}
			else if (horizontal == CENTER || horizontal == CENTER_WRAPPED)
			{
				foreach (grect, it, *areas)
				{
					(*it).x += (rect.w - (*it).w) / 2;
				}
			}
		}
	}
	
	harray<hstr> Font::testRender(grect rect, chstr text, Alignment horizontal, Alignment vertical, harray<grect>* areas)
	{
		bool wrapped = (horizontal == LEFT_WRAPPED || horizontal == RIGHT_WRAPPED || horizontal == CENTER_WRAPPED);
		harray<hstr> result;
		const char* str = text.c_str();
		int byteLength;
		bool checkingSpaces;
		float lineHeight = this->getLineHeight();
		float width;
		float offset;
		unsigned int code = 0;
		int i = 0;
		int start = 0;
		int current = 0;
		// extraction of all lines
		while (i < text.size())
		{
			i = start + current;
			while (i < text.size() && str[i] == ' ') // skip initial spaces in the line
			{
				i++;
			}
			start = i;
			current = 0;
			width = 0.0f;
			offset = 0.0f;
			checkingSpaces = false;
			while (true) // checking how much fits into this line
			{
				code = getCharUtf8(&str[i], &byteLength);
				if (code == '\n')
				{
					width = offset;
					i += byteLength;
					current = i - start;
					break;
				}
				if (code == ' ' || code == '\0')
				{
					if (!checkingSpaces)
					{
						width = offset;
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
				offset += this->characters[code].aw * this->scale;
				if (wrapped && offset > rect.w) // current word doesn't fit anymore
				{
					if (current == 0) // whole word doesn't fit into line, just chop it off
					{
						width = offset - this->characters[code].aw * this->scale;;
						current = i - start;
					}
					break;
				}
				i += byteLength;
			}
			// horizontal offset
			if (areas != NULL)
			{
				*areas += grect(rect.x, rect.y + result.size() * lineHeight, width, lineHeight);
			}
			result += (current > 0 ? text(start, current).trim() : "");
			if (vertical == Atres::TOP && result.size() * lineHeight >= rect.h)
			{
				break;
			}
		}
		this->aligmentCorrection(rect, horizontal, vertical, &result, areas);
		return result;
	}
	
	void Font::render(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color)
	{
		CharacterRenderOp* op = rops + nOps;
		harray<grect> areas;
		harray<hstr> lines = this->testRender(rect, text, horizontal, vertical, &areas);
		float height = this->getHeight();
		float lineHeight = this->getLineHeight();
		if (lines.size() > 0)
		{
			int byteLength;
			FontCharDef chr;
			unsigned int code;
			float width;
			float ratioTop;
			float ratioBottom;
			for (int i = 0; i < lines.size(); i++)
			{
				width = 0.0f;
				ratioTop = (areas[i].y < rect.y ? (areas[i].y + areas[i].h - rect.y) / lineHeight : 1.0f);
				ratioBottom = (rect.y + rect.h < areas[i].y + areas[i].h ? (rect.y + rect.h - areas[i].y) / lineHeight : 1.0f);
				for (int j = 0; j < lines[i].size(); j += byteLength, op++, nOps++)
				{
					code = getCharUtf8(&lines[i][j], &byteLength);
					chr = this->characters[code];
					op->texture = this->texture;
					op->color = color;
					op->italic = op->underline = op->strikethrough = false;
					op->src.x = chr.x;
					op->src.y = chr.y + this->height * (1.0f - ratioTop);
					op->src.w = chr.w;
					op->src.h = this->height * (ratioTop + ratioBottom - 1.0f);
					op->dest.x = areas[i].x + width;
					op->dest.y = areas[i].y + height * (1.0f - ratioTop);
					op->dest.w = chr.w * this->scale;
					op->dest.h = height * (ratioTop + ratioBottom - 1.0f);
					width += chr.aw * this->scale;
				}
			}
		}
	}
}
