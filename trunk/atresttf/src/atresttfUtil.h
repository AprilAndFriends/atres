/// @file
/// @version 3.41
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines utility functions.

#ifndef ATRESTTF_ATRESTTF_UTIL_H
#define ATRESTTF_ATRESTTF_UTIL_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hltypes/hmap.h>

namespace atres
{
	class Font;
}

namespace atresttf
{
	FT_Library getLibrary();
	FT_Face getFace(atres::Font* font);
	void registerFace(atres::Font* font, FT_Face face);
	void destroyFace(atres::Font* font);

	extern FT_Library library;
	extern hmap<atres::Font*, FT_Face> faces;

};

#endif

