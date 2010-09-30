/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_FONT_H
#define ATRES_FONT_H

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Atres.h"
#include "AtresExport.h"

namespace Atres
{
	struct AtresExport CharacterDefinition
	{
		float x, y, w, aw;
	};

	class AtresExport Font
	{
	public:
		Font(chstr filename);
		Font(Font& f, float scale = 1.0f);
		~Font();

		hstr getName() { return this->name; }
		float getHeight() { return this->height * this->scale; }
		float getLineHeight() { return this->lineHeight * this->scale; }
		float getScale() { return this->scale; }
		void setScale(float value);
		hmap<unsigned int, CharacterDefinition>& getCharacters() { return this->characters; }
		April::Texture* getTexture() { return this->texture; }
		
		bool hasChar(unsigned int charcode);
		
		float getTextWidth(chstr text);
		int getTextCount(chstr text, float maxWidth);
		
		RenderRectangle makeRenderRectangle(grect rect, grect area, unsigned int code);
		
	protected:
		hstr name;
		float scale;
		float defaultScale;
		float height;
		float lineHeight;
		hmap<unsigned int, CharacterDefinition> characters;
		April::Texture* texture;
		
	};
}

#endif

