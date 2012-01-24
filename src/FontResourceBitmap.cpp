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
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "FontResourceBitmap.h"

namespace atres
{
	FontResourceBitmap::FontResourceBitmap(chstr filename) : FontResource(filename)
	{
		hstr path = get_basedir(filename) + "/";
		harray<hstr> lines = hfile::hread(filename).split("\n", -1, true);
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_first();
			if (!this->_readBasicParameter(line))
			{
				if (line.starts_with("Texture="))
				{
					this->texture = april::rendersys->loadTexture(path + line.replace("Texture=", ""));
				}
				else if (line.starts_with("-"))
				{
					break;
				}
			}
		}

		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		if (this->correctedHeight == 0.0f)
		{
			this->correctedHeight = this->height;
		}
		CharacterDefinition c;
		unsigned int code;
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			c.bx = 0.0f;
			c.aw = 0.0f;
			data = (*it).split(" ", -1, true);
			if (is_between(data.size(), 4, 6))
			{
				code = (unsigned int)data.pop_first();
				c.x = (float)data.pop_first();
				c.y = (float)data.pop_first();
				c.w = (float)data.pop_first();
				c.h = this->height;
				if (data.size() > 0)
				{
					c.aw = (float)data.pop_first();
					if (data.size() > 0)
					{
						c.bx = (float)data.pop_first();
					}
				}
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

	april::Texture* FontResourceBitmap::getTexture(unsigned int charcode)
	{
		return this->texture;
	}

}
