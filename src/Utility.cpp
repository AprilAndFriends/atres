/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Utility.h"

namespace atres
{
	CharacterDefinition::CharacterDefinition() : x(0.0f), y(0.0f), w(0.0f), h(0.0f), bx(0.0f), aw(0.0f)
	{
	}

	RenderSequence::RenderSequence() : texture(NULL)
	{
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

	void CacheKeySequence::set(hstr text, hstr fontName, gvec2 size, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		this->text = text;
		this->fontName = fontName;
		this->size = size;
		this->horizontal = horizontal;
		this->vertical = vertical;
		this->color = color;
		this->offset = offset;
	}

	bool CacheKeySequence::operator==(const CacheKeySequence& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->size == other.size &&
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
