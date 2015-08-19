/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines utility classes and enums used by the font renderer.

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
	
	class atresExport CharacterDefinition
	{
	public:
		grect rect;
		gvec2 bearing;
		float advance;
		
		CharacterDefinition();
		~CharacterDefinition();

	};
	
	class atresExport IconDefinition
	{
	public:
		grect rect;

		IconDefinition();
		~IconDefinition();

	};

	class atresExport RenderRectangle
	{
	public:
		grect src;
		grect dest;

		RenderRectangle();
		~RenderRectangle();

	};
	
	class atresExport RenderSequence
	{
	public:
		april::Texture* texture;
		april::Color color;
		harray<april::TexturedVertex> vertices;
		
		RenderSequence();
		~RenderSequence();

		/// @note Not thread-safe!
		void addRenderRectangle(RenderRectangle rect);

	};
	
	class atresExport RenderWord
	{
	public:
		hstr text;
		grect rect;
		int start;
		int count;
		int spaces;
		float fullWidth;
		harray<float> charWidths;
		
		RenderWord();
		~RenderWord();

	};
	
	class atresExport RenderLine
	{
	public:
		hstr text;
		grect rect;
		int start;
		int count;
		int spaces;
		bool terminated;
		harray<RenderWord> words;
		
		RenderLine();
		~RenderLine();

	};
	
	class atresExport RenderText
	{
	public:
		harray<RenderLine> lines;
		harray<RenderSequence> textSequences;
		harray<RenderSequence> shadowSequences;
		harray<RenderSequence> borderSequences;

		RenderText();
		~RenderText();

	};

	class atresExport FormatTag
	{
	public:
		FormatTagType type;
		hstr data;
		int start;
		int count;
		
		FormatTag();
		~FormatTag();

	};

	class atresExport TextureContainer
	{
	public:
		april::Texture* texture;
		harray<unsigned int> characters;
		harray<hstr> icons;

		TextureContainer();
		~TextureContainer();

	};

	class CacheEntryBasicText
	{
	public:
		hstr text;
		hstr fontName;
		grect rect;
		Alignment horizontal;
		Alignment vertical;
		april::Color color;
		gvec2 offset;

		CacheEntryBasicText();
		virtual ~CacheEntryBasicText();

		void set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset);
		bool operator==(const CacheEntryBasicText& other) const;
		bool operator!=(const CacheEntryBasicText& other) const;
		unsigned int hash() const;

	};

	class CacheEntryText : public CacheEntryBasicText
	{
	public:
		RenderText value;

		CacheEntryText();
		~CacheEntryText();

		bool operator==(const CacheEntryText& other) const;
		bool operator!=(const CacheEntryText& other) const;

	};

	class CacheEntryLines : public CacheEntryBasicText
	{
	public:
		harray<RenderLine> value;

		CacheEntryLines();
		~CacheEntryLines();

		bool operator==(const CacheEntryLines& other) const;
		bool operator!=(const CacheEntryLines& other) const;

	};

	class CacheEntryLine
	{
	public:
		hstr text;
		hstr fontName;
		gvec2 size;
		RenderLine value;

		CacheEntryLine();
		~CacheEntryLine();

		void set(hstr text, hstr fontName, gvec2 size);
		bool operator==(const CacheEntryLine& other) const;
		bool operator!=(const CacheEntryLine& other) const;
		unsigned int hash() const;

	};

};

#endif

