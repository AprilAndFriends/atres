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
/// Defines all accessible functions for atres.

#ifndef ATRES_H
#define ATRES_H

#include <hltypes/hstring.h>

#include "Renderer.h"
#include "atresExport.h"

namespace atres
{
	/// @brief Used for logging display.
	atresExport extern hstr logTag;

	/// @brief Initializes ATReS.
	atresFnExport void init();
	/// @brief Destroys ATReS.
	atresFnExport void destroy();
	/// @brief Gets texture size for font textures.
	/// @return The texture size.
	atresFnExport int getTextureSize();
	/// @brief Sets texture size for font textures.
	/// @param[in] value The texture size.
	atresFnExport void setTextureSize(int value);
	/// @brief Gets whether font texture can use alpha-textures.
	/// @return True if alpha-textures can be used.
	atresFnExport bool isAllowAlphaTextures();
	/// @brief Sets whether font texture can use alpha-textures.
	/// @param[in] value Whether alpha-textures can be used.
	/// @note Useful for VRAM conservation, but not supported on some GPU drivers (depending on platform).
	atresFnExport void setAllowAlphaTextures(bool value);

}
#endif

