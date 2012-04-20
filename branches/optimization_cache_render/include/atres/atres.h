/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.5
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
    atresFnExport void init();
    atresFnExport void destroy();
	atresFnExport void setLogFunction(void (*fnptr)(chstr));
	atresFnExport void log(chstr message, chstr prefix = "[atres] ");

};

#endif

