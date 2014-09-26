/// @file
/// @version 3.4
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
		DEPRECATED_ATTRIBUTE FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true);
		DEPRECATED_ATTRIBUTE FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, float descender, bool loadBasicAscii);
		
		~FontTtf();

		HL_DEFINE_GET(hstr, fontFilename, FontFilename);
		april::Texture* getTexture(unsigned int charCode);
		bool hasChar(unsigned int charCode);

		DEPRECATED_ATTRIBUTE FontTtf(unsigned char* data, int dataSize, chstr name, float height, float scale, float lineHeight = 0.0f, bool loadBasicAscii = true) : atres::Font(name)
		{
			this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
			this->fontStream.write_raw(data, dataSize);
			this->_initializeFont();
		}
		DEPRECATED_ATTRIBUTE FontTtf(unsigned char* data, int dataSize, chstr name, float height, float scale, float lineHeight, float descender, bool loadBasicAscii) : atres::Font(name)
		{
			this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
			this->descender = descender;
			this->customDescender = true;
			this->fontStream.write_raw(data, dataSize);
			this->_initializeFont();
		}

	protected:
		int penX;
		int penY;
		int rowHeight;
		hstr fontFilename;
		hstream fontStream;
		bool loadBasicAscii;

		void _setInternalValues(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii);

		void _initializeFont();
		april::Texture* _createTexture();
		void _loadBasicCharacters();
		bool _addCharacterBitmap(unsigned int charCode, bool ignoreCharacterEnabled = false);

	};

	DEPRECATED_ATTRIBUTE typedef FontTtf FontResourceTtf;

}

#endif

