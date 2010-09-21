/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <hltypes/exception.h>

#include "Atres.h"
#include "Font.h"

namespace Atres
{
    std::map<hstr,Font*> fonts;
	Font* default_font=0;
	void (*g_logFunction)(chstr)=atres_writelog;

    void init()
    {
    }
    
    void destroy()
    {
        for (std::map<hstr,Font*>::iterator it=fonts.begin();it!=fonts.end();it++)
		{
			delete it->second;
		}
    }

	void logMessage(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
	void atres_writelog(chstr message)
	{
		printf("%s\n", message.c_str());		
	}
	
	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction=fnptr;
	}
	
	void drawText(chstr fontName, grect rect, chstr text, Alignment alignment, April::Color color, Effect effect)
	{
		Font* f = getFont(fontName);
		if (effect == SHADOW || effect == BORDER || effect == BORDER_4)
		{
			f->render(grect(rect.x + 1, rect.y + 1, rect.w, rect.h), text, alignment, color);
			if (effect == BORDER || effect == BORDER_4)
			{
				f->render(grect(rect.x - 1, rect.y - 1, rect.w, rect.h), text, alignment, color);
				f->render(grect(rect.x + 1, rect.y - 1, rect.w, rect.h), text, alignment, color);
				f->render(grect(rect.x - 1, rect.y + 1, rect.w, rect.h), text, alignment, color);
				if (effect == BORDER)
				{
					f->render(grect(rect.x, rect.y - 1, rect.w, rect.h), text, alignment, color);
					f->render(grect(rect.x, rect.y + 1, rect.w, rect.h), text, alignment, color);
					f->render(grect(rect.x - 1, rect.y, rect.w, rect.h), text, alignment, color);
					f->render(grect(rect.x + 1, rect.y, rect.w, rect.h), text, alignment, color);
				}
			}
			flushRenderOperations();
		}
		f->render(rect, text, alignment, color);
		flushRenderOperations();
	}

	void drawText(grect rect, chstr text, Alignment alignment, April::Color color, Effect effect)
	{
		drawText("", rect, text, alignment, color, effect);
	}

	void drawText(chstr fontName, grect rect, chstr text, Alignment alignment,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText(fontName, rect, text, alignment, April::Color(a, r, g, b), effect);
	}
	
	void drawText(grect rect, chstr text, Alignment alignment,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText("", rect, text, alignment, April::Color(a, r, g, b), effect);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text,
		Alignment alignment, April::Color color, Effect effect)
	{
		drawText(fontName, grect(x, y, w, h), text, alignment, color, effect);
	}
	
	void drawText(float x, float y, float w, float h, chstr text,
		Alignment alignment, April::Color color, Effect effect)
	{
		drawText("", grect(x, y, w, h), text, alignment, color, effect);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment alignment,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText(fontName, grect(x, y, w, h), text, alignment, April::Color(a, r, g, b), effect);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment alignment,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText("", x, y, w, h, text, alignment, r, g, b, a, effect);
	}
	
	//2DO - delete from here
	/*
	float drawText(chstr font_name,float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		drawText(font_name, grect(x, y, w_max, h_max), text, alignment, April::Color(a, r, g, b), effect);
		return 0.0f;
	}

	float drawWrappedText(chstr font_name,float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		drawText(font_name, grect(x, y, w_max, h_max), text, alignment, April::Color(a, r, g, b), effect);
		return 0.0f;
	}

	float drawText(float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawText("",x,y,w_max,h_max,text,r,g,b,a,alignment,effect);
	}

	float drawWrappedText(float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawWrappedText("",x,y,w_max,h_max,text,r,g,b,a,alignment,effect);
	}
	*/
	//2DO - to here

	float getFontHeight(chstr fontName)
	{
		return getFont(fontName)->getHeight();
	}
	
	float getTextWidth(chstr fontName, chstr text)
	{
		harray<grect> areas;
		getFont(fontName)->testRender(grect(0,0,100000,100000), text, LEFT, &areas);
		float maxWidth = 0.0f;
		foreach (grect, it, areas)
		{
			if (maxWidth < (*it).w)
			{
				maxWidth = (*it).w;
			}
		}
		return maxWidth;
	}

	float getTextHeight(chstr fontName, chstr text, float maxWidth)
	{
		harray<grect> areas;
		//2DO - when not wrapped, no unlimited rect is needed
		Font* f = getFont(fontName);
		harray<hstr> lines = f->testRender(grect(0, 0, maxWidth, 100000), text, LEFT, &areas);
		return (lines.size() * f->getLineHeight());
	}
	
	int getTextCount(chstr fontName, chstr text, float maxWidth, float maxHeight)
	{
		int count;
		getFont(fontName)->testRender(grect(0, 0, maxWidth, maxHeight), text, LEFT, NULL, &count);
		return count;
	}
	
	void setDefaultFont(chstr name)
	{
		default_font=fonts[name];
	}
    
    void loadFont(chstr filename)
    {
        Font* f=new Font(filename);
        fonts[f->getName()]=f;
		if (default_font == 0) default_font=f;
    }
    
    Font* getFont(chstr name)
    {
		if (name == "" && default_font != 0)
		{
			default_font->setScale(0);
			return default_font;
		}
		
        Font* f;
        if (fonts.find(name) == fonts.end())
		{
			int pos=name.find(":");
			if (pos != -1)
			{
				f=getFont(name(0,pos));
				f->setScale((float)atof(name(pos+1,10).c_str()));
			}
			else throw resource_error("Font",name,"Atres");
		}
		else
		{
			f=fonts[name];
			f->setScale(0);
		}
        return f;
    }
	
}
