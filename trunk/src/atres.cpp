/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdio.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "Renderer.h"

namespace atres
{
	hstr logTag = "atres";

	void init()
	{
		hlog::write(logTag, "Initializing Atres.");
		atres::renderer = new Renderer();
	}
	
	void destroy()
	{
		if (atres::renderer != NULL)
		{
			hlog::write(logTag, "Destroying Atres.");
			delete atres::renderer;
			atres::renderer = NULL;
		}
	}

}
