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
/// Defines a font.

#ifndef ATRES_FONT_RESOURCE_TTF_H
#define ATRES_FONT_RESOURCE_TTF_H

#include <atres/FontResource.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresttfExport.h"

namespace atresttf
{
	class atresttfExport FontResourceTtf : public atres::FontResource
	{
	public:
		FontResourceTtf(chstr filename);
		~FontResourceTtf();

	};
}

#endif

