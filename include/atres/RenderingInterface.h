/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_RENDERING_IFACE_H
#define ATRES_RENDERING_IFACE_H

#include <string>
#include "AtresExport.h"

namespace Atres
{
	struct AtresExport CharacterRenderOp
	{
		unsigned int resource;
		unsigned short sx,sy,sw,sh;
		float dx,dy,dw,dh;
		unsigned char r,g,b,a, italic, underline, strikethrough;
	};
	
	class AtresExport RenderInterface
	{
	public:
		RenderInterface();
		virtual ~RenderInterface();
		
		virtual unsigned int loadResource(const char* filename)=0;
		virtual void render(CharacterRenderOp* rops,int n)=0;
	};
}

#endif

