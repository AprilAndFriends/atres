/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/Texture.h>
#include <hltypes/hstring.h>

#include "FontResourceTtf.h"

namespace atresttf
{
	FontResourceTtf::FontResourceTtf(chstr filename) : atres::FontResource(filename)
	{
	}

	FontResourceTtf::~FontResourceTtf()
	{
	}
	
	/*
	april::Texture* FontResourceTtf::getTexture(unsigned int charcode)
	{
		return NULL;
	}
	*/
	
}
