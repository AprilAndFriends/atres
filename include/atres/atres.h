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
/// Defines all accessible functions for atres.

#ifndef ATRES_H
#define ATRES_H

#include <hltypes/hstring.h>

#include "Renderer.h"
#include "atresExport.h"

namespace atres
{
	extern hstr logTag;

	atresFnExport void init();
	atresFnExport void destroy();
	atresFnExport int getTextureSize();
	atresFnExport void setTextureSize(int value);
	atresFnExport bool isAllowAlphaTextures();
	atresFnExport void setAllowAlphaTextures(bool value);

}

#endif

