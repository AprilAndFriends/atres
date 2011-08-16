/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <hltypes/hstring.h>
#include <hltypes/hfile.h>

#include "freetype.h"
#include "FontResourceTtf.h"

#define TEXTURE_SIZE 1024
#define CHARACTER_SPACE 4

#define PTSIZE2INT(value) (((value) + 63) >> 6)

namespace atresttf
{
	FontResourceTtf::FontResourceTtf(chstr filename) : atres::FontResource(filename)
	{
		hstr path = get_basedir(filename) + "/";
		harray<hstr> lines = hfile::hread(filename).split("\n");
		hstr line;
		while (lines.size() > 0)
		{
			line = lines.pop_front();
			if (line.starts_with("Name="))
			{
				this->name = line.replace("Name=", "");
			}
			else if (line.starts_with("File="))
			{
				this->fontFilename = path + line.replace("File=", "");
			}
			else if (line.starts_with("LineHeight="))
			{
				this->lineHeight = (float)line.replace("LineHeight=", "");
			}
			else if (line.starts_with("Height="))
			{
				this->height = (float)line.replace("Height=", "");
			}
			else if (line.starts_with("Scale="))
			{
				this->scale = (float)line.replace("Scale=", "");
				this->baseScale = this->scale;
			}
		}
		this->_initializeFont();
	}

	FontResourceTtf::FontResourceTtf(chstr fontFilename, chstr name, float height, float scale, float lineHeight) : atres::FontResource(name)
	{
		this->fontFilename = fontFilename;
		// TODO - should check system fonts if file does not exit!
		this->height = height;
		this->scale = scale;
		this->baseScale = scale;
		this->lineHeight = lineHeight;
		this->_initializeFont();
	}

	FontResourceTtf::~FontResourceTtf()
	{
		foreach (TextureContainer*, it, this->textureContainers)
		{
			delete (*it)->texture;
			delete (*it);
		}
		this->textureContainers.clear();
	}
	
	april::Texture* FontResourceTtf::getTexture(unsigned int charcode)
	{
		if (!this->_addCharacterBitmap(charcode))
		{
			return NULL;
		}
		foreach (TextureContainer*, it, this->textureContainers)
		{
			if ((*it)->characters.contains(charcode))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	bool FontResourceTtf::hasChar(unsigned int charcode)
	{
		this->_addCharacterBitmap(charcode);
		return FontResource::hasChar(charcode);
	}
	
	void FontResourceTtf::_initializeFont()
	{
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		// libfreetype stuff
		FT_Library library = atresttf::getLibrary();
		FT_Face face;
		FT_Error error = FT_New_Face(library, this->fontFilename.c_str(), 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			atres::log("Error: Format not supported in " + this->fontFilename);
			return;
		}
		if (error != 0)
		{
			atres::log("Error: Could not read face 0 in " + this->fontFilename + hstr(error));
			return;
		}
		FT_Size_RequestRec request;
		memset(&request, 0, sizeof(FT_Size_RequestRec));
		request.height = (long)hround((double)this->height) << 6;
		request.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
		error = FT_Request_Size(face, &request);
		if (error != 0)
		{
			atres::log("Error: Could not set font size in " + this->fontFilename);
			return;
		}
		atresttf::setFace(this, face);
		TextureContainer* textureContainer = new TextureContainer();
		textureContainer->texture = april::rendersys->createBlankTexture(TEXTURE_SIZE, TEXTURE_SIZE, april::AT_ARGB);
		this->textureContainers += textureContainer;
		for (unsigned int code = 32; code < 256; code++)
		{
			this->_addCharacterBitmap(code);
		}
	}

	bool FontResourceTtf::_addCharacterBitmap(unsigned int charcode)
	{
		if (this->characters.has_key(charcode))
		{
			return true;
		}
		FT_Face face = atresttf::getFace(this);
		unsigned int glyphIndex = FT_Get_Char_Index(face, (unsigned long)charcode);
		if (glyphIndex == 0)
		{
			return false;
		}
		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER);
		if (error != 0)
		{
			atres::log("Error: Could not load glyph from " + this->fontFilename);
			return false;
		}
		FT_GlyphSlot glyph = face->glyph;
		int size = glyph->bitmap.rows * glyph->bitmap.width * 4;
		unsigned char* data = new unsigned char[size];
		memset(data, 255, size * sizeof(unsigned char));
		int index;
		for (int j = 0; j < glyph->bitmap.rows; j++)
		{
			for (int i = 0; i < glyph->bitmap.width; i++)
			{
				index = i + j * glyph->bitmap.width;
				data[index * 4 + 3] = glyph->bitmap.buffer[index];
			}
		}
		TextureContainer* textureContainer = this->textureContainers.back();
		int maxHeight = PTSIZE2INT(face->size->metrics.height) + CHARACTER_SPACE * 2;
		if (textureContainer->penX + glyph->bitmap.width + 4 > TEXTURE_SIZE)
		{
			textureContainer->penX = CHARACTER_SPACE;
			textureContainer->penY += maxHeight;
		}
		if (textureContainer->penY + maxHeight > TEXTURE_SIZE)
		{
			textureContainer = new TextureContainer();
			textureContainer->texture = april::rendersys->createBlankTexture(TEXTURE_SIZE, TEXTURE_SIZE, april::AT_ARGB);
			this->textureContainers += textureContainer;
		}
		int ascender = PTSIZE2INT(face->size->metrics.ascender);
		int x = textureContainer->penX;
		int y = textureContainer->penY + ascender - glyph->bitmap_top;
		textureContainer->texture->blit(x, y, data, glyph->bitmap.width,
			glyph->bitmap.rows, 4, 0, 0, glyph->bitmap.width, glyph->bitmap.rows);
		atres::CharacterDefinition c;
		c.x = (float)x;
		c.y = (float)textureContainer->penY;
		c.w = (float)glyph->bitmap.width;
		c.h = (float)(ascender - PTSIZE2INT(face->size->metrics.descender));
		c.bx = (float)PTSIZE2INT(glyph->metrics.horiBearingX);
		c.aw = (float)PTSIZE2INT(glyph->advance.x);
		this->characters[charcode] = c;
		textureContainer->penX += glyph->bitmap.width + CHARACTER_SPACE * 2;
		textureContainer->characters += charcode;
		return true;
	}
	
}
