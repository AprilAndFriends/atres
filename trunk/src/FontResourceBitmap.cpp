/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "FontResourceBitmap.h"

namespace atres
{
	FontResourceBitmap::FontResourceBitmap(chstr filename) : FontResource(filename)
	{
		hstr path = get_basedir(filename) + "/";
		harray<hstr> lines = hresource::hread(filename).split("\n", -1, true);
		hstr line;
		bool multiTexture = false;
		atres::TextureContainer* textureContainer = NULL;
		while (lines.size() > 0)
		{
			line = lines.pop_first();
			if (!this->_readBasicParameter(line))
			{
				if (line.starts_with("Texture="))
				{
					textureContainer = new atres::TextureContainer();
					textureContainer->texture = april::rendersys->createTexture(path + line.replace("Texture=", ""));
					this->textureContainers += textureContainer;
				}
				else if (line.starts_with("MultiTexture="))
				{
					harray<hstr> textureNames = line.replace("MultiTexture=", "").split("\t");
					foreach (hstr, it, textureNames)
					{
						textureContainer = new atres::TextureContainer();
						textureContainer->texture = april::rendersys->createTexture(path + (*it));
						this->textureContainers += textureContainer;
					}
					multiTexture = true;
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
		int minAttribute = (multiTexture ? 5 : 4);
		int maxAttribute = (multiTexture ? 8 : 7);
		int textureIndex = 0;
		foreach (hstr, it, lines)
		{
			c.bx = 0.0f;
			c.by = 0.0f;
			c.aw = 0.0f;
			data = (*it).split(" ", -1, true);
			if (is_between(data.size(), minAttribute, maxAttribute))
			{
				code = (unsigned int)data.pop_first();
				if (multiTexture)
				{
					textureIndex = (int)data.pop_first();
				}
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
						if (data.size() > 0)
						{
							c.h = c.aw;
							c.aw = c.bx;
							c.bx = (float)data.pop_first();
							if (data.size() > 0)
							{
								c.by = (float)data.pop_first();
							}
						}
					}
				}
				if (c.aw == 0.0f)
				{
					c.aw = c.w;
				}
				this->characters[code] = c;
				this->textureContainers[textureIndex]->characters += code;
			}
		}
		this->loaded = false;
	}

	FontResourceBitmap::~FontResourceBitmap()
	{
	}

}
