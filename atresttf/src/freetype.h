/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions for Atres TTF.

#ifndef ATRESTTF_FREETYPE_H
#define ATRESTTF_FREETYPE_H

#include <hltypes/hmap.h>

namespace atres
{
	class FontResource;
}

namespace atresttf
{
	FT_Library getLibrary();
	FT_Face getFace(atres::FontResource* fontResource);
	void addFace(atres::FontResource* fontResource, FT_Face face);
	void removeFace(atres::FontResource* fontResource, FT_Face face);

	extern FT_Library library;
	extern hmap<atres::FontResource*, FT_Face> faces;

};

#endif

