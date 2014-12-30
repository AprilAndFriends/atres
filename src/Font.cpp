/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Font.h"

namespace atres
{
	Font::Font(chstr name) : height(0.0f), scale(1.0f), baseScale(1.0f),
		lineHeight(0.0f), descender(0.0f), customDescender(false), loaded(false)
	{
		this->name = name;
	}

	Font::~Font()
	{
		this->characters.clear();
		foreach (TextureContainer*, it, this->textureContainers)
		{
			delete (*it)->texture;
			delete (*it);
		}
		this->textureContainers.clear();
	}

	bool Font::_readBasicParameter(chstr line)
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
		if (line.starts_with("Descender="))
		{
			this->descender = (float)line.replace("Descender=", "");
			this->customDescender = true;
			return true;
		}
		return false;
	}
	
	bool Font::hasChar(unsigned int charCode)
	{
		return this->characters.hasKey(charCode);
	}
	
	float Font::getHeight()
	{
		return (this->height * this->scale * this->baseScale);
	}
	
	float Font::getScale()
	{
		return (this->scale * this->baseScale);
	}
	
	float Font::getLineHeight()
	{
		return (this->lineHeight * this->scale * this->baseScale);
	}

	float Font::getDescender()
	{
		return (this->descender * this->scale * this->baseScale);
	}
	
	harray<april::Texture*> Font::getTextures()
	{
		harray<april::Texture*> result;
		foreach (TextureContainer*, it, this->textureContainers)
		{
			result += (*it)->texture;
		}
		return result;
	}
	
	april::Texture* Font::getTexture(unsigned int charCode)
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

	float Font::getTextWidth(chstr text)
	{
		// using static definitions to avoid memory allocation for optimization
		static float textX = 0.0f;
		static float textW = 0.0f;
		static float ax = 0.0f;
		static float aw = 0.0f;
		static float scale = 1.0f;
		static CharacterDefinition* character = NULL;
		static std::basic_string<unsigned int> chars;
		textX = 0.0f;
		textW = 0.0f;
		ax = 0.0f;
		aw = 0.0f;
		scale = this->getScale();
		chars = text.u_str();
		for_itert (unsigned int, i, 0, chars.size())
		{
			character = &this->characters[chars[i]];
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
		}
		return textW;
	}
	
	int Font::getTextCount(chstr text, float maxWidth)
	{
		// using static definitions to avoid memory allocation for optimization
		static float textX = 0.0f;
		static float textW = 0.0f;
		static float ax = 0.0f;
		static float aw = 0.0f;
		static float scale = 1.0f;
		static CharacterDefinition* character = NULL;
		static std::basic_string<unsigned int> chars;
		textX = 0.0f;
		textW = 0.0f;
		ax = 0.0f;
		aw = 0.0f;
		scale = this->getScale();
		chars = text.u_str();
		for_itert (unsigned int, i, 0, chars.size())
		{
			character = &this->characters[chars[i]];
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
				return text.utf8_substr(0, i).size();
			}
		}
		return text.size();
	}
	
	RenderRectangle Font::makeRenderRectangle(const grect& rect, grect area, unsigned int code)
	{
		static RenderRectangle result;
		result.src.set(0.0f, 0.0f, 0.0f, 0.0f);
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
			static april::Texture* texture = NULL;
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
