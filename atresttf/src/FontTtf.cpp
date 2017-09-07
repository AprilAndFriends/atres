/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "atresttfUtil.h"
#include "FontTtf.h"

#define UNICODE_CHAR_SPACE 0x20
#define UNICODE_CHAR_NON_BREAKING_SPACE 0xA0

#define FLOAT2PTLONG(value) (long)((value) * 64)
#define FLOAT2PTSIZE(value) (int)((value) * 64)
#define PTSIZE2FLOAT(value) ((value) / 64.0f)

namespace atresttf
{
	FontTtf::FontTtf(chstr filename, bool loadBasicAscii) : atres::FontDynamic(filename)
	{
		this->customDescender = false;
		this->loadBasicAscii = loadBasicAscii;
		hstr path = hrdir::baseDir(filename);
		harray<hstr> lines = hresource::hread(filename).split("\n", -1, true);
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.removeFirst();
			if (!this->_readBasicParameter(line))
			{
				if (line.startsWith("File="))
				{
					this->fontFilename = hrdir::joinPath(path, line.replaced("File=", ""), false);
				}
			}
		}
	}

	FontTtf::FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii) : atres::FontDynamic(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
	}
	
	FontTtf::FontTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii) : atres::FontDynamic(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->strikeThroughOffset = strikeThroughOffset;
		this->underlineOffset = underlineOffset;
	}
	
	FontTtf::FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii) : atres::FontDynamic(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->fontStream.writeRaw(stream);
	}

	FontTtf::FontTtf(hstream& stream, chstr name, float height, float scale, float lineHeight, float descender, float strikeThroughOffset, float underlineOffset, bool loadBasicAscii) : atres::FontDynamic(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->strikeThroughOffset = strikeThroughOffset;
		this->underlineOffset = underlineOffset;
		this->fontStream.writeRaw(stream);
	}

	void FontTtf::_setInternalValues(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii)
	{
		this->fontFilename = fontFilename;
		this->name = name;
		this->height = height;
		this->baseScale = scale;
		this->scale = scale;
		this->lineHeight = lineHeight;
		this->loadBasicAscii = loadBasicAscii;
		this->descender = 0.0f;
		this->internalDescender = 0.0f;
		this->strikeThroughOffset = 0.0f;
		this->underlineOffset = 0.0f;
		this->customDescender = false;
	}

	FontTtf::~FontTtf()
	{
		if (this->loaded)
		{
			atresttf::destroyFace(this);
		}
	}

	void FontTtf::setBorderMode(const BorderMode& value)
	{
		this->_setBorderMode(value);
	}

	bool FontTtf::_load()
	{
		if (this->fontStream.size() == 0)
		{
			if (this->fontFilename == "")
			{
				this->fontFilename = atresttf::findSystemFontFilename(this->name);
			}
			if (this->fontFilename == "") // no font file
			{
				return false;
			}
			if (!hresource::exists(this->fontFilename) && !hfile::exists(this->fontFilename)) // font file does not exist
			{
				hlog::error(logTag, "Could not find: " + this->fontFilename);
				return false;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		// libfreetype stuff
		if (this->fontStream.size() == 0)
		{
			this->fontStream.clear();
			if (hresource::exists(this->fontFilename)) // prefer local fonts
			{
				hresource file;
				file.open(this->fontFilename);
				this->fontStream.writeRaw(file);
			}
			else
			{
				hfile file;
				file.open(this->fontFilename);
				this->fontStream.writeRaw(file);
			}
		}
		FT_Library library = atresttf::getLibrary();
		FT_Face face = NULL;
		FT_Error error = FT_New_Memory_Face(library, (unsigned char*)this->fontStream, (FT_Long)this->fontStream.size(), 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			hlog::error(logTag, "Format not supported in: " + this->fontFilename);
			this->fontStream.clear();
			return false;
		}
		if (error != 0)
		{
			hlog::error(logTag, "Could not read face 0 in: " + this->fontFilename + "; Error code: " + hstr(error));
			this->fontStream.clear();
			return false;
		}
		FT_Size_RequestRec request;
		memset(&request, 0, sizeof(FT_Size_RequestRec));
		request.height = FLOAT2PTLONG(hround((double)this->height));
		request.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
		error = FT_Request_Size(face, &request);
		if (error != 0)
		{
			hlog::error(logTag, "Could not set font size in: " + this->fontFilename);
			this->fontStream.clear();
			FT_Done_Face(face);
			return false;
		}
		if (!atres::FontDynamic::_load())
		{
			hlog::error(logTag, "Could not load base class in: " + this->fontFilename);
			this->fontStream.clear();
			FT_Done_Face(face);
			return false;
		}
		this->internalDescender = -PTSIZE2FLOAT(face->size->metrics.descender);
		if (!this->customDescender)
		{
			this->descender = this->internalDescender;
		}
		atresttf::registerFace(this, face);
		// adding all base ASCII characters right away
		if (this->loadBasicAscii)
		{
			this->_tryCreateFirstTextureContainer();
			this->textureContainers.last()->texture->lock();
			for_itert (unsigned int, code, 32, 128)
			{
				this->_tryAddCharacterBitmap(code, true);
			}
			this->textureContainers.last()->texture->unlock();
		}
		return true;
	}

	april::Image* FontTtf::_loadCharacterImage(unsigned int charCode, bool initial, float& advance, int& leftOffset, int& topOffset, float& ascender, float& descender, float& bearingX)
	{
		FT_Face face = atresttf::getFace(this);
		unsigned long charIndex = charCode;
		if (charIndex == UNICODE_CHAR_NON_BREAKING_SPACE) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = UNICODE_CHAR_SPACE;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			if (!initial && charCode >= UNICODE_CHAR_SPACE)
			{
				hlog::debugf(logTag, "Character '0x%X' does not exist in: %s", charCode, this->fontFilename.cStr());
			}
			return NULL;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error != 0)
		{
			hlog::error(logTag, "Could not load glyph from: " + this->fontFilename);
			return NULL;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);
			if (error != 0)
			{
				hlog::error(logTag, "Could not render glyph from: " + this->fontFilename);
				return NULL;
			}
		}
		advance = PTSIZE2FLOAT(face->glyph->advance.x);
		leftOffset = face->glyph->bitmap_left;
		topOffset = face->glyph->bitmap_top;
		ascender = -PTSIZE2FLOAT(face->size->metrics.ascender);
		descender = -PTSIZE2FLOAT(face->size->metrics.descender);
		bearingX = PTSIZE2FLOAT(face->glyph->metrics.horiBearingX);
		return april::Image::create(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, april::Image::Format::Alpha);
	}

	april::Image* FontTtf::_loadBorderCharacterImage(unsigned int charCode, float borderThickness)
	{
		FT_Face face = atresttf::getFace(this);
		unsigned long charIndex = charCode;
		if (charIndex == UNICODE_CHAR_NON_BREAKING_SPACE) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = UNICODE_CHAR_SPACE;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			if (charCode >= UNICODE_CHAR_SPACE)
			{
				hlog::debugf(logTag, "Border character '0x%X' does not exist in: %s", charCode, this->fontFilename.cStr());
			}
			return NULL;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error != 0)
		{
			hlog::error(logTag, "Could not load glyph from: " + this->fontFilename);
			return NULL;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) // native border actually not supported in this font
		{
			hlog::error(logTag, "Not an outline glyph: " + this->fontFilename);
			return NULL;
		}
		FT_Stroker stroker;
		error = FT_Stroker_New(atresttf::getLibrary(), &stroker);
		if (error != 0)
		{
			hlog::error(logTag, "Could not create stroker: " + this->fontFilename);
			return NULL;
		}
		FT_Stroker_Set(stroker, FLOAT2PTSIZE(borderThickness), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_MITER_FIXED, 1 << 16);
		FT_Glyph glyph;
		error = FT_Get_Glyph(face->glyph, &glyph);
		if (error != 0)
		{
			hlog::error(logTag, "Could not get glyph from: " + this->fontFilename);
			FT_Stroker_Done(stroker);
			return NULL;
		}
		error = FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
		FT_Stroker_Done(stroker);
		if (error != 0)
		{
			hlog::error(logTag, "Could not stroke: " + this->fontFilename);
			FT_Done_Glyph(glyph);
			return NULL;
		}
		error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LIGHT, NULL, true);
		if (error != 0)
		{
			hlog::error(logTag, "Could not render bitmap: " + this->fontFilename);
			FT_Done_Glyph(glyph);
			return NULL;
		}
		FT_Bitmap bitmap = ((FT_BitmapGlyph)glyph)->bitmap;
		april::Image* image = NULL;
		if (bitmap.width == (unsigned int)bitmap.pitch)
		{
			image = april::Image::create(bitmap.width, bitmap.rows, bitmap.buffer, april::Image::Format::Alpha);
		}
		else
		{
			// making sure data is properly copied if "pitch" does not match "width"
			image = april::Image::create(bitmap.width, bitmap.rows, april::Color::White, april::Image::Format::Alpha);
			image->write(0, 0, bitmap.width, bitmap.rows, 0, 0, bitmap.buffer, bitmap.pitch, bitmap.rows, april::Image::Format::Alpha);
		}
		FT_Done_Glyph(glyph);
		return image;
	}

	float FontTtf::getKerning(unsigned int previousCharCode, unsigned int charCode)
	{
		if (previousCharCode == 0 || charCode == 0)
		{
			return 0.0f;
		}
		FT_Face face = atresttf::getFace(this);
		if (!FT_HAS_KERNING(face))
		{
			return 0.0f;
		}
		std::pair<unsigned int, unsigned int> key;
		if (this->kerningCache.hasKey(key))
		{
			return this->kerningCache[key];
		}
		unsigned long charIndex = charCode;
		if (charIndex == UNICODE_CHAR_NON_BREAKING_SPACE) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = UNICODE_CHAR_SPACE;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			this->kerningCache[key] = 0.0f;
			return this->kerningCache[key];
		}
		unsigned long previousCharInex = previousCharCode;
		if (previousCharInex == UNICODE_CHAR_NON_BREAKING_SPACE) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			previousCharInex = UNICODE_CHAR_SPACE;
		}
		unsigned int previousGlyphIndex = FT_Get_Char_Index(face, previousCharInex);
		if (previousGlyphIndex == 0)
		{
			this->kerningCache[key] = 0.0f;
			return this->kerningCache[key];
		}
		FT_Vector kerningVector;
		FT_Error error = FT_Get_Kerning(face, previousGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &kerningVector);
		if (error != 0)
		{
			hlog::errorf(logTag, "Could not get kerning for pair 0x%2X,0x%2X, error: 0x%2X", previousGlyphIndex, glyphIndex, error);
			this->kerningCache[key] = 0.0f;
			return this->kerningCache[key];
		}
		this->kerningCache[key] = PTSIZE2FLOAT(kerningVector.x);
		return this->kerningCache[key];
	}

}
