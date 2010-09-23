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
#include <hltypes/exception.h>

#include "Atres.h"
#include "Font.h"
#include <stdio.h>

namespace Atres
{
    hmap<hstr,Font*> fonts;
	Font* default_font=0;
	void (*g_logFunction)(chstr)=atres_writelog;
	gvec2 shadowOffset(1.0f, 1.0f);
	April::Color shadowColor(255, 0, 0, 0);
	float borderOffset = 1.0f;
	April::Color borderColor(255, 0, 0, 0);

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

	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction=fnptr;
	}
	
	void logMessage(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
	void atres_writelog(chstr message)
	{
		printf("%s\n", message.c_str());		
	}
	
/******* DRAW TEXT *****************************************************/

	void drawText(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		getFont(fontName)->render(rect, text, horizontal, vertical, color, offset);
		flushRenderOperations();
	}

	void drawTextShadowed(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		Font* f = getFont(fontName);
		harray<grect> areas;
		harray<hstr> lines = f->testRender(rect, text, horizontal, vertical, areas, offset);
		f->renderRaw(rect + shadowOffset, lines, areas, shadowColor);
		flushRenderOperations();
		f->renderRaw(rect, lines, areas, color);
		flushRenderOperations();
	}

	void drawTextBordered(chstr fontName, grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		Font* f = getFont(fontName);
		harray<grect> areas;
		harray<hstr> lines = f->testRender(rect, text, horizontal, vertical, areas, offset);
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(0.0f, -borderOffset) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(-borderOffset, 0.0f) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(borderOffset, 0.0f) * f->getScale());
		f->renderRaw(rect, lines, areas, borderColor, gvec2(0.0f, borderOffset) * f->getScale());
		flushRenderOperations();
		f->renderRaw(rect, lines, areas, color);
		flushRenderOperations();
	}

/******* DRAW TEXT OVERLOADS *******************************************/

	void drawText(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawText("", rect, text, horizontal, vertical, color, offset);
	}

	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawText("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawText(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawText(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawText("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
	void drawTextShadowed(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawTextShadowed("", rect, text, horizontal, vertical, color, offset);
	}

	void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextShadowed(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextShadowed("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextShadowed(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextShadowed(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawTextShadowed(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextShadowed("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
	void drawTextBordered(grect rect, chstr text, Alignment horizontal, Alignment vertical, April::Color color, gvec2 offset)
	{
		drawTextBordered("", rect, text, horizontal, vertical, color, offset);
	}

	void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextBordered(fontName, grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		April::Color color, gvec2 offset)
	{
		drawTextBordered("", grect(x, y, w, h), text, horizontal, vertical, color, offset);
	}
	
	void drawTextBordered(chstr fontName, float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextBordered(fontName, grect(x, y, w, h), text, horizontal, vertical, April::Color(a, r, g, b), offset);
	}
	
	void drawTextBordered(float x, float y, float w, float h, chstr text, Alignment horizontal, Alignment vertical,
		unsigned char r, unsigned char g, unsigned char b, unsigned char a, gvec2 offset)
	{
		drawTextBordered("", x, y, w, h, text, horizontal, vertical, r, g, b, a, offset);
	}
	
/******* PROPERTIES ****************************************************/

	gvec2 getShadowOffset()
	{
		return shadowOffset;
	}
	
	void setShadowOffset(gvec2 value)
	{
		shadowOffset = value;
	}
	
	April::Color getShadowColor()
	{
		return shadowColor;
	}
	
	void setShadowColor(April::Color value)
	{
		shadowColor = value;
	}
	
	float getBorderOffset()
	{
		return borderOffset;
	}
	
	void setBorderOffset(float value)
	{
		borderOffset = value;
	}
	
	April::Color getBorderColor()
	{
		return borderColor;
	}
	
	void setBorderColor(April::Color value)
	{
		borderColor = value;
	}
	
/******* OTHER *********************************************************/

	float getFontHeight(chstr fontName)
	{
		return getFont(fontName)->getHeight();
	}
	
	float getTextWidth(chstr fontName, chstr text)
	{
		harray<grect> areas;
		getFont(fontName)->testRender(grect(0, 0, 1, 1), text, LEFT, TOP, areas);
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
		harray<hstr> lines = f->testRender(grect(0, 0, maxWidth, 100000), text, LEFT_WRAPPED, TOP, areas);
		return (lines.size() * f->getLineHeight());
	}
	
	int getTextCount(chstr fontName, chstr text, float maxWidth, float maxHeight)
	{
		harray<grect> areas;
		harray<hstr> lines = getFont(fontName)->testRender(grect(0, 0, maxWidth, maxHeight), text, LEFT_WRAPPED, TOP, areas);
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
		logMessage(hsprintf("loading font %s", filename.c_str()));
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
