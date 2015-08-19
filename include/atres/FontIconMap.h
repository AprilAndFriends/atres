/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a font with icons instead.

#ifndef ATRESTTF_FONT_ICON_MAP_H
#define ATRESTTF_FONT_ICON_MAP_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "FontDynamic.h"

namespace atres
{
	class atresExport FontIconMap : public FontDynamic
	{
	public:
		FontIconMap(chstr name);
		~FontIconMap();

	};

}

#endif

