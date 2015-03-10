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
			line = lines.removeFirst();
			if (!this->_readBasicParameter(line))
			{
				if (line.startsWith("Texture="))
				{
					textureContainer = new atres::TextureContainer();
					textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, line.replaced("Texture=", "")), april::Texture::TYPE_IMMUTABLE, april::Texture::LOAD_ASYNC);
					this->textureContainers += textureContainer;
				}
				else if (line.startsWith("MultiTexture="))
				{
					harray<hstr> textureNames = line.replaced("MultiTexture=", "").split("\t");
					foreach (hstr, it, textureNames)
					{
						textureContainer = new atres::TextureContainer();
						textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, (*it)), april::Texture::TYPE_IMMUTABLE, april::Texture::LOAD_ASYNC);
						this->textureContainers += textureContainer;
					}
					multiTexture = true;
				}
				else if (line.startsWith("#") && this->textureContainers.size() > 0)
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
			if (!lines.first().startsWith("#"))
			{
				break;
			}
			lines.removeFirst();
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
				code = (unsigned int)data.removeFirst();
				if (multiTexture)
				{
					textureIndex = (int)data.removeFirst();
				}
				c.x = (float)data.removeFirst();
				c.y = (float)data.removeFirst();
				c.w = (float)data.removeFirst();
				c.h = this->height;
				if (data.size() > 0)
				{
					c.aw = (float)data.removeFirst();
					if (data.size() > 0)
					{
						c.bx = (float)data.removeFirst();
						if (data.size() > 0)
						{
							c.h = c.aw;
							c.aw = c.bx;
							c.bx = (float)data.removeFirst();
							if (data.size() > 0)
							{
								c.by = (float)data.removeFirst();
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
