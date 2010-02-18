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
#include "Font.h"
#include "RenderingInterface.h"

namespace Atres
{
Font::Font(std::string filename)
{
	std::ifstream f(filename.c_str());
	
	if (!f.is_open())
		throw "Can't open Font file"+filename+"!";
    
	char line[512];
	while (!f.eof())
	{
		f.getline(line,512);
		if (line[0] == '#') continue;
	}
	
	
 //   mName=xmlGetPropString(cur, "name");
 //   std::string texture_name=xmlGetPropString(cur, "texture");
//    int slash=filename.rfind('/');
//    if (slash == -1) slash=filename.rfind('\\');
 //   std::string base=(slash >= 0) ? filename.substr(0,slash+1) : "";
  //  mTexture=rendersys->loadTexture(base+texture_name);
   // float w=mTexture->getWidth(),h=mTexture->getHeight();
    /*
    
    mHScale=h*0.75f;
    mHeight=xmlGetPropFloat(cur, "h")/h;
    mWScale=mHScale*w/h;
    
    FontCharDef c; unsigned int code;
    for (cur = cur->xmlChildrenNode; cur != 0; cur=cur->next)
	{
        if (XML_EQ(cur,"chr"))
        {
            code=xmlGetPropInt(cur,"code");
            c.x=xmlGetPropFloat(cur,"x")/w;//-0.5f/w;
            c.y=xmlGetPropFloat(cur,"y")/h;//-0.5f/h;
            c.w=xmlGetPropFloat(cur,"w")/w;
			try { c.aw=xmlGetPropFloat(cur,"aw")/w; }
			catch (_XMLException) { c.aw=c.w; }
            mChars[code]=c;
        }
    }
    
    xmlFreeDoc(doc);
	 * 
	 */
	 
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

void Font::textfunc(float x,float y,float max_w,FontAlignment alignment,std::string text,bool draw,float r,float g,float b,float a,float* w_out,float* h_out)
{
    // ascii only at the moment
    const char* s=text.c_str();
    int len=text.size(),i=0,j=0,last_j;
    unsigned int c=0,pc;
    
    /*static TexturedVertex *vertices=0;*/
    //TexturedVertex *v;
    float offset=0,width,h=mHeight*mHScale,text_w=0,text_h=0;
    FontCharDef chr;
    
    /*if (draw)
    {
        if (vertices) delete vertices;
        vertices=new TexturedVertex[len*6];
        v=vertices;
    }
    */
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
				offset=width; last_j=j;
			}
			width+=mChars[c].aw*mWScale;
            if (c == 0) break;
			if ((pc == ',' || pc == '.' || pc == '!' || pc == '?') && c != ' ') { offset=width; last_j=j; }
            
            if (width > max_w) break;
            
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
 /*               v->x=offset;               v->y=y;   v->u=chr.x;       v->v=chr.y; v++;
                v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
                v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;

                v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
                v->x=offset+chr.w*mWScale; v->y=y+h; v->u=chr.x+chr.w; v->v=chr.y+mHeight; v++;
                v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;*/
            }
            offset+=chr.aw*mWScale;
        }
        y+=h;
        text_h+=h;
    }
    
/*    if (draw)
    {
        rendersys->setTexture(mTexture);
        if (r == 1 && g == 1 && b == 1 && a == 1) rendersys->render(TRIANGLE_LIST, vertices, len*6);
        else                                      rendersys->render(TRIANGLE_LIST, vertices, len*6,r,g,b,a);
        
        //delete v;
    }*/
    if (w_out) *w_out=text_w;
    if (h_out) *h_out=text_h;
}

float Font::drawText(float x,float y,std::string text,float r,float g,float b,float a,FontAlignment alignment,FontEffect effect)
{
    float w;
    if (effect == SHADOW || effect == BORDER)
        textfunc(x+1,y+1,100000,alignment,text,1,0,0,0,a,0,0);
    if (effect == BORDER)
    {
        textfunc(x-1,y-1,100000,alignment,text,1,0,0,0,a,0,0);
        textfunc(x+1,y-1,100000,alignment,text,1,0,0,0,a,0,0);
        textfunc(x-1,y+1,100000,alignment,text,1,0,0,0,a,0,0);
    }
    textfunc(x, y, 100000, alignment,text,1,r,g,b,a,&w,0);
    return w;
}

float Font::drawWrappedText(float x,float y,float w_max,std::string text,float r,float g,float b,float a,FontAlignment alignment,FontEffect effect)
{
    float h;
    if (effect == SHADOW || effect == BORDER)
        textfunc(x+1,y+1,w_max,alignment,text,1,0,0,0,a,0,0);
    if (effect == BORDER)
    {
        textfunc(x-1,y-1,w_max,alignment,text,1,0,0,0,a,0,0);
        textfunc(x+1,y-1,w_max,alignment,text,1,0,0,0,a,0,0);
        textfunc(x-1,y+1,w_max,alignment,text,1,0,0,0,a,0,0);
    }
    textfunc(x,y,w_max,alignment,text,1,r,g,b,a,0,&h);
    return h;
}

float Font::getTextWidth(std::string text,FontAlignment alignment)
{
    float w;
    textfunc(0,0, 100000, alignment,text,0,1,1,1,1,&w,0);
    return w;
}

float Font::getWrappedTextHeight(float w_max,std::string text,FontAlignment alignment)
{
    float h;
    textfunc(0,0,w_max,alignment,text,0,1,1,1,1,0,&h);
    return h;
}
}
