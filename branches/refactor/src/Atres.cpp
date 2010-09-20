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
	
	float drawText(chstr font_name,float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		float w;
		Font* f=getFont(font_name);
		if (effect == SHADOW || effect == BORDER || effect == BORDER_4)
		{
			f->render(x+1,y+1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
		}
		if (effect == BORDER || effect == BORDER_4)
		{
			f->render(x-1,y-1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
			f->render(x+1,y-1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
			f->render(x-1,y+1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
		}
		if (effect == BORDER)
		{
			f->render(x,y-1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
			f->render(x,y+1,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
			f->render(x+1,y,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
			f->render(x-1,y,w_max,h_max,alignment,0,text,1,0,0,0,a,0,0,0);
		}
		flushRenderOperations();
		f->render(x,y,w_max,h_max,alignment,0,text,1,r,g,b,a,&w,0,0);
		flushRenderOperations();
		return w;
	}

	float drawWrappedText(chstr font_name,float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		float h;
		Font* f=getFont(font_name);
		if (effect == SHADOW || effect == BORDER || effect == BORDER_4)
		{
			f->render(x+1,y+1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
		}
		if (effect == BORDER || effect == BORDER_4)
		{
			f->render(x-1,y-1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
			f->render(x+1,y-1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
			f->render(x-1,y+1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
		}
		if (effect == BORDER)
		{
			f->render(x,y-1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
			f->render(x,y+1,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
			f->render(x+1,y,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
			f->render(x-1,y,w_max,h_max,alignment,1,text,1,0,0,0,a,0,0,0);
		}
		flushRenderOperations();
		f->render(x,y,w_max,h_max,alignment,1,text,1,r,g,b,a,0,&h,0);
		flushRenderOperations();
		return h;
	}

	float drawText(float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawText("",x,y,w_max,h_max,text,r,g,b,a,alignment,effect);
	}

	float drawWrappedText(float x,float y,float w_max,float h_max,chstr text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawWrappedText("",x,y,w_max,h_max,text,r,g,b,a,alignment,effect);
	}

	float getTextWidth(chstr font_name,chstr text)
	{
		//float w;
		//getFont(font_name)->render(0,0,100000,100000,LEFT,0,text,0,1,1,1,1,&w,0,0);
		harray<grect> sizes;
		//2DO - when not wrapped, no unlimited rect is needed
		getFont(font_name)->testRender(grect(0,0,100000,100000), text, LEFT, &sizes);
		float maxWidth = 0.0f;
		foreach (grect, it, sizes)
		{
			if (maxWidth < (*it).w)
			{
				maxWidth = (*it).w;
			}
		}
		return maxWidth;
	}

	float getTextHeight(chstr font_name,chstr text)
	{
		return getFont(font_name)->getHeight();
	}
	
	float getWrappedTextHeight(chstr font_name,float w_max,chstr text)
	{
		harray<grect> sizes;
		//2DO - when not wrapped, no unlimited rect is needed
		Font* f = getFont(font_name);
		harray<hstr> lines = f->testRender(grect(0,0,w_max,100000), text, LEFT, &sizes);
		return (lines.size() * f->getLineHeight());
	}
	
	int getWrappedTextCount(chstr font_name,float w_max,float h_max,chstr text)
	{
		int count;
		getFont(font_name)->testRender(grect(0,0,w_max,h_max), text, LEFT, NULL, &count);
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
