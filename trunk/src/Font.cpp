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
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "Font.h"
#include "RenderingInterface.h"

#define CHR_BUFFER_MAX 2048

namespace Atres
{
	CharacterRenderOp rops[CHR_BUFFER_MAX];
	int nOps=0;
	
	void flushRenderOperations()
	{
		if (nOps)
		{
			getRenderInterface()->render(rops,nOps);
			nOps=0;
		}
	}
	
	Font::Font(std::string filename)
	{
		std::ifstream f(filename.c_str());
		
		if (!f.is_open())
			throw "Can't open Font file"+filename+"!";
		
		mScale=mDefaultScale=1;
		char line[512];
		mLineHeight=0;
		while (!f.eof())
		{
			f.getline(line,512);
#ifndef _WIN32
			int len=strlen(line);
			if (line[len-1] == '\r') line[len-1]=0;
#endif
			if      (strstr(line,"Name=")) mName=line+5;
			else if (strstr(line,"Resource="))
				    mResource=getRenderInterface()->loadResource(line+9);
			else if (strstr(line,"LineHeight=")) mLineHeight=atof(line+11);
			else if (strstr(line,"Height=")) mHeight=atof(line+7);
			else if (strstr(line,"Scale=")) mScale=mDefaultScale=atof(line+6);
			//if (strstr(line,"Scale="));
			if (line[0] == '#') continue;
			if (line[0] == '-') break;
		}
		if (mLineHeight == 0) mLineHeight=mHeight;
		
		FontCharDef c; unsigned int code;
		while (!f.eof())
		{
			f >> code;
			f >> c.x; f >> c.y; f >> c.w; f >> c.aw;
			if (c.aw == 0) c.aw=c.w;
			mChars[code]=c;
		}
		 
		f.close();
	}

	Font::Font(Font& f,float scale)
	{
		// todo: copy constructor
	}

	Font::~Font()
	{
		mChars.clear();
	}
	
	void Font::setScale(float scale)
	{
		if (scale == 0) mScale=mDefaultScale;
		else mScale=scale;
	}

	unsigned int utf8_getchar(const char* s,int& char_len_out)
	{
		if (*s < 0)
		{
			const unsigned char* u=(const unsigned char*) s;
			const unsigned char first=*u;

			if ((first & 0xE0) == 0xC0)
			{
				char_len_out=2;
				return ((first & 0x1F) << 6) | (u[1] & 0x3F);
			}
			else if ((first & 0xF0) == 0xE0)
			{
				char_len_out=3;
				return ((((first & 0xF) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F);
			}
			else
			{
				char_len_out=4;
				return ((((((first & 7) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F)) << 6) | (u[3] & 0x3F);
			}
		}
		else
		{
			char_len_out=1;
			return *s;
		}
	}

	void Font::render(float x,float y,float max_w,Alignment alignment,std::string text,bool draw,float r,float g,float b,float a,float* w_out,float* h_out)
	{
		// ascii only at the moment
		const char* s=text.c_str();
		int len=text.size(),i=0,j=0,last_j;
		unsigned int c=0,pc;
		
		unsigned char byte_r=r*255,byte_g=g*255,byte_b=b*255,byte_a=a*255;

		float offset=0,width,h=mHeight*mScale,text_w=0,starty=y;
		FontCharDef chr;
		
		nOps=0;
		CharacterRenderOp* op=rops;

		for (;s[i] != 0;)
		{
			int char_len;
			// first, get the last index of the last word we can fit into the current line
			for (width=offset=0,last_j=0;;j+=char_len)
			{
				pc=c;
				c=utf8_getchar(s+j,char_len);
				if (c == '\n') { j++; offset=width; break; }
				if (c == ' ' || c == 0)
				{
					offset=width; last_j=j+(c == ' ');
				}
				width+=mChars[c].aw*mScale;
				if (c == 0) break;
				if ((pc == ',' || pc == '.' || pc == '!' || pc == '?') && c != ' ') { offset=width; last_j=j; }
				
				if (width > max_w)
				{
					if (offset == 0) { offset=width-mChars[c].aw*mScale; last_j=j; }
					break;
				}
				
			}
			if (offset > text_w) text_w=offset;
			if (width > max_w && last_j > 0) j=last_j;
			if      (alignment == LEFT)
			{
				offset=x;
				if (i > 0 && s[i] == ' ') i++; // jump over spaces in the beginnings of lines after the first one
			}
			else if (alignment == RIGHT) offset=x-offset;
			else                         offset=x-offset/2;
			for (;i < j;i+=char_len)
			{
				c=utf8_getchar(s+i,char_len);
				chr=mChars[c];
				if (draw && c != ' ')
				{
					op->resource=mResource;
					op->r=byte_r; op->g=byte_g; op->b=byte_b; op->a=byte_a;
					op->italic=op->underline=op->strikethrough=0;
					op->sx=chr.x; op->sy=chr.y; op->sw=chr.w; op->sh=mHeight;
					op->dx=offset; op->dw=chr.w*mScale; op->dy=y; op->dh=h;
					op++;
					nOps++;
					if (nOps >= CHR_BUFFER_MAX) { flushRenderOperations(); op=rops; }
					
	 /*             v->x=offset;               v->y=y;   v->u=chr.x;       v->v=chr.y; v++;
					v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
					v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;

					v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
					v->x=offset+chr.w*mWScale; v->y=y+h; v->u=chr.x+chr.w; v->v=chr.y+mHeight; v++;
					v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;
	  */
				}
				offset+=chr.aw*mScale;
			}
			y+=mLineHeight*mScale;
		}
		
	/*    if (draw)
		{
			rendersys->setTexture(mTexture);
			if (r == 1 && g == 1 && b == 1 && a == 1) rendersys->render(TRIANGLE_LIST, vertices, len*6);
			else                                      rendersys->render(TRIANGLE_LIST, vertices, len*6,r,g,b,a);
			
			//delete v;
		}*/
		flushRenderOperations();
		if (w_out) *w_out=text_w;
		if (h_out) *h_out=y-starty;
	}
}
