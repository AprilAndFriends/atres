/************************************************************************************\
This source file is part of the APRIL User Interface Library                         *
For latest info, see http://libaprilui.sourceforge.net/                              *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_UTIL_H
#define ATRES_UTIL_H

#include <april/RenderSystem.h>
#include <hltypes/hstring.h>

struct xml_node;

namespace Atres
{
	April::Color hexstr_to_color(chstr hex);
	bool is_hexstr(chstr hex);
	
}
#endif
