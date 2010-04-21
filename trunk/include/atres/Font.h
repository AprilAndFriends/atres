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
#include <string>
#include "Atres.h"
#include "AtresExport.h"

namespace Atres
{
	struct AtresExport FontCharDef
	{
		float x,y,w,aw;
	};

	class AtresExport Font
	{
		std::map<unsigned int,FontCharDef> mChars;
		float mScale,mDefaultScale;
		std::string mName;
		unsigned int mResource;
		float mHeight,mLineHeight;

		
	public:
		Font(std::string filename);
		Font(Font& f,float scale=1);
		~Font();

		void render(float x,float y,float max_w,Alignment alignment,std::string text,bool draw,float r,float g,float b,float a,float* w_out,float* h_out);

		float getHeight() { return mHeight*mScale; }
		float getScale() { return mScale; }
		void setScale(float scale);
		std::string getName() { return mName; }
	};
}

#endif

