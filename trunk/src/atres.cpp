/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "FontResource.h"
#include "Renderer.h"

namespace atres
{
	hstr logTag = "atres";

	void init()
	{
		hlog::write(atres::logTag, "Initializing Atres.");
		atres::renderer = new Renderer();
	}
	
	void destroy()
	{
		if (atres::renderer != NULL)
		{
			hlog::write(atres::logTag, "Destroying Atres.");
			delete atres::renderer;
			atres::renderer = NULL;
		}
	}

}
