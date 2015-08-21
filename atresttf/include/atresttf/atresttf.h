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
	atresttfFnExport harray<hstr> getSystemFonts();
	atresttfFnExport hstr findSystemFontFilename(chstr name);
	atresttfFnExport hstr getSystemFontsPath();

};

#endif

