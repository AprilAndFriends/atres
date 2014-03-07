/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/Color.h>
#include <april/RenderSystem.h>
#include <hltypes/hstring.h>

#include "Utility.h"

namespace atres
{
	static april::TexturedVertex vert[6];

	CharacterDefinition::CharacterDefinition() : x(0.0f), y(0.0f), w(0.0f), h(0.0f), bx(0.0f), aw(0.0f)
	{
	}

	RenderSequence::RenderSequence() : texture(NULL)
	{
	}

	void RenderSequence::addRenderRectangle(RenderRectangle rect)
	{
		vert[0].x = rect.dest.left();	vert[0].y = rect.dest.top();	vert[0].u = rect.src.left();	vert[0].v = rect.src.top();
		vert[1].x = rect.dest.right();	vert[1].y = rect.dest.top();	vert[1].u = rect.src.right();	vert[1].v = rect.src.top();
		vert[2].x = rect.dest.left();	vert[2].y = rect.dest.bottom();	vert[2].u = rect.src.left();	vert[2].v = rect.src.bottom();
		vert[3].x = rect.dest.right();	vert[3].y = rect.dest.top();	vert[3].u = rect.src.right();	vert[3].v = rect.src.top();
		vert[4].x = rect.dest.right();	vert[4].y = rect.dest.bottom();	vert[4].u = rect.src.right();	vert[4].v = rect.src.bottom();
		vert[5].x = rect.dest.left();	vert[5].y = rect.dest.bottom();	vert[5].u = rect.src.left();	vert[5].v = rect.src.bottom();
		this->vertices.add(vert, 6);
	}
	
	RenderWord::RenderWord() : start(0), count(0), spaces(0), fullWidth(0.0f)
	{
	}
	
	RenderLine::RenderLine() : start(0), count(0), spaces(0), terminated(false)
	{
	}
	
	FormatTag::FormatTag() : type(TAG_TYPE_ESCAPE), start(0), count(0)
	{
	}

	CacheEntryText::CacheEntryText() : horizontal(CENTER_WRAPPED), vertical(CENTER)
	{
	}

	void CacheEntryText::set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		this->text = text;
		this->fontName = fontName;
		this->rect = rect;
		this->horizontal = horizontal;
		this->vertical = vertical;
		// overrides alpha to 255 because of a conflict within AprilUI that would cause alpha-animated
		// text to be cached for every possible alpha value (0-255) and rendered very slowly
		this->color = april::Color(color, 255);
		this->offset = offset;
	}

	bool CacheEntryText::operator==(const CacheEntryText& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->rect == other.rect &&
			this->horizontal == other.horizontal &&
			this->vertical == other.vertical &&
			this->color.r == other.color.r &&
			this->color.g == other.color.g &&
			this->color.b == other.color.b &&
			this->offset == other.offset);
	}

	unsigned int CacheEntryText::hash() const
	{
		unsigned int result = 0xFFFFFFFF;
		for_iter (i, 0, this->text.size())
		{
			result ^= this->text[i] << ((i % 4) * 8);
		}
		for_iter (i, 0, this->fontName.size())
		{
			result ^= this->fontName[i] << ((i % 4) * 8);
		}
		result ^= *((unsigned int*)(&this->rect.x));
		result ^= *((unsigned int*)(&this->rect.y));
		result ^= *((unsigned int*)(&this->rect.w));
		result ^= *((unsigned int*)(&this->rect.h));
		result ^= (((unsigned int)(this->vertical)) & 0xFFFF);
		result ^= (((unsigned int)(this->horizontal) << 16) & 0xFFFF);
		result ^= (this->color.r << 8);
		result ^= (this->color.g << 16);
		result ^= (this->color.b << 24);
		result ^= *((unsigned int*)(&this->offset.x));
		result ^= *((unsigned int*)(&this->offset.y));
		return result;
	}
	
	void CacheEntryLine::set(hstr text, hstr fontName, gvec2 size)
	{
		this->text = text;
		this->fontName = fontName;
		this->size = size;
	}

	bool CacheEntryLine::operator==(const CacheEntryLine& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->size == other.size);
	}
	
	unsigned int CacheEntryLine::hash() const
	{
		unsigned int result = 0xFFFFFFFF;
		for_iter (i, 0, this->text.size())
		{
			result ^= this->text[i] << ((i % 4) * 8);
		}
		for_iter (i, 0, this->fontName.size())
		{
			result ^= this->fontName[i] << ((i % 4) * 8);
		}
		result ^= *((unsigned int*)(&this->size.x));
		result ^= *((unsigned int*)(&this->size.y));
		return result;
	}
	
	TextureContainer::TextureContainer() : texture(NULL)
	{
	}

}
