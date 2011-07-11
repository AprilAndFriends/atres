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

#include <april/Texture.h>
#include <hltypes/hstring.h>
#include <hltypes/hfile.h>

#include "freetype.h"
#include "FontResourceTtf.h"

namespace atresttf
{
	FontResourceTtf::FontResourceTtf(chstr filename) : atres::FontResource(filename)
	{
		this->scale = 1.0f;
		this->baseScale = 1.0f;
		this->lineHeight = 0.0f;
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
				this->fontFilename = line.replace("File=", "");
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
			else if (line.starts_with("#"))
			{
				continue;
			}
			else if (line.starts_with("-"))
			{
				break;
			}
		}
		if (this->lineHeight == 0.0f)
		{
			this->lineHeight = this->height;
		}
		// libfreetype stuff
		FT_Library library = atresttf::getLibrary();
		FT_Error error = FT_New_Face(library, this->fontFilename.c_str(), 0, &this->face);
		if (error == FT_Err_Unknown_File_Format)
		{
			atres::log("Error: Format not supported in " + this->fontFilename);
			return;
		}
		if (error != 0)
		{
			atres::log("Error: Could not read face 0 in " + this->fontFilename);
			return;
		}
		error = FT_Set_Pixel_Sizes(face, 0, (unsigned int)this->height);
		if (error != 0)
		{
			atres::log("Error: Could not set char size");
			return;
		}

		

		unsigned int glyphIndex;
		for (unsigned int code = 32; code < 256; code++)
		{
			glyphIndex = FT_Get_Char_Index(this->face, (unsigned long)code);
			if (glyphIndex != 0)
			{
				this->_addCharacterBitmap(code);
			}

			
		}

		/*
		unsigned int code;
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			c.aw = 0.0f;
			data = (*it).split(" ");
			if (data.size() == 5)
			{
				code = (unsigned int)data.pop_front();
				c.x = (float)data.pop_front();
				c.y = (float)data.pop_front();
				c.w = (float)data.pop_front();
				c.aw = (float)data.pop_front();
				if (c.aw == 0.0f)
				{
					c.aw = c.w;
				}
				this->characters[code] = c;
			}
		}
		*/
	}

	FontResourceTtf::~FontResourceTtf()
	{
	}
	
	april::Texture* FontResourceTtf::getTexture(unsigned int charcode)
	{
		return this->textureContainers[0].texture;
	}

	bool FontResourceTtf::_addCharacterBitmap(unsigned int charcode)
	{
		if (this->characters.has_key(charcode))
		{
			return true;
		}
		unsigned int glyphIndex = FT_Get_Char_Index(this->face, (unsigned long)charcode);
		if (glyphIndex == 0)
		{
			return false;
		}
		// TODO
		FT_Error error = FT_Load_Glyph(this->face, glyphIndex, FT_LOAD_RENDER);
		//this->face->glyph->
		//atres::CharacterDefinition c;
		return false;
	}
	
}
