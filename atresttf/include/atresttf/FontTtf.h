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
/// Defines a TTF font.

#ifndef ATRESTTF_FONT_TTF_H
#define ATRESTTF_FONT_TTF_H

#include <atres/FontDynamic.h>
#include <atres/Utility.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresttfExport.h"

namespace april
{
	class Texture;
}

namespace atresttf
{
	/// @brief Defines a font object that can load font definitions from TTF files.
	class atresttfExport FontTtf : public atres::FontDynamic
	{
	public:
		/// @brief Constructor.
		/// @param[in] filename Filename of the font definition.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		FontTtf(chstr filename, bool loadBasicAscii = true);
		/// @brief Constructor.
		/// @param[in] fontFilename Font filename.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true);
		/// @brief Constructor.
		/// @param[in] fontFilename Font filename.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		/// @param[in] textureSize The filename texture size.
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii, int textureSize);
		/// @brief Constructor.
		/// @param[in] fontFilename Font filename.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] descender Font descender value.
		/// @param[in] strikeThroughOffset Vertical offset for strike-through.
		/// @param[in] underlineOffset Vertical offset for underline.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii);
		/// @brief Constructor.
		/// @param[in] fontFilename Font filename.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] descender Font descender value.
		/// @param[in] strikeThroughOffset Vertical offset for strike-through.
		/// @param[in] underlineOffset Vertical offset for underline.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		/// @param[in] textureSize The filename texture size.
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii, int textureSize);
		/// @brief Constructor.
		/// @param[in] stream Font file data stream.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true);
		/// @brief Constructor.
		/// @param[in] stream Font file data stream.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		/// @param[in] textureSize The filename texture size.
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii, int textureSize);
		/// @brief Constructor.
		/// @param[in] stream Font file data stream.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] descender Font descender value.
		/// @param[in] strikeThroughOffset Vertical offset for strike-through.
		/// @param[in] underlineOffset Vertical offset for underline.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii);
		/// @brief Constructor.
		/// @param[in] stream Font file data stream.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] descender Font descender value.
		/// @param[in] strikeThroughOffset Vertical offset for strike-through.
		/// @param[in] underlineOffset Vertical offset for underline.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		/// @param[in] textureSize The filename texture size.
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii, int textureSize);
		/// @brief Destructor.
		~FontTtf();

		/// @brief Font filename.
		HL_DEFINE_GET(hstr, fontFilename, FontFilename);
		/// @brief Sets the border rendering mode.
		/// @param[in] value The border rendering mode.
		void setBorderMode(const BorderMode& value);

		/// @brief Gets kerning between two char codes.
		/// @param[in] previousCharCode Character unicode value of the preceding character.
		/// @param[in] charCode Character unicode value.
		/// @return The kerning value.
		float getKerning(unsigned int previousCharCode, unsigned int charCode);

	protected:
		/// @brief Whether to use a custom descender value that overrides the actual font's descender.
		bool customDescender;
		/// @brief Font filename.
		hstr fontFilename;
		/// @brief Font file data stream.
		hstream fontStream;
		/// @brief Whether to pre-load the basic ASCII range of characters.
		bool loadBasicAscii;
		/// @brief Cache for calculated kerning values.
		/// @note Mostly used for internal optimization.
		hmap<std::pair<unsigned int, unsigned int>, float> kerningCache;

		/// @brief Loads the font definition.
		/// @param[in] fontFilename Font filename.
		/// @param[in] name Font name.
		/// @param[in] height Font height.
		/// @param[in] scale Font scale.
		/// @param[in] lineHeight Font line height.
		/// @param[in] loadBasicAscii Whether to pre-load the basic ASCII range of characters.
		/// @param[in] textureSize The filename texture size.
		/// @note This mostly exists, because constructors cannot call each other before C++11.
		void _setInternalValues(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii, int textureSize);

		/// @brief Loads the font definition.
		/// @return True if successfully loaded.
		bool _load();
		
		/// @brief Loads an character image.
		/// @param[in] charCode Character unicode value.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @param[out] advance Horizontal advance value.
		/// @param[out] leftOffset Horizontal offset from the left boundary of the bitmap.
		/// @param[out] topOffset Vertical offset from the top boundary of the bitmap.
		/// @param[out] ascender Ascender value.
		/// @param[out] descender Descender value.
		/// @param[out] bearingX Horizontal bearing.
		/// @return The loaded image.
		april::Image* _loadCharacterImage(unsigned int charCode, bool initial, float& advance, int& leftOffset, int& topOffset, float& ascender, float& descender, float& bearingX);
		/// @brief Loads a border character image.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The loaded image.
		april::Image* _loadBorderCharacterImage(unsigned int charCode, float borderThickness);

	};

}
#endif

