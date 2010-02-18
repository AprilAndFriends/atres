/************************************************************************************
This source file is part of the Advanced Text Rendering System
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#ifndef ATRES_FONT_H
#define ATRES_FONT_H

#include <map>
#include <string>

namespace Atres
{
	struct FontCharDef
	{
		float x,y,w,aw;
	};

	enum FontAlignment
	{
		LEFT,RIGHT,CENTER
	};

	enum FontEffect
	{
		NONE,SHADOW,BORDER
	};

	class Font
	{
		std::map<unsigned int,FontCharDef> mChars;
		float mWScale,mHScale;
		std::string mName;
		float mHeight;

		// todo: make sure to include reference counting if you clone a font!
		void textfunc(float x,float y,float max_w,FontAlignment alignment,std::string text,bool draw,float r,float g,float b,float a,float* w_out,float* h_out);
		
	public:
		Font(std::string filename);
		Font(Font& f,float scale=1);
		~Font();
		
		float drawText(float x,float y,std::string text,float r=1,float g=1,float b=1,float a=1,FontAlignment alignment=LEFT,FontEffect effect=NONE);
		float drawWrappedText(float x,float y,float w_max,std::string text,float r=1,float g=1,float b=1,float a=1,FontAlignment alignment=LEFT,FontEffect effect=NONE);
		
		float getTextWidth(std::string text,FontAlignment alignment=LEFT);
		float getWrappedTextHeight(float w_max,std::string text,FontAlignment alignment=LEFT);
		
		float getHeight() { return mHeight*mHScale; }
		float getWidthScale() { return mWScale; }
		float getHeightScale() { return mHScale; }
		std::string getName() { return mName; }
	};
}

#endif

