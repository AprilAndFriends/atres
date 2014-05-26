/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions for Atres TTF.

#ifndef ATRESTTF_FREETYPE_H
#define ATRESTTF_FREETYPE_H

#include <hltypes/hmap.h>

namespace atres
{
	class Font;
}

namespace atresttf
{
	FT_Library getLibrary();
	FT_Face getFace(atres::Font* font);
	void addFace(atres::Font* font, FT_Face face);
	void removeFace(atres::Font* font, FT_Face face);

	extern FT_Library library;
	extern hmap<atres::Font*, FT_Face> faces;

};

#endif

