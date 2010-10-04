/************************************************************************************\
This source file is part of the APRIL User Interface Library                         *
For latest info, see http://libaprilui.sourceforge.net/                              *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <ctype.h>
#include <stdio.h>

#include <april/RenderSystem.h>
#include <hltypes/hstring.h>

#include "Util.h"

namespace Atres
{
	int hexstr_to_int(chstr s)
	{
		int i;
		sscanf(s.c_str(), "%x", &i);
		return i;
	}
	
	April::Color hexstr_to_color(chstr hex)
	{
		unsigned char a, r, g, b;
		hstr value = hex;
		if (value(0, 2) != "0x") value = "0x" + value;
		if (value.size() == 8)
		{
			r = hexstr_to_int(value(2, 2));
			g = hexstr_to_int(value(4, 2));
			b = hexstr_to_int(value(6, 2));
			a = 255;
		}
		else if (value.size() == 10)
		{
			a = hexstr_to_int(value(2, 2));
			r = hexstr_to_int(value(4, 2));
			g = hexstr_to_int(value(6, 2));
			b = hexstr_to_int(value(8, 2));
		}
		else throw "Color format must be either 0xAARRGGBB or 0xRRGGBB";
		return April::Color(a, r, g, b);
	}
	
	bool is_hexstr(chstr hex)
	{
		if (hex.size() % 2 == 0)
		{
			const char* str = hex.c_str();
			for (int i = 0; i < hex.size(); i++)
			{
				if (!isxdigit(str[i]))
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
}
