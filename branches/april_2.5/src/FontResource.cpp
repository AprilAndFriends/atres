/// @file
/// @author  Boris Mikic
/// @version 3.0
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
	
	bool FontResource::hasChar(unsigned int charCode)
	{
		return this->characters.has_key(charCode);
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
	
	april::Texture* FontResource::getTexture(unsigned int charCode)
	{
		foreach (TextureContainer*, it, this->textureContainers)
		{
			if ((*it)->characters.contains(charCode))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	float FontResource::getTextWidth(chstr text)
	{
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
			code = text.first_unicode_char(i, &byteLength);
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
			code = text.first_unicode_char(i, &byteLength);
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
		RenderRectangle result;
		result.dest = area;
		// if destination rectangle not entirely inside drawing area
		if (rect.intersects(result.dest))
		{
			static gvec2 fullSize(1.0f, 1.0f);
			static gvec2 leftTop;
			static gvec2 rightBottom;
			static gvec2 textureInvertedSize;
			static CharacterDefinition* chr = NULL;
			static grect charRect;
			static april::Texture* texture;
			texture = this->getTexture(code);
			textureInvertedSize.set(1.0f / texture->getWidth(), 1.0f / texture->getHeight());
			chr = &this->characters[code];
			charRect.set(chr->x, chr->y, chr->w, chr->h);
			// vertical/horizontal cutoff of destination rectangle (using left/right/top/bottom semantics for consistency)
			leftTop.x = (area.left() < rect.left() ? (area.right() - rect.left()) / area.w : fullSize.x);
			leftTop.y = (area.top() < rect.top() ? (area.bottom() - rect.top()) / area.h : fullSize.y);
			rightBottom.x = (rect.right() < area.right() ? (rect.right() - area.left()) / area.w : fullSize.x);
			rightBottom.y = (rect.bottom() < area.bottom() ? (rect.bottom() - area.top()) / area.h : fullSize.y);
			// apply cutoff on destination
			result.dest.setPosition(area.getPosition() + area.getSize() * (fullSize - leftTop));
			result.dest.setSize(area.getSize() * (leftTop + rightBottom - fullSize));
			// apply cutoff on source
			result.src.setPosition((charRect.getPosition() + charRect.getSize() * (fullSize - leftTop)) * textureInvertedSize);
			result.src.setSize((charRect.getSize() * (leftTop + rightBottom - fullSize)) * textureInvertedSize);
		}
		return result;
	}
	
}
