/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "FontResource.h"

namespace atres
{
	FontResource::FontResource(chstr name)
	{
		this->name = name;
	}
	/*
	FontResource::FontResource(FontResource& f, float scale)
	{
		// todo: copy constructor
	}
	*/

	FontResource::~FontResource()
	{
		this->characters.clear();
	}
	
	bool FontResource::hasChar(unsigned int charcode)
	{
		return this->characters.has_key(charcode);
	}
	
	float FontResource::getHeight()
	{
		return (this->height * this->scale * this->baseScale);
	}
	
	float FontResource::getLineHeight()
	{
		return (this->lineHeight * this->scale * this->baseScale);
	}
	
	float FontResource::getScale()
	{
		return (this->scale * this->baseScale);
	}
	
	float FontResource::getTextWidth(chstr text)
	{
		const char* str = text.c_str();
		float result = 0.0f;
		unsigned int code;
		int byteLength;
		int i = 0;
		while (i < text.size())
		{
			code = utf8_to_uint(&str[i], &byteLength);
			result += this->characters[code].aw * this->getScale();
			i += byteLength;
		}
		return result;
	}
	
	int FontResource::getTextCount(chstr text, float maxWidth)
	{
		const char* str = text.c_str();
		unsigned int code;
		float width = 0.0f;
		int byteLength;
		int i = 0;
		while (i < text.size())
		{
			code = utf8_to_uint(&str[i], &byteLength);
			width += this->characters[code].aw * this->getScale();
			if (width > maxWidth)
			{
				break;
			}
			i += byteLength;
		}
		return i;
	}
	
	RenderRectangle FontResource::makeRenderRectangle(grect rect, grect area, unsigned int code)
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
