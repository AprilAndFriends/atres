/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a font that uses a bitmap as source.

#ifndef ATRES_FONT_BITMAP_H
#define ATRES_FONT_BITMAP_H

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "Font.h"

namespace atres
{
	class atresExport FontBitmap : public Font
	{
	public:
		FontBitmap(chstr filename);
		~FontBitmap();

	protected:
		hstr fontFilename;

		bool _load();

	};

}

#endif

