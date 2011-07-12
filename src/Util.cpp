/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ctype.h>
#include <stdio.h>

#include <april/RenderSystem.h>
#include <hltypes/hstring.h>

#include "Util.h"

namespace atres
{
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
