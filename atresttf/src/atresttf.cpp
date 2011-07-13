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
#include <atres/FontResource.h>
#include <hltypes/exception.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "freetype.h"

namespace atresttf
{
	FT_Library library = NULL;
	hmap<atres::FontResource*, FT_Face> faces;

    void init()
    {
		atres::log("initializing atresttf");
		FT_Error error = FT_Init_FreeType(&library);
		if (error != 0)
		{
			atres::log("Error while initializing atresttf!");
		}
	}
    
    void destroy()
    {
		atres::log("destroying atresttf");
		FT_Error error = FT_Done_FreeType(library);
		if (error != 0)
		{
			atres::log("Error while destroying atresttf!");
		}
    }

	FT_Library getLibrary()
	{
		if (library == NULL)
		{
			hl_exception("Error: AtresTtf not initialized!");
		}
		return library;
	}

	FT_Face getFace(atres::FontResource* fontResource)
	{
		return faces[fontResource];
	}

	void setFace(atres::FontResource* fontResource, FT_Face face)
	{
		faces[fontResource] = face;
	}

}
