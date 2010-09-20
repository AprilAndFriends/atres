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
		
		void render(float x, float y, float w, float h, Alignment alignment, bool wrap, chstr text, bool draw, float r, float g, float b, float a, float* w_out, float* h_out, int *c_out) __attribute__((deprecated));
		void render(grect rect, Alignment alignment, bool wrap, chstr text, bool draw, float r, float g, float b, float a, float* w_out, float* h_out, int *c_out) __attribute__((deprecated));
		void render(float x, float y, float w, float h, Alignment alignment, bool wrap, chstr text, bool draw, April::Color color, float* w_out, float* h_out, int *c_out) __attribute__((deprecated));
		
		harray<hstr> testRender(grect rect, chstr text, Alignment alignment, harray<grect>* sizes = NULL, int* count = NULL);
		void render(grect rect, chstr text, Alignment alignment, April::Color color, float* w_out, float* h_out, int *c_out);
		
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

