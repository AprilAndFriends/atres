/// @file
/// @version 4.0
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
	static int textureSize = 1024;
	static bool allowAlphaTextures = true;

	void init()
	{
		hlog::write(logTag, "Initializing Atres.");
		atres::renderer = new Renderer();
	}
	
	void destroy()
	{
		hlog::write(logTag, "Destroying Atres.");
		if (atres::renderer != NULL)
		{
			delete atres::renderer;
			atres::renderer = NULL;
		}
	}

	int getTextureSize()
	{
		return textureSize;
	}

	void setTextureSize(int value)
	{
		textureSize = value;
	}

	bool isAllowAlphaTextures()
	{
		return allowAlphaTextures;
	}

	void setAllowAlphaTextures(bool value)
	{
		allowAlphaTextures = value;
	}

}
