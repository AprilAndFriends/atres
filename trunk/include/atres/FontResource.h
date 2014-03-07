/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a font resource interface.

#ifndef ATRES_FONT_RESOURCE_H
#define ATRES_FONT_RESOURCE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "atresExport.h"
#include "Utility.h"

namespace atres
{
	class atresExport FontResource
	{
	public:
		FontResource(chstr name);
		virtual ~FontResource();

		HL_DEFINE_GET(hstr, name, Name);
		float getHeight();
		float getScale();
		HL_DEFINE_SET(float, scale, Scale);
		HL_DEFINE_GETSET(float, baseScale, BaseScale);
		float getLineHeight();
		float getDescender();
		HL_DEFINE_IS(loaded, Loaded);
		hmap<unsigned int, CharacterDefinition>& getCharacters() { return this->characters; }
		harray<april::Texture*> getTextures();
		
		virtual april::Texture* getTexture(unsigned int charCode);
		virtual bool hasChar(unsigned int charCode);
		
		/// @note Not thread-safe!
		float getTextWidth(chstr text);
		/// @note Not thread-safe!
		int getTextCount(chstr text, float maxWidth);
		
		/// @note Not thread-safe!
		RenderRectangle makeRenderRectangle(const grect& rect, grect area, unsigned int code);
		
	protected:
		hstr name;
		float height;
		float scale;
		float baseScale;
		float lineHeight;
		float descender;
		bool customDescender;
		bool loaded;
		hmap<unsigned int, CharacterDefinition> characters;
		harray<TextureContainer*> textureContainers;
		
		bool _readBasicParameter(chstr line);
		
	};

}

#endif

