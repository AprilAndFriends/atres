/// @file
/// @version 4.0
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

// don't change these anymore, these are the optimal settings proven through a variety of fonts
#define SAFE_SPACE 2
#define CHARACTER_SPACE 0

namespace atres
{
	FontDynamic::StructuringImageContainer::StructuringImageContainer(april::Image* image, BorderMode borderMode, float borderThickness)
	{
		this->image = image;
		this->borderMode = borderMode;
		this->borderThickness = borderThickness;
	}

	FontDynamic::StructuringImageContainer::~StructuringImageContainer()
	{
		delete this->image;
	}

	FontDynamic::FontDynamic(chstr name) : Font(name)
	{
	}

	FontDynamic::~FontDynamic()
	{
		foreach (StructuringImageContainer*, it, this->structuringImageContainers)
		{
			delete (*it);
		}
	}

	void FontDynamic::setBorderMode(BorderMode value)
	{
		if (value == BorderMode::FontNative)
		{
			hlog::warnf(logTag, "BorderMode 'FontNative' is not supported in font '%s'.", this->name.cStr());
			return;
		}
		this->_setBorderMode(value);
	}

	bool FontDynamic::_isAllowAlphaTextures()
	{
		return isAllowAlphaTextures();
	}

	void FontDynamic::_tryCreateFirstTextureContainer()
	{
		if (this->textureContainers.size() == 0)
		{
			TextureContainer* textureContainer = new TextureContainer();
			textureContainer->texture = this->_createTexture();
			this->textureContainers += textureContainer;
		}
	}

	void FontDynamic::_tryCreateFirstBorderTextureContainer(float borderThickness)
	{
		harray<BorderTextureContainer*> borderTextureContainers = this->_getBorderTextureContainers(borderThickness);
		if (borderTextureContainers.size() == 0)
		{
			BorderTextureContainer* textureContainer = new BorderTextureContainer(borderThickness);
			textureContainer->texture = this->_createTexture();
			this->borderTextureContainers += textureContainer;
		}
	}

	FontDynamic::StructuringImageContainer* FontDynamic::_findStructuringImageContainer(BorderMode borderMode, float borderThickness)
	{
		foreach (StructuringImageContainer*, it, this->structuringImageContainers)
		{
			if ((*it)->borderMode == borderMode && heqf((*it)->borderThickness, borderThickness))
			{
				return (*it);
			}
		}
		return NULL;
	}

	april::Texture* FontDynamic::getTexture(unsigned int charCode)
	{
		if (!this->_addCharacterBitmap(charCode))
		{
			return NULL;
		}
		return Font::getTexture(charCode);
	}

	april::Texture* FontDynamic::getBorderTexture(unsigned int charCode, float borderThickness)
	{
		if (!this->_addBorderCharacterBitmap(charCode, borderThickness))
		{
			return NULL;
		}
		return Font::getBorderTexture(charCode, borderThickness);
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

	bool FontDynamic::hasBorderCharacter(unsigned int charCode, float borderThickness)
	{
		this->_addBorderCharacterBitmap(charCode, borderThickness);
		return Font::hasBorderCharacter(charCode, borderThickness);
	}

	bool FontDynamic::hasIcon(chstr iconName)
	{
		this->_addIconBitmap(iconName);
		return Font::hasIcon(iconName);
	}

	april::Texture* FontDynamic::_createTexture()
	{
		int textureSize = getTextureSize();
		april::Texture* texture = NULL;
		if (this->_isAllowAlphaTextures() && april::rendersys->getCaps().textureFormats.has(april::Image::FORMAT_ALPHA))
		{
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Color::Clear, april::Image::FORMAT_ALPHA, april::Texture::TYPE_MANAGED);
			if (texture != NULL && !texture->isLoaded())
			{
				april::rendersys->destroyTexture(texture);
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
		int advance = 0;
		int leftOffset = 0;
		int topOffset = 0;
		int ascender = 0;
		int descender = 0;
		int bearingX = 0;
		april::Image* image = this->_loadCharacterImage(charCode, initial, advance, leftOffset, topOffset, ascender, descender, bearingX);
		if (image == NULL)
		{
			return false;
		}
		// this makes sure that there is no vertical overlap between characters
		int lineOffset = (int)this->height - descender;
		int bearingY = -hmin(lineOffset - topOffset, 0);
//		int offsetY = hmax(lineOffset - topOffset, 0);
		int charWidth = image->w + SAFE_SPACE * 2;
		int charHeight = image->h + SAFE_SPACE * 2;
		// add bitmap to texture
		this->_tryCreateFirstTextureContainer();
		TextureContainer* textureContainer = this->_addBitmap(this->textureContainers, initial, image, charWidth, charHeight, hsprintf("character 0x%X", charCode), hmax(leftOffset, 0), 0, SAFE_SPACE);
		// character definition
		CharacterDefinition character;
		character.rect.set((float)textureContainer->penX, (float)textureContainer->penY, (float)charWidth, (float)charHeight);
		character.advance = (float)advance;
		character.bearing.set((float)bearingX, (float)(lineOffset + ascender + bearingY));
		character.offsetY = (float)(lineOffset - topOffset);
		this->characters[charCode] = character;
		textureContainer->characters += charCode;
		textureContainer->penX += charWidth + CHARACTER_SPACE * 2;
		return true;
	}

	bool FontDynamic::_addBorderCharacterBitmap(unsigned int charCode, float borderThickness)
	{
		if (Font::hasBorderCharacter(charCode, borderThickness)) // cannot use current class' implementation since it would cause recursion
		{
			return true;
		}
		april::Image* image = NULL;
		if (this->borderMode == BorderMode::FontNative)
		{
			image = this->_loadBorderCharacterImage(charCode, borderThickness);
		}
		else if (this->borderMode != BorderMode::Software)
		{
			image = this->_generateBorderCharacterImage(charCode, borderThickness);
		}
		if (image == NULL)
		{
			return false;
		}
		// this makes sure that there is no vertical overlap between characters
		BorderCharacterDefinition borderCharacter(borderThickness);
		int charWidth = image->w + SAFE_SPACE * 2;
		int charHeight = image->h + SAFE_SPACE * 2;
		// add bitmap to texture
		this->_tryCreateFirstBorderTextureContainer(borderThickness);
		harray<BorderTextureContainer*> borderTextureContainers = this->_getBorderTextureContainers(borderThickness);
		harray<TextureContainer*> textureContainers = borderTextureContainers.cast<TextureContainer*>();
		TextureContainer* textureContainer = this->_addBitmap(textureContainers, false, image, charWidth, charHeight, hsprintf("border-character 0x%X", charCode), 0, 0, SAFE_SPACE);
		if (textureContainers.size() > borderTextureContainers.size())
		{
			this->borderTextureContainers += textureContainers(borderTextureContainers.size(), textureContainers.size() - borderTextureContainers.size()).cast<BorderTextureContainer*>();
		}
		// character definition
		borderCharacter.rect.set((float)textureContainer->penX, (float)textureContainer->penY, (float)charWidth, (float)charHeight);
		this->borderCharacters[charCode] += borderCharacter;
		textureContainer->characters += charCode;
		textureContainer->penX += charWidth + CHARACTER_SPACE * 2;
		return true;
	}

	bool FontDynamic::_addIconBitmap(chstr iconName, bool initial)
	{
		if (this->icons.hasKey(iconName))
		{
			return true;
		}
		int advance = 0;
		april::Image* image = this->_loadIconImage(iconName, initial, advance);
		if (image == NULL)
		{
			return false;
		}
		// this makes sure that there is no vertical overlap between icons
		int iconHeight = image->h;
		int iconWidth = image->w;
		// add bitmap to texture
		this->_tryCreateFirstTextureContainer();
		TextureContainer* textureContainer = this->_addBitmap(this->textureContainers, initial, image, iconWidth, iconHeight, hsprintf("icon '%s'", iconName.cStr()));
		// icon definition
		IconDefinition icon;
		icon.rect.set((float)textureContainer->penX, (float)textureContainer->penY, (float)iconWidth, (float)iconHeight);
		icon.advance = (float)advance;
		this->icons[iconName] = icon;
		textureContainer->icons += iconName;
		textureContainer->penX += iconWidth + CHARACTER_SPACE * 2;
		return true;
	}

	TextureContainer* FontDynamic::_addBitmap(harray<TextureContainer*>& textureContainers, bool initial, april::Image* image, int bitmapWidth, int bitmapHeight, chstr symbol, int offsetX, int offsetY, int safeSpace)
	{
		TextureContainer* textureContainer = NULL;
		// create first texture
		if (textureContainers.size() == 0)
		{
			textureContainer = textureContainer->createNew();
			textureContainer->texture = this->_createTexture();
			textureContainers += textureContainer;
		}
		// get texture
		textureContainer = textureContainers.last();
		if (!textureContainer->texture->isLoaded()) // in case texture was unloaded, reload it here
		{
			textureContainer->texture->load();
			if (initial) // and lock it if necessary
			{
				textureContainer->texture->lock();
			}
		}
		textureContainer->penX += offsetX;
		// if icon bitmap width exceeds space, go into next line
		if (textureContainer->penX + bitmapWidth + CHARACTER_SPACE * 2 > textureContainer->texture->getWidth())
		{
			textureContainer->penX = 0;
			textureContainer->penY += textureContainer->rowHeight + CHARACTER_SPACE * 2;
			textureContainer->rowHeight = bitmapHeight;
		}
		else
		{
			textureContainer->rowHeight = hmax(textureContainer->rowHeight, bitmapHeight);
		}
		if (textureContainer->penY + textureContainer->rowHeight + CHARACTER_SPACE * 2 > textureContainer->texture->getHeight())
		{
			hlog::debugf(logTag, "Font '%s': %s does not fit, creating new texture.", this->name.cStr(), symbol.cStr());
			if (initial)
			{
				textureContainer->texture->unlock();
			}
			textureContainer = textureContainer->createNew();
			textureContainer->texture = this->_createTexture();
			if (initial)
			{
				textureContainer->texture->lock();
			}
			textureContainers += textureContainer;
			// if the icon's height is higher than the texture's height, this will obviously not work too well
		}
		textureContainer->texture->write(0, 0, image->w, image->h, textureContainer->penX + safeSpace, textureContainer->penY + offsetY + safeSpace, image);
		delete image;
		return textureContainer;
	}

	april::Image* FontDynamic::_loadCharacterImage(unsigned int charCode, bool initial, int& advance, int& leftOffset, int& topOffset, int& ascender, int& descender, int& bearingX)
	{
		return NULL;
	}

	april::Image* FontDynamic::_loadBorderCharacterImage(unsigned int charCode, float borderThickness)
	{
		return NULL;
	}

	april::Image* FontDynamic::_generateBorderCharacterImage(unsigned int charCode, float borderThickness)
	{
		int advance = 0;
		int leftOffset = 0;
		int topOffset = 0;
		int ascender = 0;
		int descender = 0;
		int bearingX = 0;
		april::Image* characterImage = this->_loadCharacterImage(charCode, false, advance, leftOffset, topOffset, ascender, descender, bearingX);
		if (characterImage == NULL)
		{
			return NULL;
		}
		int borderSize = hceil(borderThickness);
		int size = 1 + borderSize * 2;
		StructuringImageContainer* structuringImageContainer = this->_findStructuringImageContainer(this->borderMode, borderThickness);
		if (structuringImageContainer == NULL)
		{
			if (this->borderMode == BorderMode::PrerenderSquare)
			{
				structuringImageContainer = new StructuringImageContainer(april::Image::create(size, size, april::Color::White, april::Image::FORMAT_ALPHA), this->borderMode, borderThickness);
			}
			else if (this->borderMode == BorderMode::PrerenderCircle)
			{
				structuringImageContainer = new StructuringImageContainer(april::Image::create(size, size, april::Color::Clear, april::Image::FORMAT_ALPHA), this->borderMode, borderThickness);
				int index = borderSize + borderSize * size;
				structuringImageContainer->image->data[index] = 255;
				unsigned char value = 0;
				gvec2 vector;
				gvec2 range(borderThickness, 0.0f);
				for_iter (j, 0, borderSize + 1)
				{
					for_iter (i, j, borderSize + 1)
					{
						vector.set((float)i, (float)j);
						value = (unsigned char)(hclamp(borderThickness - vector.length(), 0.0f, 1.0f) * 255);
						structuringImageContainer->image->data[index + i + j * size] = value;
						structuringImageContainer->image->data[index - i + j * size] = value;
						structuringImageContainer->image->data[index + i - j * size] = value;
						structuringImageContainer->image->data[index - i - j * size] = value;
						structuringImageContainer->image->data[index + j + i * size] = value;
						structuringImageContainer->image->data[index - j + i * size] = value;
						structuringImageContainer->image->data[index + j - i * size] = value;
						structuringImageContainer->image->data[index - j - i * size] = value;
					}
				}
			}
			else if (this->borderMode == BorderMode::PrerenderDiamond)
			{
				structuringImageContainer = new StructuringImageContainer(april::Image::create(size, size, april::Color::Clear, april::Image::FORMAT_ALPHA), this->borderMode, borderThickness);
				int index = borderSize + borderSize * size;
				structuringImageContainer->image->data[index] = 255;
				for_iter (j, 0, borderSize + 1)
				{
					for_iter (i, 0, borderSize + 1 - j)
					{
						structuringImageContainer->image->data[index + i + j * size] = 255;
						structuringImageContainer->image->data[index - i + j * size] = 255;
						structuringImageContainer->image->data[index + i - j * size] = 255;
						structuringImageContainer->image->data[index - i - j * size] = 255;
					}
				}
			}
			if (structuringImageContainer != NULL)
			{
				this->structuringImageContainers += structuringImageContainer;
			}
		}
		if (structuringImageContainer == NULL)
		{
			delete characterImage;
			return NULL;
		}
		april::Image* image = april::Image::create(characterImage->w + borderSize * 2, characterImage->h + borderSize * 2, april::Color::Clear, april::Image::FORMAT_ALPHA);
		image->write(0, 0, characterImage->w, characterImage->h, borderSize, borderSize, characterImage);
		delete characterImage;
		if (!image->dilate(structuringImageContainer->image))
		{
			delete image;
			image = NULL;
		}
		return image;
	}

	april::Image* FontDynamic::_loadIconImage(chstr iconName, bool initial, int& advance)
	{
		return NULL;
	}

}
