/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "Font.h"

#define CHR_BUFFER_MAX 32768

namespace Atres
{
	CharacterRenderOp rops[CHR_BUFFER_MAX];
	int nOps=0;
	
	//2DO
	void flushRenderOperations()
	{
		if (nOps)
		{
			April::TexturedVertex v[CHR_BUFFER_MAX];
			//April::Texture* t = rendersys->getTe g_font_textures[rops[0].resource];
			April::Texture* t = rops[0].texture;
			int i=0;
			float w=(float)t->getWidth(),h=(float)t->getHeight();
			for (Atres::CharacterRenderOp* op=rops;op < rops+nOps;op++)
			{
				v[i].x=op->dest.x;              v[i].y = op->dest.y;              v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y;              v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = op->src.y / h;               i++;
				v[i].x=op->dest.x + op->dest.w; v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = (op->src.x + op->src.w) / w; v[i].v = (op->src.y + op->src.h) / h; i++;
				v[i].x=op->dest.x;              v[i].y = op->dest.y + op->dest.h; v[i].z = 0; v[i].u = op->src.x / w;               v[i].v = (op->src.y + op->src.h) / h; i++;
			}
			rendersys->setTexture(t);
			rendersys->render(April::TriangleList,v,i,rops[0].color.r_float(),rops[0].color.g_float(),rops[0].color.b_float(),rops[0].color.a_float());
			/*
			rendersys->render()
			getRenderInterface()->render(rops,nOps);
			*/
			nOps=0;
		}
	}
	
	Font::Font(chstr filename)
	{
		this->scale = 1.0f;
		this->defaultScale = 1.0f;
		this->lineHeight = 0.0f;
		harray<hstr> lines = hfile::hread(filename).split("\n");
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			if (line.starts_with("Name="))
			{
				this->name = line.replace("Name=", "");
			}
			else if (line.starts_with("Resource="))
			{
				this->texture = rendersys->loadTexture(line.replace("Resource=", ""));
				Atres::logMessage(hsprintf("\"Resource=\" is deprecated. Use \"Texture=\" instead. (File: %s)", filename.c_str()));
			}
			else if (line.starts_with("Texture="))
			{
				this->texture = rendersys->loadTexture(line.replace("Texture=", ""));
			}
			else if (line.starts_with("LineHeight="))
			{
				this->lineHeight = (float)line.replace("LineHeight=", "");
			}
			else if (line.starts_with("Height="))
			{
				this->height = (float)line.replace("Height=", "");
			}
			else if (line.starts_with("Scale="))
			{
				this->scale = (float)line.replace("Scale=", "");
				this->defaultScale = this->scale;
			}
			else if (line.starts_with("#"))
			{
				continue;
			}
			else if (line.starts_with("-"))
			{
				break;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		FontCharDef c;
		unsigned int code;
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			data = (*it).split(" ");
			code = (unsigned int)data.pop_front();
			c.x = (float)data.pop_front();
			c.y = (float)data.pop_front();
			c.w = (float)data.pop_front();
			c.aw = (float)data.pop_front();
			if (c.aw == 0.0f)
			{
				c.aw = c.w;
			}
			this->characters[code] = c;
		}
	}

	Font::Font(Font& f,float scale)
	{
		// todo: copy constructor
	}

	Font::~Font()
	{
		this->characters.clear();
	}
	
	bool Font::hasChar(unsigned int charcode)
	{
		return this->characters.has_key(charcode);
	}
	
	void Font::setScale(float value)
	{
		this->scale = (value == 0.0f ? this->defaultScale : value);
	}

	unsigned int utf8_getchar(const char* s, int& char_len_out)
	{
		if (*s < 0)
		{
			const unsigned char* u = (const unsigned char*)s;
			const unsigned char first = *u;
			if ((first & 0xE0) == 0xC0)
			{
				char_len_out = 2;
				return ((first & 0x1F) << 6) | (u[1] & 0x3F);
			}
			if ((first & 0xF0) == 0xE0)
			{
				char_len_out = 3;
				return ((((first & 0xF) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F);
			}
			char_len_out = 4;
			return ((((((first & 7) << 6) | (u[1] & 0x3F) ) << 6) | (u[2] & 0x3F)) << 6) | (u[3] & 0x3F);
		}
		char_len_out = 1;
		return *s;
	}

	void Font::render(float x, float y, float w, float h, Alignment alignment, bool wrap, chstr text, bool draw, float r, float g, float b, float a, float* w_out, float* h_out, int *c_out)
	{
		this->render(grect(x, y, w, h), alignment, wrap, text, draw, April::Color(a, r, g, b), w_out, h_out, c_out);
	}
	
	void Font::render(grect rect, Alignment alignment, bool wrap, chstr text, bool draw, float r, float g, float b, float a, float* w_out, float* h_out, int *c_out)
	{
		this->render(rect, alignment, wrap, text, draw, April::Color(a, r, g, b), w_out, h_out, c_out);
	}
	
	void Font::render(float x, float y, float w, float h, Alignment alignment, bool wrap, chstr text, bool draw, April::Color color, float* w_out, float* h_out, int *c_out)
	{
		this->render(grect(x, y, w, h), alignment, wrap, text, draw, color, w_out, h_out, c_out);
	}
	
	//2DO
	void Font::render(grect rect, Alignment alignment, bool wrap, chstr text, bool draw, April::Color color, float* w_out, float* h_out, int *c_out)
	{
		// ascii only at the moment
		const char* s=text.c_str();
		int len=text.size(),i=0,j=0,last_j;
		unsigned int c=0,pc;
		
		float offset=0,width,h=this->height*this->scale,text_w=0,starty=rect.y;
		float y = starty;
		FontCharDef chr;
		
		CharacterRenderOp* op=rops+nOps;
		
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
				width+=this->characters[c].aw*this->scale;
				if (c == 0) break;
				if ((pc == ',' || pc == '.' || pc == '!' || pc == '?') &&
					c != ' ' && c != ',' && c != '.' && c != '!' & c != '?')
				{
					offset=width; last_j=j;
				}
				
				if (width > rect.w) // line must have at least one character
				{
					if (offset == 0)
					{
						offset=width-this->characters[c].aw*this->scale;
						if (offset == 0) // this happens when max_w is smaller then the character width
						{
							// in that case, allow one character through, even though
							// it will break the max_w limit
							offset=width;
							j+=char_len;
						}
						last_j=j;
					}
					break;
				}
				
			}
			if (offset > text_w) text_w=offset;
			if (width > rect.w && last_j > 0) j=last_j;
			if      (alignment == LEFT)
			{
				offset=rect.x;
				if (i > 0 && s[i] == ' ') i++; // jump over spaces in the beginnings of lines after the first one
			}
			else if (alignment == RIGHT) offset=rect.x-offset;
			else                         offset=rect.x-offset/2;
			for (;i < j;i+=char_len)
			{
				c=utf8_getchar(s+i,char_len);
				chr=this->characters[c];
				if (draw && c != ' ')
				{
					op->texture = this->texture;
					op->color = color;
					op->italic = op->underline = op->strikethrough = false;
					op->src.x = (unsigned short)chr.x; op->src.y = (unsigned short)chr.y;
					op->src.w = (unsigned short)chr.w; op->src.h = (unsigned short)this->height;
					op->dest.x = offset; op->dest.w = chr.w * this->scale; op->dest.y = y; op->dest.h = h;
					op++;
					nOps++;
					
					//if (nOps >= CHR_BUFFER_MAX) { flushRenderOperations(); op=rops; }
					
	 /*             v->x=offset;               v->y=y;   v->u=chr.x;       v->v=chr.y; v++;
					v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
					v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;

					v->x=offset+chr.w*mWScale; v->y=y;   v->u=chr.x+chr.w; v->v=chr.y; v++;
					v->x=offset+chr.w*mWScale; v->y=y+h; v->u=chr.x+chr.w; v->v=chr.y+mHeight; v++;
					v->x=offset;               v->y=y+h; v->u=chr.x;       v->v=chr.y+mHeight; v++;
	  */
				}
				offset += chr.aw*this->scale;
			}
			y+=this->lineHeight*this->scale;
			if (!wrap) break;
			if (rect.y - starty >= rect.h) break;
			
		}
		
	/*    if (draw)
		{
			rendersys->setTexture(mTexture);
			if (r == 1 && g == 1 && b == 1 && a == 1) rendersys->render(TRIANGLE_LIST, vertices, len*6);
			else                                      rendersys->render(TRIANGLE_LIST, vertices, len*6,r,g,b,a);
			
			//delete v;
		}*/
		
		if (w_out) *w_out=text_w;
		if (h_out) *h_out=y-starty;
		if (c_out) *c_out=j;
	}
}
