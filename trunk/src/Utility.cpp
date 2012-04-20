/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>

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
		this->vertexes.add(vert, 6);
	}
	
	RenderWord::RenderWord() : start(0), spaces(0), fullWidth(0.0f)
	{
	}
	
	RenderLine::RenderLine() : spaces(0), terminated(false)
	{
	}
	
	FormatTag::FormatTag() : type(ESCAPE), start(0), count(0)
	{
	}

	CacheKeySequence::CacheKeySequence() : horizontal(CENTER_WRAPPED), vertical(CENTER)
	{
	}

	void CacheKeySequence::set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		this->text = text;
		this->fontName = fontName;
		this->rect = rect;
		this->horizontal = horizontal;
		this->vertical = vertical;
		this->color = color;
		this->offset = offset;
	}

	bool CacheKeySequence::operator==(const CacheKeySequence& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->rect == other.rect &&
			this->horizontal == other.horizontal &&
			this->vertical == other.vertical &&
			this->color == other.color &&
			this->offset == other.offset);
	}
	
	void CacheKeyLine::set(hstr text, hstr fontName, gvec2 size)
	{
		this->text = text;
		this->fontName = fontName;
		this->size = size;
	}

	bool CacheKeyLine::operator==(const CacheKeyLine& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->size == other.size);
	}
	
}
