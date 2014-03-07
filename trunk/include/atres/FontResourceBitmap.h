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
/// Defines a font.

#ifndef ATRES_FONT_RESOURCE_BITMAP_H
#define ATRES_FONT_RESOURCE_BITMAP_H

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "FontResource.h"

namespace atres
{
	class atresExport FontResourceBitmap : public FontResource
	{
	public:
		FontResourceBitmap(chstr filename);
		~FontResourceBitmap();

	};
}

#endif

