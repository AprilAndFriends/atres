/// @file
/// @version 5.0
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
	FontBitmap::FontBitmap(chstr filename) :
		Font(filename)
	{
		this->fontFilename = filename;
	}

	FontBitmap::~FontBitmap()
	{
	}

	bool FontBitmap::_load()
	{
		if (!Font::_load())
		{
			return false;
		}
		if (!hresource::exists(this->fontFilename))
		{
			return false;
		}
		hstr path = hrdir::baseDir(this->fontFilename);
		harray<hstr> lines = hresource::hread(this->fontFilename).split("\n", -1, true);
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
					textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, line.replaced("Texture=", "")), april::Texture::Type::Immutable, april::Texture::LoadMode::Async);
					textureContainer->texture->loadMetaData();
					this->textureContainers += textureContainer;
				}
				else if (line.startsWith("MultiTexture="))
				{
					harray<hstr> textureNames = line.replaced("MultiTexture=", "").split("\t");
					foreach (hstr, it, textureNames)
					{
						textureContainer = new atres::TextureContainer();
						textureContainer->texture = april::rendersys->createTextureFromResource(hrdir::joinPath(path, (*it)), april::Texture::Type::Immutable, april::Texture::LoadMode::Async);
						textureContainer->texture->loadMetaData();
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
		CharacterDefinition* c = NULL;
		unsigned int code = 0;
		harray<hstr> data;
		int minAttribute = (multiTexture ? 5 : 4);
		int maxAttribute = (multiTexture ? 9 : 8);
		int textureIndex = 0;
		foreach (hstr, it, lines)
		{
			data = (*it).split(" ", -1, true);
			if (hbetweenII(data.size(), minAttribute, maxAttribute))
			{
				c = new CharacterDefinition();
				c->bearing.x = 0.0f;
				c->bearing.y = 0.0f;
				c->advance = 0.0f;
				code = (unsigned int)data.removeFirst();
				if (multiTexture)
				{
					textureIndex = (int)data.removeFirst();
				}
				c->rect.x = (float)data.removeFirst();
				c->rect.y = (float)data.removeFirst();
				c->rect.w = (float)data.removeFirst();
				c->rect.h = this->height;
				if (data.size() > 0)
				{
					c->advance = (float)data.removeFirst();
					if (data.size() > 0)
					{
						c->bearing.x = (float)data.removeFirst();
						if (data.size() > 0)
						{
							c->rect.h = c->advance;
							c->advance = c->bearing.x;
							c->bearing.x = (float)data.removeFirst();
							if (data.size() > 0)
							{
								c->bearing.y = (float)data.removeFirst();
							}
						}
					}
				}
				if (c->advance == 0.0f)
				{
					c->advance = c->rect.w;
				}
				this->characters[code] = c;
				this->textureContainers[textureIndex]->characters += code;
			}
		}
		return true;
	}

}
