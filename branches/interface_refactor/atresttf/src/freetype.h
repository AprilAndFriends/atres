/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions for Atres TTF.

#ifndef ATRESTTF_FREETYPE_H
#define ATRESTTF_FREETYPE_H

#include "atresttfExport.h"

namespace atresttf
{
	FT_Library getLibrary();

	extern FT_Library library;

};

#endif

