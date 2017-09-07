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
/// Defines all functions for Atres TTF.

#ifndef ATRESTTF_H
#define ATRESTTF_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresttfExport.h"

namespace atresttf
{
	/// @brief Used for logging display.
	atresttfExport extern hstr logTag;

	/// @brief Initializes ATReSTTF.
	atresttfFnExport void init();
	/// @brief Destroys ATReSTTF.
	atresttfFnExport void destroy();
	/// @brief Gets a list of all installed system fonts.
	/// @return List of all installed system fonts.
	atresttfFnExport harray<hstr> getSystemFonts();
	/// @brief Finds the filename of the font with the given font name.
	/// @param[in] name Font name.
	/// @return The filename of the font.
	atresttfFnExport hstr findSystemFontFilename(chstr name);
	/// @brief Gets the path where system fonts are installed.
	/// @return The path where system fonts are installed.
	atresttfFnExport hstr getSystemFontsPath();

}
#endif

