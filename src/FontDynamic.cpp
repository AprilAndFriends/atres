/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "FontDynamic.h"

#define SAFE_SPACE 2
#define CHARACTER_SPACE 0

namespace atres
{
	FontDynamic::FontDynamic(chstr name) : Font(name)
	{
		this->penX = 0;
		this->penY = 0;
		this->rowHeight = 0;
	}

	FontDynamic::~FontDynamic()
	{
	}

	april::Texture* FontDynamic::getTexture(unsigned int charCode)
	{
		if (!this->_addCharacterBitmap(charCode))
		{
			return NULL;
		}
		return Font::getTexture(charCode);
	}

	april::Texture* FontDynamic::getTexture(chstr iconName)
	{
		if (!this->_addIconBitmap(iconName))
		{
			return NULL;
		}
		return Font::getTexture(iconName);
	}

	bool FontDynamic::hasCharacter(unsigned int charCode)
	{
		this->_addCharacterBitmap(charCode);
		return Font::hasCharacter(charCode);
	}

	bool FontDynamic::hasIcon(chstr iconName)
	{
		this->_addIconBitmap(iconName);
		return Font::hasIcon(iconName);
	}

	bool FontDynamic::_load()
	{
		if (!Font::_load())
		{
			return false;
		}
		april::Texture* texture = this->_createTexture();
		if (texture == NULL)
		{
			return false;
		}
		TextureContainer* textureContainer = new TextureContainer();
		textureContainer->texture = texture;
		this->textureContainers += textureContainer;
		this->penX = 0;
		this->penY = 0;
		this->rowHeight = 0;
		return true;
	}

	april::Texture* FontDynamic::_createTexture()
	{
		int textureSize = getTextureSize();
		april::Texture* texture = NULL;
		if (isAllowAlphaTextures() && april::rendersys->getCaps().textureFormats.has(april::Image::FORMAT_ALPHA))
		{
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Color::Clear, april::Image::FORMAT_ALPHA, april::Texture::TYPE_MANAGED);
			if (texture != NULL && !texture->isLoaded())
			{
				delete texture;
				texture = NULL;
				hlog::warn(logTag, "Could not create alpha texture for font, trying an RGBA format.");
			}
		}
		if (texture == NULL)
		{
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Color::Blank, april::rendersys->getNativeTextureFormat(april::Image::FORMAT_RGBA), april::Texture::TYPE_MANAGED);
		}
		return texture;
	}

	bool FontDynamic::_addCharacterBitmap(unsigned int charCode, bool initial)
	{
		if (this->characters.hasKey(charCode))
		{
			return true;
		}
		int leftOffset = 0;
		int topOffset = 0;
		int ascender = 0;
		int descender = 0;
		int bearingX = 0;
		int advance = 0;
		april::Image* image = this->_loadCharacterImage(charCode, initial, leftOffset, topOffset, ascender, descender, bearingX, advance);
		if (image == NULL)
		{
			return false;
		}
		// calculate some standard parameters
		this->penX += hmax(leftOffset, 0);
		// this makes sure that there is no vertical overlap between characters
		int lineOffset = (int)this->height - descender;
		int bearingY = -hmin(lineOffset - topOffset, 0);
		int offsetY = hmax(lineOffset - topOffset, 0);
		int charHeight = image->h + SAFE_SPACE * 2 + offsetY;
		int charWidth = image->w + SAFE_SPACE * 2;
		// add bitmap to texture
		TextureContainer* textureContainer = this->_addBitmap(initial, image, charWidth, charHeight, offsetY, hsprintf("character 0x%X", charCode));
		// character definition
		CharacterDefinition character;
		character.rect.set((float)this->penX, (float)this->penY, (float)charWidth, (float)charHeight);
		character.bearing.set((float)bearingX, (float)(lineOffset + ascender + bearingY));
		character.advance = (float)advance;
		this->characters[charCode] = character;
		this->penX += charWidth + CHARACTER_SPACE * 2;
		textureContainer->characters += charCode;
		return true;
	}

	bool FontDynamic::_addIconBitmap(chstr iconName, bool initial)
	{
		if (this->icons.hasKey(iconName))
		{
			return true;
		}
		april::Image* image = this->_loadIconImage(iconName, initial);
		if (image == NULL)
		{
			return false;
		}
		// this makes sure that there is no vertical overlap between icons
		int iconHeight = image->h + SAFE_SPACE * 2;
		int iconWidth = image->w + SAFE_SPACE * 2;
		// add bitmap to texture
		TextureContainer* textureContainer = this->_addBitmap(initial, image, iconWidth, iconHeight, 0, hsprintf("icon '%s'", iconName.cStr()));
		// icon definition
		IconDefinition icon;
		icon.rect.set((float)this->penX, (float)this->penY, (float)iconWidth, (float)iconHeight);
		this->icons[iconName] = icon;
		this->penX += iconWidth + CHARACTER_SPACE * 2;
		textureContainer->icons += iconName;
		return true;
	}

	TextureContainer* FontDynamic::_addBitmap(bool initial, april::Image* image, int bitmapWidth, int bitmapHeight, int offsetY, chstr name)
	{
		// get texture
		TextureContainer* textureContainer = this->textureContainers.last();
		if (!textureContainer->texture->isLoaded()) // in case texture was unloaded, reload it here
		{
			textureContainer->texture->load();
			if (initial) // and lock it if necessary
			{
				textureContainer->texture->lock();
			}
		}
		// if icon bitmap width exceeds space, go into next line
		if (this->penX + bitmapWidth + CHARACTER_SPACE > textureContainer->texture->getWidth())
		{
			this->penX = 0;
			this->penY += this->rowHeight + CHARACTER_SPACE * 2;
			this->rowHeight = bitmapHeight;
		}
		else
		{
			this->rowHeight = hmax(this->rowHeight, bitmapHeight);
		}
		if (this->penY + this->rowHeight + CHARACTER_SPACE > textureContainer->texture->getHeight())
		{
			hlog::debugf(logTag, "Font '%s': %s does not fit, creating new texture.", this->name.cStr(), name.cStr());
			if (initial)
			{
				textureContainer->texture->unlock();
			}
			textureContainer = new TextureContainer();
			textureContainer->texture = this->_createTexture();
			if (initial)
			{
				textureContainer->texture->lock();
			}
			this->textureContainers += textureContainer;
			this->penX = 0;
			this->penY = 0;
			// if the icon's height is higher than the texture's height, this will obviously not work too well
		}
		textureContainer->texture->write(0, 0, image->w, image->h, this->penX + SAFE_SPACE, this->penY + offsetY + SAFE_SPACE, image);
		delete image;
		return textureContainer;
	}

	april::Image* FontDynamic::_loadCharacterImage(unsigned int charCode, bool initial, int& leftOffset, int& topOffset, int& ascender, int& descender, int& bearingX, int& advance)
	{
		return NULL;
	}
	
	april::Image* FontDynamic::_loadIconImage(chstr iconName, bool initial)
	{
		return NULL;
	}

}
