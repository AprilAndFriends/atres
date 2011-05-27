/// @file
/// @author  Kresimir Spes
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
/// Defines a font.

#ifndef ATRES_FONT_H
#define ATRES_FONT_H

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "atresExport.h"

namespace atres
{
	struct atresExport CharacterDefinition
	{
		float x;
		float y;
		float w;
		float aw;
	};

	class atresExport Font
	{
	public:
		Font(chstr filename);
		Font(Font& f, float scale = 1.0f);
		~Font();

		hstr getName() { return this->name; }
		float getHeight();
		float getLineHeight();
		float getScale();
		void setScale(float value) { this->scale = value; }
		float getBaseScale() { return this->baseScale; }
		void setBaseScale(float value) { this->baseScale = value; }
		hmap<unsigned int, CharacterDefinition>& getCharacters() { return this->characters; }
		april::Texture* getTexture() { return this->texture; }
		
		bool hasChar(unsigned int charcode);
		
		float getTextWidth(chstr text);
		int getTextCount(chstr text, float maxWidth);
		
		RenderRectangle makeRenderRectangle(grect rect, grect area, unsigned int code);
		
	protected:
		hstr name;
		float scale;
		float baseScale;
		float height;
		float lineHeight;
		hmap<unsigned int, CharacterDefinition> characters;
		april::Texture* texture;
		
	};
}

#endif

