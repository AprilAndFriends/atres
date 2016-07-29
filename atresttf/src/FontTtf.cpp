/// @file
/// @version 4.0
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

#define PTSIZE2INT(value) (int)(((value) + 63) >> 6)
#define FLOAT2PTSIZE(value) (int)((value) * 64)

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

	void FontTtf::setBorderMode(BorderMode value)
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
		FT_Library library = atresttf::getLibrary();
		FT_Face face = NULL;
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
		request.height = (long)hround((double)this->height) << 6;
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
		this->internalDescender = -(float)PTSIZE2INT(face->size->metrics.descender);
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
				this->_addCharacterBitmap(code, true);
			}
			this->textureContainers.last()->texture->unlock();
		}
		return true;
	}

	april::Image* FontTtf::_loadCharacterImage(unsigned int charCode, bool initial, int& advance, int& leftOffset, int& topOffset, int& ascender, int& descender, int& bearingX)
	{
		FT_Face face = atresttf::getFace(this);
		unsigned long charIndex = charCode;
		if (charIndex == 0xA0) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = 0x20;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			if (!initial && charCode >= 0x20)
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
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (error != 0)
			{
				hlog::error(logTag, "Could not render glyph from: " + this->fontFilename);
				return NULL;
			}
		}
		advance = PTSIZE2INT(face->glyph->advance.x);
		leftOffset = face->glyph->bitmap_left;
		topOffset = face->glyph->bitmap_top;
		ascender = (int)(-PTSIZE2INT(face->size->metrics.ascender));
		descender = (int)(-PTSIZE2INT(face->size->metrics.descender));
		bearingX = PTSIZE2INT(face->glyph->metrics.horiBearingX);
		return april::Image::create(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, april::Image::FORMAT_ALPHA);
	}

	april::Image* FontTtf::_loadBorderCharacterImage(unsigned int charCode, float borderThickness)
	{
		FT_Face face = atresttf::getFace(this);
		unsigned long charIndex = charCode;
		if (charIndex == 0xA0) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = 0x20;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			if (charCode >= 0x20)
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
		if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		{
			hlog::error(logTag, "Not an outline glyph: " + this->fontFilename);
			this->nativeBorderSupported = false; // native border actually no supported
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
		error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
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
			image = april::Image::create(bitmap.width, bitmap.rows, bitmap.buffer, april::Image::FORMAT_ALPHA);
		}
		else
		{
			// making sure data is properly copied if "pitch" does not match "width"
			image = april::Image::create(bitmap.width, bitmap.rows, april::Color::White, april::Image::FORMAT_ALPHA);
			image->write(0, 0, bitmap.width, bitmap.rows, 0, 0, bitmap.buffer, bitmap.pitch, bitmap.rows, april::Image::FORMAT_ALPHA);
		}
		FT_Done_Glyph(glyph);
		return image;
	}

}
