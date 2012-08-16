/// @file
/// @author  Boris Mikic
/// @version 2.71
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "FontResourceTtf.h"
#include "freetype.h"

#define SAFE_SPACE 1
#define CHARACTER_SPACE 0

#define PTSIZE2INT(value) (((value) + 63) >> 6)

namespace atresttf
{
	extern void log(chstr message);

	FontResourceTtf::FontResourceTtf(chstr filename) : atres::FontResource(filename), fontFile(NULL)
	{
		hstr path = get_basedir(filename) + "/";
		harray<hstr> lines = hresource::hread(filename).split("\n", -1, true);
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_first();
			if (!this->_readBasicParameter(line))
			{
				if (line.starts_with("File="))
				{
					this->fontFilename = path + line.replace("File=", "");
				}
			}
		}
		this->_initializeFont();
	}

	FontResourceTtf::FontResourceTtf(chstr fontFilename, chstr name, float height, float scale,
		float lineHeight, float correctedHeight) : atres::FontResource(name), fontFile(NULL)
	{
		this->fontFilename = fontFilename;
		this->name = name;
		this->baseScale = scale;
		this->scale = scale;
		this->height = height;
		this->lineHeight = lineHeight;
		this->correctedHeight = correctedHeight;
		this->_initializeFont();
	}

	FontResourceTtf::~FontResourceTtf()
	{
		if (this->fontFile != NULL)
		{
			delete [] this->fontFile;
		}
	}

	april::Texture* FontResourceTtf::getTexture(unsigned int charcode)
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
		if (!this->_addCharacterBitmap(charcode))
		{
			return NULL;
		}
		return FontResource::getTexture(charcode);
	}

	bool FontResourceTtf::hasChar(unsigned int charcode)
	{
		this->_addCharacterBitmap(charcode);
		return FontResource::hasChar(charcode);
	}
	
	void FontResourceTtf::_initializeFont()
	{
		if (this->fontFilename == "")
		{
			this->fontFilename = atresttf::findSystemFontFilename(this->name);
		}
		if (this->fontFilename == "") // no font file
		{
			return;
		}
		if (!hresource::exists(this->fontFilename)) // font file does not exist
		{
			atresttf::log("Error: could not find " + this->fontFilename);
			return;
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		if (this->correctedHeight == 0.0f)
		{
			this->correctedHeight = this->height;
		}
		// libfreetype stuff
		FT_Library library = atresttf::getLibrary();
		FT_Face face = NULL;
		hresource file(this->fontFilename);
		if (this->fontFile != NULL) // making sure there are no memory leaks whatsoever
		{
			delete [] this->fontFile;
		}
		int size = file.size();
		this->fontFile = new unsigned char[size];
		file.read_raw(this->fontFile, size);
		file.close();
		FT_Error error = FT_New_Memory_Face(library, this->fontFile, size, 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			atresttf::log("Error: Format not supported in " + this->fontFilename);
			delete [] this->fontFile;
			this->fontFile = NULL;
			return;
		}
		if (error != 0)
		{
			atresttf::log("Error: Could not read face 0 in " + this->fontFilename + "; Error code: " + hstr(error));
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
			atresttf::log("Error: Could not set font size in " + this->fontFilename);
			delete [] this->fontFile;
			this->fontFile = NULL;
			FT_Done_Face(face);
			return;
		}
		atresttf::addFace(this, face);
		this->_loadBasicCharacters();
		this->loaded = true;
	}

	april::Texture* FontResourceTtf::_createTexture()
	{
		int textureSize = atresttf::getTextureSize();
		april::Texture* texture = april::rendersys->createTexture(textureSize, textureSize, april::Texture::FORMAT_ALPHA);
		if (!texture->isLoaded())
		{
			delete texture;
			atresttf::log("trying april::Texture::FORMAT_ARGB");
			texture = april::rendersys->createTexture(textureSize, textureSize, april::Texture::FORMAT_ARGB);
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
		for_itert (unsigned int, code, 32, 128)
		{
			this->_addCharacterBitmap(code, true);
		}
	}

	bool FontResourceTtf::_addCharacterBitmap(unsigned int charcode, bool ignoreCharacterEnabled)
	{
		if (this->characters.has_key(charcode))
		{
			return true;
		}
		FT_Face face = atresttf::getFace(this);
		unsigned int glyphIndex = FT_Get_Char_Index(face, (unsigned long)charcode);
		if (glyphIndex == 0)
		{
#ifdef _DEBUG
			if (!ignoreCharacterEnabled && charcode >= 32)
			{
				atresttf::log(hsprintf("Error: Character '0x%X' does not exist in %s", charcode, this->fontFilename.c_str()));
			}
#endif
			return false;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error != 0)
		{
			atresttf::log("Error: Could not load glyph from " + this->fontFilename);
			return false;
		}
		FT_GlyphSlot glyph = face->glyph;
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (error != 0)
			{
				atresttf::log("Error: Could not render glyph from " + this->fontFilename);
				return false;
			}
		}
		atres::TextureContainer* textureContainer = this->textureContainers.last();
		// calculate some standard parameters
		int textureSize = atresttf::getTextureSize();
		int ascender = -PTSIZE2INT(face->size->metrics.ascender);
		int descender = -PTSIZE2INT(face->size->metrics.descender);
		// this makes sure that there is no vertical overlap between characters
		int height = descender - ascender;
		int renderOffset = height - glyph->bitmap_top;
		int charHeight = renderOffset + glyph->bitmap.rows + SAFE_SPACE * 2;
		int charWidth = glyph->bitmap.width + SAFE_SPACE * 2;
		this->rowHeight = hmax(this->rowHeight, charHeight);
		// if character bitmap width exceeds space, go into next line
		if (this->penX + glyph->bitmap.width + CHARACTER_SPACE > textureSize)
		{
			this->penX = 0;
			this->penY += this->rowHeight + CHARACTER_SPACE * 2;
			this->rowHeight = charHeight;
		}
		if (this->penY + this->rowHeight + CHARACTER_SPACE > textureSize)
		{
#ifdef _DEBUG
			atresttf::log(hsprintf("Font '%s': character 0x%X does not fit, creating new texture", this->name.c_str(), charcode));
#endif
			textureContainer = new atres::TextureContainer();
			textureContainer->texture = this->_createTexture();
			this->textureContainers += textureContainer;
			this->penX = 0;
			this->penY = 0;
			// if the character's height is higher than the texture, this will obviously not work too well
		}
		if (textureContainer->texture->getFormat() == april::Texture::FORMAT_ALPHA)
		{
			textureContainer->texture->blit(this->penX + SAFE_SPACE, this->penY + renderOffset + SAFE_SPACE, glyph->bitmap.buffer,
				glyph->bitmap.width, glyph->bitmap.rows, 1, 0, 0, glyph->bitmap.width, glyph->bitmap.rows);
		}
		else
		{
			int size = glyph->bitmap.width * glyph->bitmap.rows * 4;
			unsigned char* glyphData = new unsigned char[size];
			memset(glyphData, 255, size * sizeof(unsigned char));
			int offset;
			for_iter (j, 0, glyph->bitmap.rows)
			{
				for_iter (i, 0, glyph->bitmap.width)
				{
					offset = i + j * glyph->bitmap.width;
					glyphData[offset * 4 + 3] = glyph->bitmap.buffer[offset];
				}
			}
			textureContainer->texture->blit(this->penX + SAFE_SPACE, this->penY + renderOffset + SAFE_SPACE, glyphData,
				glyph->bitmap.width, glyph->bitmap.rows, 4, 0, 0, glyph->bitmap.width, glyph->bitmap.rows);
			delete glyphData;
		}
		atres::CharacterDefinition c;
		c.x = (float)this->penX;
		c.y = (float)(this->penY + descender);
		c.w = (float)charWidth;
		c.h = (float)(charHeight - descender);
		c.bx = (float)PTSIZE2INT(glyph->metrics.horiBearingX);
		c.aw = (float)PTSIZE2INT(glyph->advance.x);
		this->characters[charcode] = c;
		this->penX += charWidth + CHARACTER_SPACE * 2;
		textureContainer->characters += charcode;
		return true;
	}
	
}
