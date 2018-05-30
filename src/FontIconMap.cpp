/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "FontIconMap.h"

namespace atres
{
	FontIconMap::FontIconMap(chstr fontDirectory, chstr name, float scale, float bearingX, float offsetY,
		float spacing, float strikeThroughOffset, float underlineOffset, float italicSkewRatio) : FontDynamic(name)
	{
		this->fontDirectory = fontDirectory;
		this->name = name;
		this->height = 0.0f;
		this->baseScale = scale;
		this->scale = scale;
		this->lineHeight = 0.0f;
		this->bearingX = bearingX;
		this->offsetY = offsetY;
		this->spacing = spacing;
		this->strikeThroughOffset = strikeThroughOffset;
		this->underlineOffset = underlineOffset;
		this->italicSkewRatio = italicSkewRatio;
	}

	FontIconMap::FontIconMap(chstr fontDirectory, chstr name, float scale, float bearingX, float offsetY,
		float spacing, float strikeThroughOffset, float underlineOffset, float italicSkewRatio, int textureSize) : FontDynamic(name)
	{
		this->fontDirectory = fontDirectory;
		this->name = name;
		this->height = 0.0f;
		this->baseScale = scale;
		this->scale = scale;
		this->lineHeight = 0.0f;
		this->bearingX = bearingX;
		this->offsetY = offsetY;
		this->spacing = spacing;
		this->strikeThroughOffset = strikeThroughOffset;
		this->underlineOffset = underlineOffset;
		this->italicSkewRatio = italicSkewRatio;
		this->textureSize = textureSize;
	}

	FontIconMap::~FontIconMap()
	{
	}

	bool FontIconMap::_isAllowAlphaTextures() const
	{
		return false;
	}

	bool FontIconMap::_load()
	{
		return (hrdir::exists(this->fontDirectory) && FontDynamic::_load());
	}

	april::Image* FontIconMap::_loadIconImage(chstr iconName, bool initial, float& advance)
	{
		HL_LAMBDA_CLASS(_withoutExtension, hstr, ((hstr const& filename) { return hresource::withoutExtension(filename); }));
		harray<hstr> files = hrdir::files(this->fontDirectory).mapped(&_withoutExtension::lambda);
		if (!files.has(iconName))
		{
			return NULL;
		}
		hstr filename = april::rendersys->findTextureResource(hrdir::joinPath(this->fontDirectory, iconName));
		if (filename == "")
		{
			return NULL;
		}
		april::Image* image = april::Image::createFromResource(filename);
		advance = image->w + this->spacing;
		return image;
	}

}
