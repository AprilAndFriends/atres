/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/Color.h>
#include <april/RenderSystem.h>
#include <hltypes/hstring.h>

#include "Utility.h"

namespace atres
{
	static april::TexturedVertex vert[6];

	CharacterDefinition::CharacterDefinition() : advance(0.0f), offsetY(0.0f)
	{
	}

	CharacterDefinition::~CharacterDefinition()
	{
	}

	IconDefinition::IconDefinition()
	{
	}

	IconDefinition::~IconDefinition()
	{
	}

	RenderRectangle::RenderRectangle()
	{
	}

	RenderRectangle::~RenderRectangle()
	{
	}

	RenderSequence::RenderSequence() : texture(NULL)
	{
	}

	RenderSequence::~RenderSequence()
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
	
	RenderWord::RenderWord() : start(0), count(0), spaces(0), icon(false), fullWidth(0.0f)
	{
	}

	RenderWord::~RenderWord()
	{
	}

	RenderLine::RenderLine() : start(0), count(0), spaces(0), terminated(false)
	{
	}
	
	RenderLine::~RenderLine()
	{
	}

	RenderText::RenderText()
	{
	}

	RenderText::~RenderText()
	{
	}

	FormatTag::FormatTag() : type(TAG_TYPE_ESCAPE), start(0), count(0)
	{
	}

	FormatTag::~FormatTag()
	{
	}

	TextureContainer::TextureContainer() : texture(NULL)
	{
	}

	TextureContainer::~TextureContainer()
	{
		if (this->texture != NULL)
		{
			delete this->texture;
		}
	}

	CacheEntryBasicText::CacheEntryBasicText() : horizontal(CENTER_WRAPPED), vertical(CENTER)
	{
	}

	CacheEntryBasicText::~CacheEntryBasicText()
	{
	}

	void CacheEntryBasicText::set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
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

	bool CacheEntryBasicText::operator==(const CacheEntryBasicText& other) const
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

	bool CacheEntryBasicText::operator!=(const CacheEntryBasicText& other) const
	{
		return !(*this == other);
	}

	unsigned int CacheEntryBasicText::hash() const
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
		// this code with fvar is used to avoid strict aliasing violations
		const float* fvar = NULL;
		fvar = &this->rect.x;	result ^= *((unsigned int*)fvar);
		fvar = &this->rect.y;	result ^= *((unsigned int*)fvar);
		fvar = &this->rect.w;	result ^= *((unsigned int*)fvar);
		fvar = &this->rect.h;	result ^= *((unsigned int*)fvar);
		result ^= (((unsigned int)(this->vertical)) & 0xFFFF);
		result ^= (((unsigned int)(this->horizontal) << 16) & 0xFFFF);
		result ^= (this->color.r << 8);
		result ^= (this->color.g << 16);
		result ^= (this->color.b << 24);
		fvar = &this->offset.x;	result ^= *((unsigned int*)fvar);
		fvar = &this->offset.y;	result ^= *((unsigned int*)fvar);
		return result;
	}

	CacheEntryText::CacheEntryText() : CacheEntryBasicText()
	{
	}

	CacheEntryText::~CacheEntryText()
	{
	}

	bool CacheEntryText::operator==(const CacheEntryText& other) const
	{
		return CacheEntryBasicText::operator==(other);
	}

	bool CacheEntryText::operator!=(const CacheEntryText& other) const
	{
		return CacheEntryBasicText::operator!=(other);
	}

	CacheEntryLines::CacheEntryLines() : CacheEntryBasicText()
	{
	}

	CacheEntryLines::~CacheEntryLines()
	{
	}

	bool CacheEntryLines::operator==(const CacheEntryLines& other) const
	{
		return CacheEntryBasicText::operator==(other);
	}

	bool CacheEntryLines::operator!=(const CacheEntryLines& other) const
	{
		return CacheEntryBasicText::operator!=(other);
	}

	CacheEntryLine::CacheEntryLine()
	{
	}

	CacheEntryLine::~CacheEntryLine()
	{
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

	bool CacheEntryLine::operator!=(const CacheEntryLine& other) const
	{
		return !(*this == other);
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
		// this code with fvar is used to avoid strict aliasing violations
		const float* fvar = NULL;
		fvar = &this->size.x;	result ^= *((unsigned int*)fvar);
		fvar = &this->size.y;	result ^= *((unsigned int*)fvar);
		return result;
	}
	
}
