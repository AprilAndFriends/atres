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
	
	void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, Effect effect)
	{
		Font* f = getFont(fontName);
		if (effect == SHADOW || effect == BORDER || effect == BORDER_4)
		{
			f->render(grect(rect.x + 1, rect.y + 1, rect.w, rect.h), text, horizontal, vertical, color);
			if (effect == BORDER || effect == BORDER_4)
			{
				f->render(grect(rect.x - 1, rect.y - 1, rect.w, rect.h), text, horizontal, vertical, color);
				f->render(grect(rect.x + 1, rect.y - 1, rect.w, rect.h), text, horizontal, vertical, color);
				f->render(grect(rect.x - 1, rect.y + 1, rect.w, rect.h), text, horizontal, vertical, color);
				if (effect == BORDER)
				{
					f->render(grect(rect.x, rect.y - 1, rect.w, rect.h), text, horizontal, vertical, color);
					f->render(grect(rect.x, rect.y + 1, rect.w, rect.h), text, horizontal, vertical, color);
					f->render(grect(rect.x - 1, rect.y, rect.w, rect.h), text, horizontal, vertical, color);
					f->render(grect(rect.x + 1, rect.y, rect.w, rect.h), text, horizontal, vertical, color);
				}
			}
			flushRenderOperations();
		}
		f->render(rect, text, horizontal, vertical, color);
		flushRenderOperations();
	}

	void drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, Effect effect)
	{
		drawText("", rect, text, horizontal, vertical, color, effect);
	}

	void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText(fontName, rect, text, horizontal, vertical, April::Color(a, r, g, b), effect);
	}
	
	void drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText("", rect, text, horizontal, vertical, April::Color(a, r, g, b), effect);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, Effect effect)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, color, effect);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, Effect effect)
	{
		drawText("", grect(x, y, w, h), text, horizontal, vertical, color, effect);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), effect);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, Effect effect)
	{
		drawText("", x, y, w, h, text, horizontal, vertical, r, g, b, a, effect);
	}
	
	float getFontHeight(chstr fontName)
	{
		return getFont(fontName)->getHeight();
	}
	
	float getTextWidth(chstr fontName, chstr text)
	{
		harray<grect> areas;
		getFont(fontName)->testRender(grect(0, 0, 1, 1), text, LEFT, TOP, &areas);
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
		harray<hstr> lines = f->testRender(grect(0, 0, maxWidth, 100000), text, LEFT_WRAPPED, TOP, &areas);
		return (lines.size() * f->getLineHeight());
	}
	
	int getTextCount(chstr fontName, chstr text, float maxWidth, float maxHeight)
	{
		harray<hstr> lines = getFont(fontName)->testRender(grect(0, 0, maxWidth, maxHeight), text, LEFT_WRAPPED, TOP);
		if (lines.size() == 0)
		{
			return 0;
		}
		hstr str = text;
		foreach (hstr, it, lines)
		{
			str = str.replace((*it), "").ltrim().ltrim('\n');
		}
		return (text.size() - str.size());
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
