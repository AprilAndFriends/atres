/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H

#include <atres/atres.h>

#include "atresttf.h"
#include "freetype.h"

namespace atresttf
{
	FT_Library library;

    void init()
    {
		FT_Error error = FT_Init_FreeType(&library);
		if (error != 0)
		{
			atres::log("Error loading FreeType 2!");
		}
	}
    
    void destroy()
    {
    }

}
