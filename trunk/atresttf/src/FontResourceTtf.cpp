/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "FontResourceTtf.h"
#include "freetype.h"

#define SAFE_SPACE 2
#define CHARACTER_SPACE 0

#define PTSIZE2INT(value) (((value) + 63) >> 6)

namespace atresttf
{
	FontResourceTtf::FontResourceTtf(chstr filename, bool loadBasicAscii) : atres::FontResource(filename)
	{
		this->fontFile = NULL;
		this->fontDataSize = 0;
		this->loadBasicAscii = loadBasicAscii;
		hstr path = hrdir::basedir(filename);
		harray<hstr> lines = hresource::hread(filename).split("\n", -1, true);
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.remove_first();
			if (!this->_readBasicParameter(line))
			{
				if (line.starts_with("File="))
				{
					this->fontFilename = hrdir::join_path(path, line.replace("File=", ""), false);
				}
			}
		}
		this->_initializeFont();
	}

	FontResourceTtf::FontResourceTtf(chstr fontFilename, chstr name, float height, float scale,
		float lineHeight, bool loadBasicAscii) : atres::FontResource(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
		this->fontFile = NULL;
		this->fontDataSize = 0;
		this->_initializeFont();
	}
	
	FontResourceTtf::FontResourceTtf(chstr fontFilename, chstr name, float height, float scale,
		float lineHeight, float descender, bool loadBasicAscii) : atres::FontResource(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->fontFile = NULL;
		this->fontDataSize = 0;
		this->_initializeFont();
	}
	
	FontResourceTtf::FontResourceTtf(unsigned char* data, int dataSize, chstr name, float height, float scale,
		float lineHeight, bool loadBasicAscii) : atres::FontResource(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->fontFile = new unsigned char[dataSize];
		memcpy(this->fontFile, data, dataSize);
		this->fontDataSize = dataSize;
		this->_initializeFont();
	}

	FontResourceTtf::FontResourceTtf(unsigned char* data, int dataSize, chstr name, float height, float scale,
		float lineHeight, float descender, bool loadBasicAscii) : atres::FontResource(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->fontFile = new unsigned char[dataSize];
		memcpy(this->fontFile, data, dataSize);
		this->fontDataSize = dataSize;
		this->_initializeFont();
	}

	void FontResourceTtf::_setInternalValues(chstr fontFilename, chstr name, float height, float scale, float lineHeight, bool loadBasicAscii)
	{
		this->fontFilename = fontFilename;
		this->name = name;
		this->height = height;
		this->baseScale = scale;
		this->scale = scale;
		this->lineHeight = lineHeight;
		this->loadBasicAscii = loadBasicAscii;
	}

	FontResourceTtf::~FontResourceTtf()
	{
		if (this->fontFile != NULL)
		{
			delete [] this->fontFile;
		}
	}

	april::Texture* FontResourceTtf::getTexture(unsigned int charCode)
	{
		bool reload = false;
		foreach (atres::TextureContainer*, it, this->textureContainers)
		{
			if (!(*it)->texture->isLoaded())
			{
				reload = true;
				break;
			}
		}
		if (reload)
		{
			// font textures were deleted somewhere for some reason (e.g. Android's onPause), initiate reloading
			this->characters.clear();
			foreach (atres::TextureContainer*, it, this->textureContainers)
			{
				delete (*it)->texture;
				delete (*it);
			}
			this->textureContainers.clear();
			this->_loadBasicCharacters();
		}
		if (!this->_addCharacterBitmap(charCode))
		{
			return NULL;
		}
		return FontResource::getTexture(charCode);
	}

	bool FontResourceTtf::hasChar(unsigned int charCode)
	{
		this->_addCharacterBitmap(charCode);
		return FontResource::hasChar(charCode);
	}
	
	void FontResourceTtf::_initializeFont()
	{
		int size = this->fontDataSize;
		if (this->fontDataSize == 0)
		{
			if (this->fontFilename == "")
			{
				this->fontFilename = atresttf::findSystemFontFilename(this->name);
			}
			if (this->fontFilename == "") // no font file
			{
				return;
			}
			if (!hresource::exists(this->fontFilename) && !hfile::exists(this->fontFilename)) // font file does not exist
			{
				hlog::error(atresttf::logTag, "Could not find: " + this->fontFilename);
				return;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		// libfreetype stuff
		FT_Library library = atresttf::getLibrary();
		FT_Face face = NULL;
		if (this->fontDataSize == 0)
		{
			if (this->fontFile != NULL) // making sure there are no memory leaks whatsoever
			{
				delete [] this->fontFile;
				this->fontFile = NULL;
			}
			if (hresource::exists(this->fontFilename)) // prefer local fonts
			{
				hresource file(this->fontFilename);
				size = file.size();
				this->fontFile = new unsigned char[size];
				file.read_raw(this->fontFile, size);
				file.close();
			}
			else
			{
				hfile file(this->fontFilename);
				size = file.size();
				this->fontFile = new unsigned char[size];
				file.read_raw(this->fontFile, size);
				file.close();
			}
		}
		FT_Error error = FT_New_Memory_Face(library, this->fontFile, size, 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			hlog::error(atresttf::logTag, "Format not supported in: " + this->fontFilename);
			delete [] this->fontFile;
			this->fontFile = NULL;
			return;
		}
		if (error != 0)
		{
			hlog::error(atresttf::logTag, "Could not read face 0 in: " + this->fontFilename + "; Error code: " + hstr(error));
			delete [] this->fontFile;
			this->fontFile = NULL;
			return;
		}
		FT_Size_RequestRec request;
		memset(&request, 0, sizeof(FT_Size_RequestRec));
		request.height = (long)hround((double)this->height) << 6;
		request.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
		error = FT_Request_Size(face, &request);
		if (error != 0)
		{
			hlog::error(atresttf::logTag, "Could not set font size in: " + this->fontFilename);
			delete [] this->fontFile;
			this->fontFile = NULL;
			FT_Done_Face(face);
			return;
		}
		if (!this->customDescender)
		{
			this->descender = -(float)PTSIZE2INT(face->size->metrics.descender);
		}
		atresttf::addFace(this, face);
		this->_loadBasicCharacters();
		this->loaded = true;
	}

	april::Texture* FontResourceTtf::_createTexture()
	{
		int textureSize = atresttf::getTextureSize();
		april::Texture* texture = NULL;
		if (atresttf::isAllowAlphaTextures())
		{
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Color::Clear, april::Image::FORMAT_ALPHA, april::Texture::TYPE_MANAGED);
			if (texture != NULL && !texture->isLoaded())
			{
				delete texture;
				texture = NULL;
			}
		}
		if (texture == NULL)
		{
			hlog::warn(atresttf::logTag, "Trying a RGBA format.");
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Color(april::Color::White, 0), april::rendersys->getNativeTextureFormat(april::Image::FORMAT_RGBA), april::Texture::TYPE_MANAGED);
		}
		return texture;
	}

	void FontResourceTtf::_loadBasicCharacters()
	{
		// creating an initial texture and texture container
		atres::TextureContainer* textureContainer = new atres::TextureContainer();
		textureContainer->texture = this->_createTexture();
		this->textureContainers += textureContainer;
		this->penX = 0;
		this->penY = 0;
		this->rowHeight = 0;
		// adding all base ASCII characters right away
		if (this->loadBasicAscii)
		{
			for_itert (unsigned int, code, 32, 128)
			{
				this->_addCharacterBitmap(code, true);
			}
		}
	}

	bool FontResourceTtf::_addCharacterBitmap(unsigned int charCode, bool ignoreCharacterEnabled)
	{
		if (this->characters.has_key(charCode))
		{
			return true;
		}
		FT_Face face = atresttf::getFace(this);
		unsigned long charIndex = charCode;
		if (charIndex == 0xA0) // non-breaking space character should be treated just like a normal space when retrieving the glyph from the font
		{
			charIndex = 0x20;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(face, charIndex);
		if (glyphIndex == 0)
		{
			if (!ignoreCharacterEnabled && charCode >= 0x20)
			{
				hlog::debugf(atresttf::logTag, "Character '0x%X' does not exist in: %s", charCode, this->fontFilename.c_str());
			}
			return false;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error != 0)
		{
			hlog::error(atresttf::logTag, "Could not load glyph from: " + this->fontFilename);
			return false;
		}
		FT_GlyphSlot glyph = face->glyph;
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (error != 0)
			{
				hlog::error(atresttf::logTag, "Could not render glyph from: " + this->fontFilename);
				return false;
			}
		}
		atres::TextureContainer* textureContainer = this->textureContainers.last();
		this->penX += hmax(glyph->bitmap_left, 0);
		// calculate some standard parameters
		int ascender = -PTSIZE2INT(face->size->metrics.ascender);
		int descender = -PTSIZE2INT(face->size->metrics.descender);
		// this makes sure that there is no vertical overlap between characters
		int lineOffset = (int)this->height - descender;
		int bearingY = -hmin(lineOffset - glyph->bitmap_top, 0);
		int offsetY = hmax(lineOffset - glyph->bitmap_top, 0);
		int charHeight = glyph->bitmap.rows + SAFE_SPACE * 2 + offsetY;
		int charWidth = glyph->bitmap.width + SAFE_SPACE * 2;
		// if character bitmap width exceeds space, go into next line
		if (this->penX + charWidth + CHARACTER_SPACE > textureContainer->texture->getWidth())
		{
			this->penX = 0;
			this->penY += this->rowHeight + CHARACTER_SPACE * 2;
			this->rowHeight = charHeight;
		}
		else
		{
			this->rowHeight = hmax(this->rowHeight, charHeight);
		}
		if (this->penY + this->rowHeight + CHARACTER_SPACE > textureContainer->texture->getHeight())
		{
			hlog::debugf(atresttf::logTag, "Font '%s': character 0x%X does not fit, creating new texture.", this->name.c_str(), charCode);
			textureContainer = new atres::TextureContainer();
			textureContainer->texture = this->_createTexture();
			this->textureContainers += textureContainer;
			this->penX = 0;
			this->penY = 0;
			// if the character's height is higher than the texture, this will obviously not work too well
		}
		if (glyph->bitmap.buffer != NULL)
		{
			textureContainer->texture->write(0, 0, glyph->bitmap.width, glyph->bitmap.rows,
				this->penX + SAFE_SPACE, this->penY + offsetY + SAFE_SPACE, glyph->bitmap.buffer, glyph->bitmap.width,
				glyph->bitmap.rows, april::Image::FORMAT_ALPHA);
		}
		atres::CharacterDefinition c;
		c.x = (float)this->penX;
		c.y = (float)this->penY;
		c.w = (float)charWidth;
		c.h = (float)charHeight;
		c.bx = (float)PTSIZE2INT(glyph->metrics.horiBearingX);
		c.by = (float)(lineOffset + ascender + bearingY);
		c.aw = (float)PTSIZE2INT(glyph->advance.x);
		this->characters[charCode] = c;
		this->penX += charWidth + CHARACTER_SPACE * 2;
		textureContainer->characters += charCode;
		return true;
	}
	
}
