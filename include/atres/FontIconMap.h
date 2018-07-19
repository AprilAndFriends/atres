/// @file
/// @version 5.0
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
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "FontDynamic.h"

namespace atres
{
	/// @brief Font for icons.
	class atresExport FontIconMap : public FontDynamic
	{
	public:
		typedef hmap<hstr, float> FontOffset;

		/// @brief Basic constructor.
		/// @param[in] fontDirectory Directory for the icons.
		/// @param[in] name Font name.
		/// @param[in] scale Font scale.
		/// @param[in] bearingX Horizontal bearing.
		/// @param[in] offsetY Vertical render offset from the baseline.
		/// @param[in] spacing Spacing between icons.
		/// @param[in] strikeThroughOffset Vertical offset of strike-through rendering.
		/// @param[in] underlineOffset Vertical offset of underline rendering.
		/// @param[in] italicSkewRatio Height ratio of skewing for italic rendering.
		FontIconMap(chstr fontDirectory, chstr name, float scale, float bearingX = 0.0f, float offsetY = 0.0f, float spacing = 0.0f,
			float strikeThroughOffset = 0.0f, float underlineOffset = 0.0f, float italicSkewRatio = 0.3f);
		/// @brief Constructor.
		/// @param[in] fontDirectory Directory for the icons.
		/// @param[in] name Font name.
		/// @param[in] scale Font scale.
		/// @param[in] bearingX Horizontal bearing.
		/// @param[in] offsetY Vertical render offset from the baseline.
		/// @param[in] spacing Spacing between icons.
		/// @param[in] strikeThroughOffset Vertical offset of strike-through rendering.
		/// @param[in] underlineOffset Vertical offset of underline rendering.
		/// @param[in] textureSize The filename texture size.
		/// @param[in] italicSkewRatio Height ratio of skewing for italic rendering.
		FontIconMap(chstr fontDirectory, chstr name, float scale, float bearingX, float offsetY, float spacing,
			float strikeThroughOffset, float underlineOffset, float italicSkewRatio, int textureSize);

		/// @brief Horizontal bearing.
		HL_DEFINE_GET(float, bearingX, BearingX);
		/// @brief Vertical render offset from the baseline.
		HL_DEFINE_GET(float, offsetY, OffsetY);
		/// @brief Spacing between icons.
		HL_DEFINE_GET(float, spacing, Spacing);
		/// @brief Special offsets for fonts.
		HL_DEFINE_GET(FontOffset, customFontOffsets, CustomFontOffsets);

		/// @brief Add custom font offset.
		/// @param[in] fontName The name of the font.
		/// @param[in] offset The offset.
		void addCustomFontOffset(chstr fontName, float offser);

	protected:
		/// @brief Directory for the icons.
		hstr fontDirectory;
		/// @brief Horizontal bearing.
		float bearingX;
		/// @brief Vertical render offset from the baseline.
		float offsetY;
		/// @brief Spacing between icons.
		float spacing;
		/// @brief Special offsets for fonts.
		FontOffset customFontOffsets;

		/// @brief Checks if alpha-textures can be used for this font.
		/// @return True if alpha-textures can be used for this font.
		bool _isAllowAlphaTextures() const;

		/// @brief Loads the font definition.
		/// @return True if successfully loaded.
		bool _load();

		/// @brief Loads an icon image.
		/// @param[in] iconName Name of the icon image to load.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @param[out] advance The Horizontal advance value.
		/// @return The loaded image.
		april::Image* _loadIconImage(chstr iconName, bool initial, float& advance);

	};

}
#endif

