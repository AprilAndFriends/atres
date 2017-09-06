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
/// Defines a font that uses a bitmap as source.

#ifndef ATRES_FONT_BITMAP_H
#define ATRES_FONT_BITMAP_H

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "Font.h"

namespace atres
{
	/// @brief Special handler that loads font data from a bitmap.
	class atresExport FontBitmap : public Font
	{
	public:
		/// @brief Basic constructor.
		/// @param[in] filename The filename of the bitmap font definition.
		FontBitmap(chstr filename);
		/// @brief Destructor.
		~FontBitmap();

	protected:
		/// @brief The filename of the bitmap font definition.
		hstr fontFilename;

		/// @brief Loads the font definition.
		/// @return True if successfully loaded.
		bool _load();

	};

}
#endif

