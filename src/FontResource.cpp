/// @file
/// @author  Boris Mikic
/// @version 2.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "FontResource.h"

namespace atres
{
	FontResource::FontResource(chstr name) : height(0.0f), scale(1.0f), baseScale(1.0f),
		lineHeight(0.0f), correctedHeight(0.0f), loaded(false)
	{
		this->name = name;
	}

	FontResource::~FontResource()
	{
		this->characters.clear();
		foreach (TextureContainer*, it, this->textureContainers)
		{
			delete (*it)->texture;
			delete (*it);
		}
		this->textureContainers.clear();
	}

	bool FontResource::_readBasicParameter(chstr line)
	{
		if (line.starts_with("Name="))
		{
			this->name = line.replace("Name=", "");
			return true;
		}
		if (line.starts_with("Height="))
		{
			this->height = (float)line.replace("Height=", "");
			return true;
		}
		if (line.starts_with("Scale="))
		{
			this->baseScale = (float)line.replace("Scale=", "");
			return true;
		}
		if (line.starts_with("LineHeight="))
		{
			this->lineHeight = (float)line.replace("LineHeight=", "");
			return true;
		}
		if (line.starts_with("CorrectedHeight="))
		{
			this->correctedHeight = (float)line.replace("CorrectedHeight=", "");
			return true;
		}
		return false;
	}
	
	bool FontResource::hasChar(unsigned int charcode)
	{
		return this->characters.has_key(charcode);
	}
	
	float FontResource::getHeight()
	{
		return (this->height * this->scale * this->baseScale);
	}
	
	float FontResource::getScale()
	{
		return (this->scale * this->baseScale);
	}
	
	float FontResource::getLineHeight()
	{
		return (this->lineHeight * this->scale * this->baseScale);
	}

	float FontResource::getCorrectedHeight()
	{
		return (this->correctedHeight * this->scale * this->baseScale);
	}
	
	harray<april::Texture*> FontResource::getTextures()
	{
		harray<april::Texture*> result;
		foreach (TextureContainer*, it, this->textureContainers)
		{
			result += (*it)->texture;
		}
		return result;
	}
	
	april::Texture* FontResource::getTexture(unsigned int charcode)
	{
		foreach (TextureContainer*, it, this->textureContainers)
		{
			if ((*it)->characters.contains(charcode))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	float FontResource::getTextWidth(chstr text)
	{
		const char* str = text.c_str();
		float textX = 0.0f;
		float textW = 0.0f;
		float ax = 0.0f;
		float aw = 0.0f;
		unsigned int code;
		int byteLength;
		int i = 0;
		float scale = this->getScale();
		CharacterDefinition* character;
		while (i < text.size())
		{
			code = utf8_to_uint(&str[i], &byteLength);
			character = &this->characters[code];
			if (textX < -character->bx * scale)
			{
				ax = (character->aw - character->bx) * scale;
				aw = character->w * scale;
			}
			else
			{
				ax = character->aw * scale;
				aw = (character->w + character->bx) * scale;
			}
			textW = textX + hmax(ax, aw);
			textX += ax;
			i += byteLength;
		}
		return textW;
	}
	
	int FontResource::getTextCount(chstr text, float maxWidth)
	{
		const char* str = text.c_str();
		unsigned int code;
		float textX = 0.0f;
		float textW = 0.0f;
		float ax = 0.0f;
		float aw = 0.0f;
		int byteLength;
		int i = 0;
		CharacterDefinition* character;
		while (i < text.size())
		{
			code = utf8_to_uint(&str[i], &byteLength);
			character = &this->characters[code];
			if (textX < -character->bx * scale)
			{
				ax = (character->aw - character->bx) * scale;
				aw = character->w * scale;
			}
			else
			{
				ax = character->aw * scale;
				aw = (character->w + character->bx) * scale;
			}
			textW = textX + hmax(ax, aw);
			textX += ax;
			if (textW > maxWidth)
			{
				break;
			}
			i += byteLength;
		}
		return i;
	}
	
	RenderRectangle FontResource::makeRenderRectangle(const grect& rect, grect area, unsigned int code)
	{
		//rect.set(0, 0, 100000, 100000);
		RenderRectangle result;
		float scaledHeight = this->getHeight();
		CharacterDefinition chr = this->characters[code];
		// vertical cutoff of destination rectangle
		float ratioTop = (area.y < rect.y ? (area.y + area.h - rect.y) / scaledHeight : 1.0f);
		float ratioBottom = (rect.y + rect.h < area.y + area.h ? (rect.y + rect.h - area.y) / scaledHeight : 1.0f);
		// destination rectangle
		result.dest.x = area.x;
		result.dest.y = area.y + scaledHeight * (1.0f - ratioTop);
		result.dest.w = area.w;
		result.dest.h = chr.h * scaledHeight / this->height * (ratioTop + ratioBottom - 1.0f);
		if (rect.intersects(result.dest)) // if destination rectangle inside drawing area
		{
			// horizontal cutoff of destination rectangle
			float ratioLeft = (result.dest.x < rect.x ? (result.dest.x + result.dest.w - rect.x) / result.dest.w : 1.0f);
			float ratioRight = (rect.x + rect.w < result.dest.x + result.dest.w ? (rect.x + rect.w - result.dest.x) / result.dest.w : 1.0f);
			result.dest.x = result.dest.x + result.dest.w * (1.0f - ratioLeft);
			result.dest.w = result.dest.w * (ratioLeft + ratioRight - 1.0f);
			// source rectangle
			april::Texture* texture = this->getTexture(code);
			float tiw = 1.0f / texture->getWidth();
			float tih = 1.0f / texture->getHeight();
			result.src.x = (chr.x + chr.w * (1.0f - ratioLeft)) * tiw;
			result.src.y = (chr.y + chr.h * (1.0f - ratioTop)) * tih;
			result.src.w = (chr.w * (ratioLeft + ratioRight - 1.0f)) * tiw;
			result.src.h = (chr.h * (ratioTop + ratioBottom - 1.0f)) * tih;
		}
		return result;
	}
	
}
