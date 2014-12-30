/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/RenderSystem.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "FontBitmap.h"

namespace atres
{
	FontBitmap::FontBitmap(chstr filename) : Font(filename)
	{
		hstr path = hrdir::baseDir(filename);
		harray<hstr> lines = hresource::hread(filename).split("\n", -1, true);
		hstr line;
		bool multiTexture = false;
		atres::TextureContainer* textureContainer = NULL;
		while (lines.size() > 0)
		{
			line = lines.remove_first();
			if (!this->_readBasicParameter(line))
			{
				if (line.starts_with("Texture="))
				{
					textureContainer = new atres::TextureContainer();
					textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, line.replace("Texture=", "")), april::Texture::TYPE_IMMUTABLE, april::Texture::LOAD_ASYNC);
					this->textureContainers += textureContainer;
				}
				else if (line.starts_with("MultiTexture="))
				{
					harray<hstr> textureNames = line.replace("MultiTexture=", "").split("\t");
					foreach (hstr, it, textureNames)
					{
						textureContainer = new atres::TextureContainer();
						textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, (*it)), april::Texture::TYPE_IMMUTABLE, april::Texture::LOAD_ASYNC);
						this->textureContainers += textureContainer;
					}
					multiTexture = true;
				}
				else if (line.starts_with("#") && this->textureContainers.size() > 0)
				{
					break;
				}
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		while (true)
		{
			if (lines.size() == 0)
			{
				break;
			}
			if (!lines.first().starts_with("#"))
			{
				break;
			}
			lines.remove_first();
		}
		CharacterDefinition c;
		unsigned int code;
		harray<hstr> data;
		int minAttribute = (multiTexture ? 5 : 4);
		int maxAttribute = (multiTexture ? 9 : 8);
		int textureIndex = 0;
		foreach (hstr, it, lines)
		{
			c.bx = 0.0f;
			c.by = 0.0f;
			c.aw = 0.0f;
			data = (*it).split(" ", -1, true);
			if (hbetweenII(data.size(), minAttribute, maxAttribute))
			{
				code = (unsigned int)data.remove_first();
				if (multiTexture)
				{
					textureIndex = (int)data.remove_first();
				}
				c.x = (float)data.remove_first();
				c.y = (float)data.remove_first();
				c.w = (float)data.remove_first();
				c.h = this->height;
				if (data.size() > 0)
				{
					c.aw = (float)data.remove_first();
					if (data.size() > 0)
					{
						c.bx = (float)data.remove_first();
						if (data.size() > 0)
						{
							c.h = c.aw;
							c.aw = c.bx;
							c.bx = (float)data.remove_first();
							if (data.size() > 0)
							{
								c.by = (float)data.remove_first();
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

	FontBitmap::~FontBitmap()
	{
	}

}
