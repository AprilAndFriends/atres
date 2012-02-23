/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a TTF font.

#ifndef ATRESTTF_FONT_RESOURCE_TTF_H
#define ATRESTTF_FONT_RESOURCE_TTF_H

#include <atres/FontResource.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresttfExport.h"

namespace april
{
	class Texture;
}

namespace atresttf
{
	struct TextureContainer
	{
		april::Texture* texture;
		harray<unsigned int> characters;
		int penX;
		int penY;

		TextureContainer();
	};

	class atresttfExport FontResourceTtf : public atres::FontResource
	{
	public:
		FontResourceTtf(chstr filename);
		FontResourceTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight = 0.0f, float correctedHeight = 0.0f);
		~FontResourceTtf();

		april::Texture* getTexture(unsigned int charcode);
		bool hasChar(unsigned int charcode);

	protected:
		harray<TextureContainer*> textureContainers;
		hstr fontFilename;
		unsigned char* fontFile;

		void _initializeFont();
		bool _addCharacterBitmap(unsigned int charcode, bool ignoreCharacterEnabled = false);

	};
}

#endif

