/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "FontResourceBitmap.h"

namespace atres
{
	FontResourceBitmap::FontResourceBitmap(chstr filename) : FontResource()
	{
		this->scale = 1.0f;
		this->baseScale = 1.0f;
		this->lineHeight = 0.0f;
		harray<hstr> lines = hfile::hread(filename).split("\n");
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			if (line.starts_with("Name="))
			{
				this->name = line.replace("Name=", "");
			}
			else if (line.starts_with("Texture="))
			{
				this->texture = april::rendersys->loadTexture(line.replace("Texture=", ""));
			}
			else if (line.starts_with("LineHeight="))
			{
				this->lineHeight = (float)line.replace("LineHeight=", "");
			}
			else if (line.starts_with("Height="))
			{
				this->height = (float)line.replace("Height=", "");
			}
			else if (line.starts_with("Scale="))
			{
				this->scale = (float)line.replace("Scale=", "");
				this->baseScale = this->scale;
			}
			else if (line.starts_with("#"))
			{
				continue;
			}
			else if (line.starts_with("-"))
			{
				break;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		CharacterDefinition c;
		unsigned int code;
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			c.aw = 0.0f;
			data = (*it).split(" ");
			if (data.size() == 5)
			{
				code = (unsigned int)data.pop_front();
				c.x = (float)data.pop_front();
				c.y = (float)data.pop_front();
				c.w = (float)data.pop_front();
				c.aw = (float)data.pop_front();
				if (c.aw == 0.0f)
				{
					c.aw = c.w;
				}
				this->characters[code] = c;
			}
		}
	}

	FontResourceBitmap::~FontResourceBitmap()
	{
	}
	
}
