/// @file
/// @author  Kresimir Spes
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
/// Defines all functions for text rendering.

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
	
}

#endif

