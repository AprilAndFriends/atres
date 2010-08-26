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

#include <map>
#include <hltypes/hstring.h>
#include "Atres.h"
#include "AtresExport.h"

namespace Atres
{
	void flushRenderOperations();
	
	struct AtresExport FontCharDef
	{
		float x,y,w,aw;
	};

	class AtresExport Font
	{
		std::map<unsigned int,FontCharDef> mChars;
		float mScale,mDefaultScale;
		hstr mName;
		unsigned int mResource;
		float mHeight,mLineHeight;

		
	public:
		Font(chstr filename);
		Font(Font& f,float scale=1);
		~Font();

		void render(float x,float y,float max_w,Alignment alignment,chstr text,bool draw,
			float r,float g,float b,float a,float* w_out,float* h_out);
		
		bool hasChar(unsigned int charcode);
		float getHeight() { return mHeight*mScale; }
		float getScale() { return mScale; }
		void setScale(float scale);
		hstr getName() { return mName; }
	};
}

#endif

