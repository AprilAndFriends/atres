/// @file
/// @version 4.0
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
		FontIconMap(chstr fontDirectory, chstr name, float height, float scale, float lineHeight = 0.0f, float spacing = 0.0f);
		~FontIconMap();

	protected:
		hstr fontDirectory;
		float spacing;

		bool _isAllowAlphaTextures();

		bool _load();

		april::Image* _loadIconImage(chstr iconName, bool initial, int& advance);

	};

}

#endif

