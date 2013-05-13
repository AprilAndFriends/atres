/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.03
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
#include <atres/Utility.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresttfExport.h"

namespace april
{
	class Texture;
}

namespace atresttf
{
	class atresttfExport FontResourceTtf : public atres::FontResource
	{
	public:
		FontResourceTtf(chstr filename);
		FontResourceTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight = 0.0f, float correctedHeight = 0.0f);
		FontResourceTtf(unsigned char* data, int dataSize, chstr name, float height, float scale, float lineHeight = 0.0f, float correctedHeight = 0.0f);
		
		~FontResourceTtf();

		HL_DEFINE_GET(hstr, fontFilename, FontFilename);
		april::Texture* getTexture(unsigned int charCode);
		bool hasChar(unsigned int charCode);

	protected:
		int penX;
		int penY;
		int rowHeight;
		int fontDataSize;
		hstr fontFilename;
		unsigned char* fontFile;

		void _initializeFont();
		april::Texture* _createTexture();
		void _loadBasicCharacters();
		bool _addCharacterBitmap(unsigned int charCode, bool ignoreCharacterEnabled = false);

	};

}

#endif

