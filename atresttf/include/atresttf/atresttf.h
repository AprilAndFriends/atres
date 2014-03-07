/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
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
	extern hstr logTag;

	atresttfFnExport void init();
	atresttfFnExport void destroy();
	atresttfFnExport int getTextureSize();
	atresttfFnExport void setTextureSize(int value);
	atresttfFnExport bool isAllowAlphaTextures();
	atresttfFnExport void setAllowAlphaTextures(bool value);
	atresttfFnExport harray<hstr> getSystemFonts();
	atresttfFnExport hstr findSystemFontFilename(chstr name);
	atresttfFnExport hstr getSystemFontsPath();

};

#endif

