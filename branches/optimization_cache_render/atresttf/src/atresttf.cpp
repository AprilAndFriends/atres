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

	void log(chstr message)
	{
		atres::log(message, "[atresttf] ");
	}

    void init()
    {
		atresttf::log("initializing atresttf");
		FT_Error error = FT_Init_FreeType(&library);
		if (error != 0)
		{
			atresttf::log("Error while initializing atresttf!");
		}
	}
    
    void destroy()
    {
		atresttf::log("destroying atresttf");
		foreach_map (atres::FontResource*, FT_Face, it, faces)
		{
			FT_Done_Face(it->second);
		}
		FT_Error error = FT_Done_FreeType(library);
		if (error != 0)
		{
			atresttf::log("Error while destroying atresttf!");
		}
    }

	FT_Library getLibrary()
	{
		if (library == NULL)
		{
			hl_exception("Error: atresttf not initialized!");
		}
		return library;
	}

	FT_Face getFace(atres::FontResource* fontResource)
	{
		return faces[fontResource];
	}

	void addFace(atres::FontResource* fontResource, FT_Face face)
	{
		if (faces.has_key(fontResource))
		{
			atresttf::log("Cannot add Face for Font Resource " + fontResource->getName());
			return;
		}
		faces[fontResource] = face;
	}

	void removeFace(atres::FontResource* fontResource, FT_Face face)
	{
		if (!faces.has_key(fontResource))
		{
			atresttf::log("Cannot remove Face for Font Resource " + fontResource->getName());
			return;
		}
		faces.remove_key(fontResource);
	}

}