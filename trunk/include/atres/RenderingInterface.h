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
#ifndef ATRES_RENDERING_IFACE_H
#define ATRES_RENDERING_IFACE_H

#include <string>

namespace Atres
{
	struct CharacterRenderOp
	{
		unsigned int resource;
		unsigned short sx,sy,sw,sh;
		float dx,dy,dw,dh;
		unsigned char r,g,b,a, italic, underline, strikethrough;
	};
	
	class RenderInterface
	{
	public:
		RenderInterface();
		virtual ~RenderInterface();
		
		virtual unsigned int loadResource(std::string filename)=0;
		virtual void render(CharacterRenderOp* rops,int n)=0;
	};
}

#endif

