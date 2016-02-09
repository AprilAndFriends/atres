/// @file
/// @version 4.1
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
#include <hltypes/henum.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"

namespace atres
{
	HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, Horizontal,
	(
		HL_ENUM_DECLARE(Horizontal, Left);
		HL_ENUM_DECLARE(Horizontal, Center);
		HL_ENUM_DECLARE(Horizontal, Right);
		HL_ENUM_DECLARE(Horizontal, LeftWrapped);
		HL_ENUM_DECLARE(Horizontal, RightWrapped);
		HL_ENUM_DECLARE(Horizontal, CenterWrapped);
		HL_ENUM_DECLARE(Horizontal, Justified);
		bool isLeft();
		bool isCenter();
		bool isRight();
		bool isWrapped();
	));

	HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, Vertical,
	(
		HL_ENUM_DECLARE(Vertical, Top); // top
		HL_ENUM_DECLARE(Vertical, Center); // mid
		HL_ENUM_DECLARE(Vertical, Bottom); // bot
	));

	HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, TextEffect,
	(
		HL_ENUM_DECLARE(TextEffect, None);
		HL_ENUM_DECLARE(TextEffect, Shadow);
		HL_ENUM_DECLARE(TextEffect, Border);
	));

	class atresExport RectDefinition
	{
	public:
		grect rect;

		RectDefinition();
		virtual ~RectDefinition();

	};

	class atresExport SymbolDefinition : public RectDefinition
	{
	public:
		float advance;

		SymbolDefinition();
		~SymbolDefinition();

	};

	class atresExport CharacterDefinition : public SymbolDefinition
	{
	public:
		gvec2 bearing;
		float offsetY;

		CharacterDefinition();
		~CharacterDefinition();

	};

	class atresExport BorderCharacterDefinition : public RectDefinition
	{
	public:
		float borderThickness;

		BorderCharacterDefinition(float borderThickness = 0.0f);
		~BorderCharacterDefinition();

	};

	typedef SymbolDefinition IconDefinition;
	typedef BorderCharacterDefinition BorderIconDefinition;

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
		bool multiplyAlpha;
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
		bool icon;
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
		HL_ENUM_CLASS_DECLARE(Type,
		(
			HL_ENUM_DECLARE(Type, Escape);
			HL_ENUM_DECLARE(Type, Font);
			HL_ENUM_DECLARE(Type, Icon);
			HL_ENUM_DECLARE(Type, Color);
			HL_ENUM_DECLARE(Type, Scale);
			HL_ENUM_DECLARE(Type, NoEffect);
			HL_ENUM_DECLARE(Type, Shadow);
			HL_ENUM_DECLARE(Type, Border);
			HL_ENUM_DECLARE(Type, IgnoreFormatting);
			HL_ENUM_DECLARE(Type, Close);
			HL_ENUM_DECLARE(Type, CloseConsume);
		));

		Type type;
		hstr data;
		hstr consumedData;
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
		int penX;
		int penY;
		int rowHeight;

		TextureContainer();
		virtual ~TextureContainer();

		virtual TextureContainer* createNew();

	};

	class atresExport BorderTextureContainer : public TextureContainer
	{
	public:
		float borderThickness;

		BorderTextureContainer(float borderThickness);
		~BorderTextureContainer();

		TextureContainer* createNew();

	};

	class CacheEntryBasicText
	{
	public:
		hstr text;
		hstr fontName;
		grect rect;
		Horizontal horizontal;
		Vertical vertical;
		april::Color color;
		gvec2 offset;

		CacheEntryBasicText();
		virtual ~CacheEntryBasicText();

		void set(hstr text, hstr fontName, grect rect, Horizontal horizontal, Vertical vertical, april::Color color, gvec2 offset);
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

