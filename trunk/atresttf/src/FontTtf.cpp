/// @file
/// @version 3.44
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
#include "atresttfUtil.h"
#include "FontTtf.h"

#define SAFE_SPACE 2
#define CHARACTER_SPACE 0

#define PTSIZE2INT(value) (((value) + 63) >> 6)

namespace atresttf
{
	FontTtf::FontTtf(chstr filename, bool loadBasicAscii) : atres::Font(filename)
	{
		this->customDescender = false;
		this->penX = 0;
		this->penY = 0;this->rowHeight = 0;
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
		this->_initializeFont();
	}

	FontTtf::FontTtf(chstr fontFilename, chstr name, float height, float scale,
		float lineHeight, bool loadBasicAscii) : atres::Font(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
		this->_initializeFont();
	}
	
	FontTtf::FontTtf(chstr fontFilename, chstr name, float height, float scale,
		float lineHeight, float descender, bool loadBasicAscii) : atres::Font(name)
	{
		this->_setInternalValues(fontFilename, name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->_initializeFont();
	}
	
	FontTtf::FontTtf(hstream& stream, chstr name, float height, float scale,
		float lineHeight, bool loadBasicAscii) : atres::Font(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->fontStream.writeRaw(stream);
		this->_initializeFont();
	}

	FontTtf::FontTtf(hstream& stream, chstr name, float height, float scale,
		float lineHeight, float descender, bool loadBasicAscii) : atres::Font(name)
	{
		this->_setInternalValues("", name, height, scale, lineHeight, loadBasicAscii);
		this->descender = descender;
		this->customDescender = true;
		this->fontStream.writeRaw(stream);
		this->_initializeFont();
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
		this->customDescender = false;
		this->penX = 0;
		this->penY = 0;
		this->rowHeight = 0;
	}

	FontTtf::~FontTtf()
	{
		foreach (atres::TextureContainer*, it, this->textureContainers)
		{
			delete (*it)->texture;
			delete (*it);
		}
		this->textureContainers.clear();
		atresttf::destroyFace(this);
	}

	april::Texture* FontTtf::getTexture(unsigned int charCode)
	{
		this->_checkTextures();
		if (!this->_addCharacterBitmap(charCode))
		{
			return NULL;
		}
		return Font::getTexture(charCode);
	}

	bool FontTtf::hasChar(unsigned int charCode)
	{
		this->_addCharacterBitmap(charCode);
		return Font::hasChar(charCode);
	}

	void FontTtf::_checkTextures()
	{
		/* kspes@20150416 - commenting this out, not needed with new april managed textures, plus it caused a memory issue by deleting the texture
		   if this code is going to be restored, make sure you adjust all texture pointers in the cache to point to the new textures (or clear cache simply).
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
		 */
	}
	
	void FontTtf::_initializeFont()
	{
		if (this->fontStream.size() == 0)
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
				hlog::error(logTag, "Could not find: " + this->fontFilename);
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
			return;
		}
		if (error != 0)
		{
			hlog::error(logTag, "Could not read face 0 in: " + this->fontFilename + "; Error code: " + hstr(error));
			this->fontStream.clear();
			return;
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
			return;
		}
		this->internalDescender = -(float)PTSIZE2INT(face->size->metrics.descender);
		if (!this->customDescender)
		{
			this->descender = this->internalDescender;
		}
		atresttf::registerFace(this, face);
		this->_loadBasicCharacters();
		this->loaded = true;
	}

	april::Texture* FontTtf::_createTexture()
	{
		int textureSize = atresttf::getTextureSize();
		april::Texture* texture = NULL;
		if (atresttf::isAllowAlphaTextures() && april::rendersys->getCaps().textureFormats.has(april::Image::FORMAT_ALPHA))
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

	void FontTtf::_loadBasicCharacters()
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

	bool FontTtf::_addCharacterBitmap(unsigned int charCode, bool ignoreCharacterEnabled)
	{
		this->_checkTextures();
		if (this->characters.hasKey(charCode))
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
				hlog::debugf(logTag, "Character '0x%X' does not exist in: %s", charCode, this->fontFilename.cStr());
			}
			return false;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error != 0)
		{
			hlog::error(logTag, "Could not load glyph from: " + this->fontFilename);
			return false;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (error != 0)
			{
				hlog::error(logTag, "Could not render glyph from: " + this->fontFilename);
				return false;
			}
		}
		atres::TextureContainer* textureContainer = this->textureContainers.last();
		this->penX += hmax(face->glyph->bitmap_left, 0);
		// calculate some standard parameters
		int ascender = (int)(-PTSIZE2INT(face->size->metrics.ascender));
		int descender = (int)(-PTSIZE2INT(face->size->metrics.descender));
		// this makes sure that there is no vertical overlap between characters
		int lineOffset = (int)this->height - descender;
		int bearingY = -hmin(lineOffset - face->glyph->bitmap_top, 0);
		int offsetY = hmax(lineOffset - face->glyph->bitmap_top, 0);
		int charHeight = face->glyph->bitmap.rows + SAFE_SPACE * 2 + offsetY;
		int charWidth = face->glyph->bitmap.width + SAFE_SPACE * 2;
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
			hlog::debugf(logTag, "Font '%s': character 0x%X does not fit, creating new texture.", this->name.cStr(), charCode);
			textureContainer = new atres::TextureContainer();
			textureContainer->texture = this->_createTexture();
			this->textureContainers += textureContainer;
			this->penX = 0;
			this->penY = 0;
			// if the character's height is higher than the texture, this will obviously not work too well
		}
		if (face->glyph->bitmap.buffer != NULL)
		{
			textureContainer->texture->write(0, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows,
				this->penX + SAFE_SPACE, this->penY + offsetY + SAFE_SPACE, face->glyph->bitmap.buffer, face->glyph->bitmap.width,
				face->glyph->bitmap.rows, april::Image::FORMAT_ALPHA);
		}
		atres::CharacterDefinition c;
		c.x = (float)this->penX;
		c.y = (float)this->penY;
		c.w = (float)charWidth;
		c.h = (float)charHeight;
		c.bx = (float)PTSIZE2INT(face->glyph->metrics.horiBearingX);
		c.by = (float)(lineOffset + ascender + bearingY);
		c.aw = (float)PTSIZE2INT(face->glyph->advance.x);
		this->characters[charCode] = c;
		this->penX += charWidth + CHARACTER_SPACE * 2;
		textureContainer->characters += charCode;
		return true;
	}
	
}
