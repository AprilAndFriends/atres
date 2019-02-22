/// @file
/// @version 5.0
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
		HL_ENUM_DECLARE(Horizontal, LeftWrappedUntrimmed);
		HL_ENUM_DECLARE(Horizontal, RightWrapped);
		HL_ENUM_DECLARE(Horizontal, RightWrappedUntrimmed);
		HL_ENUM_DECLARE(Horizontal, CenterWrapped);
		HL_ENUM_DECLARE(Horizontal, CenterWrappedUntrimmed);
		HL_ENUM_DECLARE(Horizontal, Justified);
		bool isLeft() const;
		bool isCenter() const;
		bool isRight() const;
		bool isWrapped() const;
		bool isUntrimmed() const;
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
		grectf rect;

		RectDefinition();
		virtual ~RectDefinition();

	};

	class atresExport SymbolDefinition : public RectDefinition
	{
	public:
		float advance;

		SymbolDefinition();

	};

	class atresExport CharacterDefinition : public SymbolDefinition
	{
	public:
		gvec2f bearing;
		float offsetY;

		CharacterDefinition();

	};

	class atresExport BorderCharacterDefinition : public RectDefinition
	{
	public:
		float borderThickness;

		BorderCharacterDefinition(float borderThickness = 0.0f);

	};

	typedef SymbolDefinition IconDefinition;
	typedef BorderCharacterDefinition BorderIconDefinition;

	class atresExport RenderRectangle
	{
	public:
		grectf src;
		grectf dest;

		RenderRectangle();

	};
	
	class atresExport RenderSequence
	{
	public:
		april::Texture* texture;
		april::Color color;
		bool multiplyAlpha;
		harray<april::TexturedVertex> vertices;
		
		RenderSequence();

		/// @note Not thread-safe!
		void addRenderRectangle(const RenderRectangle& rect, float italicSkewOffset);

	};
	
	class atresExport RenderLiningSequence
	{
	public:
		april::Color color;
		harray<april::PlainVertex> vertices;

		RenderLiningSequence();

		/// @note Not thread-safe!
		void addRectangle(cgrectf rect);

	};

	class atresExport RenderWord
	{
	public:
		hstr text;
		grectf rect;
		int start;
		int count;
		int spaces;
		bool icon;
		float advanceX;
		float bearingX;
		harray<float> charXs;
		harray<float> charHeights;
		harray<float> charAdvanceXs;
		harray<float> segmentWidths;

		RenderWord();

	};
	
	class atresExport RenderLining
	{
	public:
		grectf rect;
		int start;
		int count;

		RenderLining();

	};

	class atresExport RenderLine
	{
	public:
		hstr text;
		grectf rect;
		int start;
		int count;
		int spaces;
		float advanceX;
		bool terminated;
		harray<RenderWord> words;
		
		RenderLine();

	};
	
	class atresExport RenderText
	{
	public:
		harray<RenderLine> lines;
		harray<RenderSequence> textSequences;
		harray<RenderSequence> shadowSequences;
		harray<RenderSequence> borderSequences;
		harray<RenderLiningSequence> textLiningSequences;
		harray<RenderLiningSequence> shadowLiningSequences;
		harray<RenderLiningSequence> borderLiningSequences;

		RenderText();

	};

	class atresExport FormatTag
	{
	public:
		HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, Type,
		(
			HL_ENUM_DECLARE(Type, Escape);
			HL_ENUM_DECLARE(Type, Font);
			HL_ENUM_DECLARE(Type, Icon);
			HL_ENUM_DECLARE(Type, Color);
			HL_ENUM_DECLARE(Type, Scale);
			HL_ENUM_DECLARE(Type, NoEffect);
			HL_ENUM_DECLARE(Type, Shadow);
			HL_ENUM_DECLARE(Type, Border);
			HL_ENUM_DECLARE(Type, StrikeThrough);
			HL_ENUM_DECLARE(Type, Underline);
			HL_ENUM_DECLARE(Type, Italic);
			HL_ENUM_DECLARE(Type, Hide);
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

		virtual TextureContainer* createNew() const;

	};

	class atresExport BorderTextureContainer : public TextureContainer
	{
	public:
		float borderThickness;

		BorderTextureContainer(float borderThickness);

		TextureContainer* createNew() const;

	};

	class CacheEntryBasicText
	{
	public:
		hstr text;
		hstr fontName;
		grectf rect;
		Horizontal horizontal;
		Vertical vertical;
		april::Color color;
		gvec2f offset;

		CacheEntryBasicText();
		virtual ~CacheEntryBasicText();

		void set(chstr text, chstr fontName, cgrectf rect, Horizontal horizontal, Vertical vertical, const april::Color& color, cgvec2f offset);
		bool operator==(const CacheEntryBasicText& other) const;
		bool operator!=(const CacheEntryBasicText& other) const;
		unsigned int hash() const;

	};

	class CacheEntryText : public CacheEntryBasicText
	{
	public:
		RenderText value;

		CacheEntryText();

		bool operator==(const CacheEntryText& other) const;
		bool operator!=(const CacheEntryText& other) const;

	};

	class CacheEntryLines : public CacheEntryBasicText
	{
	public:
		harray<RenderLine> value;

		CacheEntryLines();

		bool operator==(const CacheEntryLines& other) const;
		bool operator!=(const CacheEntryLines& other) const;

	};

	class CacheEntryLine
	{
	public:
		hstr text;
		hstr fontName;
		gvec2f size;
		RenderLine value;

		CacheEntryLine();

		void set(chstr text, chstr fontName, cgvec2f size);
		bool operator==(const CacheEntryLine& other) const;
		bool operator!=(const CacheEntryLine& other) const;
		unsigned int hash() const;

	};

}
#endif
