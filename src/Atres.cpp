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
#include "Atres.h"
#include "Font.h"
#include "Exception.h"
#include <stdlib.h>

namespace Atres
{
    std::map<std::string,Font*> fonts;
	
	RenderInterface* render_iface=0;
	
	Font* default_font=0;

    void init(RenderInterface* iface)
    {
		render_iface=iface;
    }
    
    void destroy()
    {
        for (std::map<std::string,Font*>::iterator it=fonts.begin();it!=fonts.end();it++)
		{
			delete it->second;
		}
    }

	float drawText(std::string font_name,float x,float y,std::string text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		float w;
		Font* f=getFont(font_name);
		if (effect == SHADOW || effect == BORDER)
			f->render(x+1,y+1,100000,alignment,text,1,0,0,0,a,0,0);
		if (effect == BORDER)
		{
			f->render(x-1,y-1,100000,alignment,text,1,0,0,0,a,0,0);
			f->render(x+1,y-1,100000,alignment,text,1,0,0,0,a,0,0);
			f->render(x-1,y+1,100000,alignment,text,1,0,0,0,a,0,0);
		}
		f->render(x, y, 100000, alignment,text,1,r,g,b,a,&w,0);
		return w;
	}

	float drawWrappedText(std::string font_name,float x,float y,float w_max,std::string text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		float h;
		Font* f=getFont(font_name);
		if (effect == SHADOW || effect == BORDER)
			f->render(x+1,y+1,w_max,alignment,text,1,0,0,0,a,0,0);
		if (effect == BORDER)
		{
			f->render(x-1,y-1,w_max,alignment,text,1,0,0,0,a,0,0);
			f->render(x+1,y-1,w_max,alignment,text,1,0,0,0,a,0,0);
			f->render(x-1,y+1,w_max,alignment,text,1,0,0,0,a,0,0);
		}
		f->render(x,y,w_max,alignment,text,1,r,g,b,a,0,&h);
		return h;
	}

	float drawText(float x,float y,std::string text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawText("",x,y,text,r,g,b,a,alignment,effect);
	}

	float drawWrappedText(float x,float y,float w_max,std::string text,float r,float g,float b,float a,Alignment alignment,Effect effect)
	{
		return drawWrappedText("",x,y,w_max,text,r,g,b,a,alignment,effect);
	}

	float getTextWidth(std::string font_name,std::string text)
	{
		float w;

		getFont(font_name)->render(0,0, 100000,LEFT,text,0,1,1,1,1,&w,0);
		return w;
	}

	float getWrappedTextHeight(std::string font_name,float w_max,std::string text)
	{
		float h;
		getFont(font_name)->render(0,0,w_max,LEFT,text,0,1,1,1,1,0,&h);
		return h;
	}
	
	void setDefaultFont(std::string name)
	{
		default_font=fonts[name];
	}
    
    void loadFont(std::string filename)
    {
        Font* f=new Font(filename);
        fonts[f->getName()]=f;
		if (default_font == 0) default_font=f;
    }
    
    Font* getFont(std::string name)
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
				f=getFont(name.substr(0,pos));
				f->setScale(atof(name.substr(pos+1,10).c_str()));
			}
			else throw FontNotFoundException("Font "+name+" does not exist!");
		}
		else
		{
			f=fonts[name];
			f->setScale(0);
		}
        return f;
    }
	
	void setRenderInterface(RenderInterface* iface)
	{
		render_iface=iface;
	}
	
	RenderInterface* getRenderInterface()
	{
		return render_iface;
	}
}
