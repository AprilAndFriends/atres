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
/// Represents a font renderer.

#ifndef ATRES_RENDERER_H
#define ATRES_RENDERER_H

#include <april/Color.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresExport.h"

#define BUFFER_MAX_CHARACTERS 8192

namespace atres
{
	enum Alignment
	{
		LEFT,
		RIGHT,
		CENTER,
		LEFT_WRAPPED,
		RIGHT_WRAPPED,
		CENTER_WRAPPED,
		TOP,
		BOTTOM
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
	
	struct RenderLine
	{
		hstr text;
		grect rect;
		int start;
		RenderLine() : start(0) { }
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

	class atresExport Renderer
	{
	public:
		Renderer() { }
		~Renderer() { }

	};
	
};

#endif

