/// @file
/// @author  Boris Mikic
/// @version 2.0
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

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "atresExport.h"
#include "Renderer.h"

namespace atres
{
	struct atresExport CharacterDefinition
	{
		float x;
		float y;
		float w;
		float h;
		float aw;
	};

	class atresExport FontResource
	{
	public:
		FontResource(chstr name);
		//FontResource(FontResource& f, float scale = 1.0f);
		virtual ~FontResource();

		hstr getName() { return this->name; }
		float getHeight();
		float getLineHeight();
		float getScale();
		void setScale(float value) { this->scale = value; }
		float getBaseScale() { return this->baseScale; }
		void setBaseScale(float value) { this->baseScale = value; }
		hmap<unsigned int, CharacterDefinition>& getCharacters() { return this->characters; }
		
		virtual april::Texture* getTexture(unsigned int charcode) = 0;
		virtual bool hasChar(unsigned int charcode);
		
		float getTextWidth(chstr text);
		int getTextCount(chstr text, float maxWidth);
		
		RenderRectangle makeRenderRectangle(grect rect, grect area, unsigned int code);
		
	protected:
		hstr name;
		float height;
		float scale;
		float baseScale;
		float lineHeight;
		hmap<unsigned int, CharacterDefinition> characters;
		
	};
}

#endif

