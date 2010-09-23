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
	void flushRenderOperations();
	
	struct AtresExport FontCharDef
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
		April::Texture* getTexture() { return this->texture; }
		
		bool hasChar(unsigned int charcode);
		
		float getTextWidth(chstr text);
		int getTextCount(chstr text, float maxWidth);
		
		int verticalCorrection(grect rect, Alignment vertical, harray<hstr>& lines, harray<grect>& areas, gvec2 offset);
		void horizontalCorrection(grect rect, Alignment horizontal, harray<hstr>& lines, harray<grect>& areas, gvec2 offset);
		int testRender(grect rect, chstr text, Alignment horizontal, Alignment vertical, harray<hstr>& lines, harray<grect>& areas, gvec2 offset = gvec2());
		void render(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset = gvec2());
		void renderRaw(grect rect, harray<hstr> lines, harray<grect> areas, April::Color color, gvec2 offset = gvec2());
		
	protected:
		hstr name;
		float scale;
		float defaultScale;
		float height;
		float lineHeight;
		hmap<unsigned int, FontCharDef> characters;
		April::Texture* texture;
		
	};
}

#endif

