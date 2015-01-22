/// @file
/// @version 3.44
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a TTF font.

#ifndef ATRESTTF_FONT_TTF_H
#define ATRESTTF_FONT_TTF_H

#include <atres/Font.h>
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
	class atresttfExport FontTtf : public atres::Font
	{
	public:
		FontTtf(chstr filename, bool loadBasicAscii = true);
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true);
		FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, float descender, bool loadBasicAscii);
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true);
		FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, float descender, bool loadBasicAscii);
		
		~FontTtf();

		HL_DEFINE_GET(hstr, fontFilename, FontFilename);
		april::Texture* getTexture(unsigned int charCode);
		bool hasChar(unsigned int charCode);

	protected:
		bool customDescender;
		int penX;
		int penY;
		int rowHeight;
		hstr fontFilename;
		hstream fontStream;
		bool loadBasicAscii;

		void _setInternalValues(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii);

		void _initializeFont();
		april::Texture* _createTexture();
		void _checkTextures();
		void _loadBasicCharacters();
		bool _addCharacterBitmap(unsigned int charCode, bool ignoreCharacterEnabled = false);

	};

}

#endif

