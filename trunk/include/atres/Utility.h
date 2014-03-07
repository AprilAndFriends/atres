/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.3
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
#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"

namespace atres
{
	enum Alignment
	{
		TOP, // top
		CENTER, // mid
		BOTTOM, // bot
		LEFT,
		RIGHT,
		LEFT_WRAPPED,
		RIGHT_WRAPPED,
		CENTER_WRAPPED,
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
		TAG_TYPE_ESCAPE,
		TAG_TYPE_FONT,
		TAG_TYPE_COLOR,
		TAG_TYPE_SCALE,
		TAG_TYPE_NORMAL,
		TAG_TYPE_SHADOW,
		TAG_TYPE_BORDER,
		TAG_TYPE_IGNORE_FORMATTING,
		TAG_TYPE_CLOSE
	};
	
	struct atresExport CharacterDefinition
	{
		float x;
		float y;
		float w;
		float h;
		float bx;
		float by;
		float aw;
		
		CharacterDefinition();
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
		harray<april::TexturedVertex> vertices;
		
		RenderSequence();
		/// @note Not thread-safe!
		void addRenderRectangle(RenderRectangle rect);
	};
	
	struct RenderText
	{
		harray<RenderSequence> textSequences;
		harray<RenderSequence> shadowSequences;
		harray<RenderSequence> borderSequences;
	};

	struct RenderWord
	{
		hstr text;
		grect rect;
		int start;
		int count;
		int spaces;
		float fullWidth;
		harray<float> charWidths;
		
		RenderWord();
	};
	
	struct RenderLine
	{
		hstr text;
		grect rect;
		int start;
		int count;
		int spaces;
		bool terminated;
		harray<RenderWord> words;
		
		RenderLine();
	};
	
	struct FormatTag
	{
		FormatTagType type;
		hstr data;
		int start;
		int count;
		
		FormatTag();
	};
	
	struct CacheEntryText
	{
		hstr text;
		hstr fontName;
		grect rect;
		Alignment horizontal;
		Alignment vertical;
		april::Color color;
		gvec2 offset;
		RenderText value;
		
		CacheEntryText();
		void set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset);
		bool operator==(const CacheEntryText& other) const;
		unsigned int hash() const;
	};
	
	struct CacheEntryLine
	{
		hstr text;
		hstr fontName;
		gvec2 size;
		RenderLine value;
		
		void set(hstr text, hstr fontName, gvec2 size);
		bool operator==(const CacheEntryLine& other) const;
		unsigned int hash() const;
	};
	
	struct atresExport TextureContainer
	{
		april::Texture* texture;
		harray<unsigned int> characters;
		
		TextureContainer();
	};

};

#endif

