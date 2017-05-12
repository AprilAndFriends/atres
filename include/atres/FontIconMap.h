/// @file
/// @version 4.2
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
		FontIconMap(chstr fontDirectory, chstr name, float scale, float bearingX = 0.0f, float offsetY = 0.0f, float spacing = 0.0f,
			float strikeThroughOffset = 0.0f, float underlineOffset = 0.0f);
		~FontIconMap();

		HL_DEFINE_GET(float, bearingX, BearingX);
		HL_DEFINE_GET(float, offsetY, OffsetY);
		HL_DEFINE_GET(float, spacing, Spacing);

	protected:
		hstr fontDirectory;
		float bearingX;
		float offsetY;
		float spacing;

		bool _isAllowAlphaTextures() const;

		bool _load();

		april::Image* _loadIconImage(chstr iconName, bool initial, float& advance);

	};

}

#endif

