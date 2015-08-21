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
/// Defines a font object.

#ifndef ATRES_FONT_H
#define ATRES_FONT_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "atresExport.h"
#include "Utility.h"

namespace atres
{
	class atresExport Font
	{
	public:
		Font(chstr name);
		virtual ~Font();

		bool load();

		HL_DEFINE_GET(hstr, name, Name);
		float getHeight();
		float getScale();
		HL_DEFINE_SET(float, scale, Scale);
		HL_DEFINE_GETSET(float, baseScale, BaseScale);
		float getLineHeight();
		float getDescender();
		float getInternalDescender();
		HL_DEFINE_IS(loaded, Loaded);
		inline hmap<unsigned int, CharacterDefinition>& getCharacters() { return this->characters; }
		inline hmap<hstr, IconDefinition>& getIcons() { return this->icons; }
		harray<april::Texture*> getTextures();
		
		virtual april::Texture* getTexture(unsigned int charCode);
		virtual april::Texture* getTexture(chstr iconName);
		virtual bool hasCharacter(unsigned int charCode);
		virtual bool hasIcon(chstr iconName);

		/// @note Not thread-safe!
		float getTextWidth(chstr text);
		/// @note Not thread-safe!
		int getTextCount(chstr text, float maxWidth);
		
		/// @note Not thread-safe!
		RenderRectangle makeRenderRectangle(const grect& rect, grect area, unsigned int charCode);
		RenderRectangle makeRenderRectangle(const grect& rect, grect area, chstr iconName);

		DEPRECATED_ATTRIBUTE bool hasChar(unsigned int charCode) { return this->hasCharacter(charCode); }
		
	protected:
		hstr name;
		float height;
		float scale;
		float baseScale;
		float lineHeight;
		float descender;
		float internalDescender;
		bool loaded;
		hmap<unsigned int, CharacterDefinition> characters;
		hmap<hstr, IconDefinition> icons;
		harray<TextureContainer*> textureContainers;

		virtual bool _load();
		
		bool _readBasicParameter(chstr line);

		void _applyCutoff(const grect& rect, const grect& area, const grect& symbolRect, float offsetY = 0.0f);
		
	};

}

#endif

