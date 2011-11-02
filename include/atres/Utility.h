/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines utility structs and enums used by the font renderer.

#ifndef ATRES_UTILITY_H
#define ATRES_UTILITY_H

#include <april/Color.h>
#include <april/Texture.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

namespace atres
{
	struct atresExport CharacterDefinition
	{
		float x;
		float y;
		float w;
		float h;
		float bx;
		float aw;
	};

	enum Alignment
	{
		LEFT,
		RIGHT,
		CENTER,
		LEFT_WRAPPED,
		RIGHT_WRAPPED,
		CENTER_WRAPPED,
		TOP,
		BOTTOM,
		JUSTIFIED
	};

	enum Effect
	{
		NONE,
		SHADOW,
		BORDER
	};
	
	enum FormatTagType
	{
		ESCAPE,
		FORMAT_NORMAL,
		FORMAT_SHADOW,
		FORMAT_BORDER,
		FORMAT_COLOR,
		FORMAT_FONT,
		CLOSE
	};
	
	struct RenderRectangle
	{
		grect src;
		grect dest;
	};
	
	struct RenderSequence
	{
		april::Texture* texture;
		april::Color color;
		harray<RenderRectangle> rectangles;
		RenderSequence() : texture(NULL) { }
	};
	
	struct RenderWord
	{
		hstr text;
		grect rect;
		int start;
		int spaces;
		RenderWord() : start(0), spaces(0) { }
	};
	
	struct RenderLine
	{
		hstr text;
		grect rect;
		int spaces;
		bool terminated;
		harray<RenderWord> words;
		RenderLine() : spaces(0), terminated(false) { }
	};
	
	struct FormatTag
	{
		FormatTagType type;
		hstr data;
		int start;
		int count;
		FormatTag() : type(ESCAPE), start(0), count(0) { }
	};
	
	struct CacheEntry
	{
		hstr fontName;
		int index;
		gvec2 size;
		Alignment horizontal;
		Alignment vertical;
		april::Color color;
		gvec2 offset;
		harray<RenderSequence> sequences;
		CacheEntry() : index(0), horizontal(CENTER_WRAPPED), vertical(CENTER) { }
	};

	struct CacheUnformattedEntry
	{
		hstr fontName;
		int index;
		gvec2 size;
		Alignment horizontal;
		Alignment vertical;
		april::Color color;
		gvec2 offset;
		harray<RenderSequence> sequences;
		CacheUnformattedEntry() : index(0), horizontal(CENTER_WRAPPED), vertical(CENTER) { }
	};

	struct CacheLineEntry
	{
		hstr fontName;
		int index;
		gvec2 size;
		RenderLine line;
		CacheLineEntry() : index(0) { }
	};

};

#endif

