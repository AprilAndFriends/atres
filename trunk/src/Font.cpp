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

namespace Atres
{
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
		CharacterDefinition c;
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
	
	RenderRectangle Font::makeRenderRectangle(grect rect, grect area, unsigned int code)
	{
		RenderRectangle result;
		float height = this->getHeight();
		float lineHeight = this->getLineHeight();
		CharacterDefinition chr = this->characters[code];
		// vertical cutoff of destination rectangle
		float ratioTop = (area.y < rect.y ? (area.y + area.h - rect.y) / lineHeight : 1.0f);
		float ratioBottom = (rect.y + rect.h < area.y + area.h ? (rect.y + rect.h - area.y) / lineHeight : 1.0f);
		// destination rectangle
		result.dest.x = area.x;
		result.dest.y = area.y + height * (1.0f - ratioTop);
		result.dest.w = area.w;
		result.dest.h = height * (ratioTop + ratioBottom - 1.0f);
		if (rect.intersects(result.dest)) // if destination rectangle inside drawing area
		{
			// horizontal cutoff of destination rectangle
			float ratioLeft = (result.dest.x < rect.x ? (result.dest.x + result.dest.w - rect.x) / result.dest.w : 1.0f);
			float ratioRight = (rect.x + rect.w < result.dest.x + result.dest.w ? (rect.x + rect.w - result.dest.x) / result.dest.w : 1.0f);
			result.dest.x = result.dest.x + result.dest.w * (1.0f - ratioLeft);
			result.dest.w = result.dest.w * (ratioLeft + ratioRight - 1.0f);
			// source rectangle
			result.src.x = chr.x + chr.w * (1.0f - ratioLeft);
			result.src.y = chr.y + this->height * (1.0f - ratioTop);
			result.src.w = chr.w * (ratioLeft + ratioRight - 1.0f);
			result.src.h = this->height * (ratioTop + ratioBottom - 1.0f);
		}
		return result;
	}
	
}
