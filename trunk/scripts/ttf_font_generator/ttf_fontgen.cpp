/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/main.h>
#include <april/april.h>
#include <april/Window.h>
#include <april/Texture.h>
#include <atres/atres.h>
#include <atresttf/atresttf.h>
#include <atresttf/FontResourceTtf.h>
#include <hltypes/hfile.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define LOG_TAG "ttf_fontgen"

struct TgaHeader
{
    short width;
    short height;
    unsigned char bits;
    unsigned char descriptor;
};

void generate(chstr cfgname)
{
	// default parameters
	hstr fontFilename;
	hstr fontName;
	float height = 32.0f;
	float scale = 1.0f;
	float lineHeight = 0.0f;
	float descender = 0.0f;
	bool customDescender = false;
	int textureSize = 1024;
	harray<unsigned int> rangeStarts;
	harray<unsigned int> rangeEnds;
	unsigned int rangeSegment;
	// read CFG file
	hlog::write(LOG_TAG, "- reading configuration file");
	if (!hfile::exists(cfgname))
	{
		hlog::error(LOG_TAG, "Configuration file not found!");
		return;
	}
	harray<hstr> lines = hfile::hread(cfgname).split('\n');
	foreach (hstr, it, lines)
	{
		if ((*it).lower().starts_with("fontfile:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			fontFilename = (*it)(9, (*it).size() - 9);
		}
		else if ((*it).lower().starts_with("name:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			fontName = (*it)(5, (*it).size() - 5);
		}
		else if ((*it).lower().starts_with("height:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			height = (float)(*it)(7, (*it).size() - 7);
		}
		else if ((*it).lower().starts_with("scale:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			scale = (float)(*it)(6, (*it).size() - 6);
		}
		else if ((*it).lower().starts_with("lineheight:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			lineHeight = (float)(*it)(11, (*it).size() - 11);
		}
		else if ((*it).lower().starts_with("descender:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			descender = (float)(*it)(10, (*it).size() - 10);
			customDescender = true;
		}
		else if ((*it).lower().starts_with("texturesize:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			textureSize = (int)(*it)(12, (*it).size() - 12);
		}
		else if ((*it).lower().starts_with("coderanges:"))
		{
			hlog::write(LOG_TAG, "    - found: " + (*it));
			hstr rawRanges = (*it)(11, (*it).size() - 11);
			harray<hstr> ranges = rawRanges.split(",", -1, true);
			harray<hstr> range;
			foreach (hstr, it2, ranges)
			{
				range = (*it2).split("-", -1, true);
				if (range.size() == 2)
				{
					sscanf(range[0].c_str(), "%x", &rangeSegment);
					rangeStarts += rangeSegment;
					sscanf(range[1].c_str(), "%x", &rangeSegment);
					rangeEnds += rangeSegment;
				}
			}
		}
	}
	if (rangeStarts.size() == 0)
	{
		rangeStarts += 0x1F;
		rangeEnds += 0x7F;
	}
	atresttf::setTextureSize(textureSize);
	// create font
	hlog::write(LOG_TAG, "- creating font");
	atres::FontResource* font;
	if (!customDescender)
	{
		font = new atresttf::FontResourceTtf(fontFilename, fontName, height, 1.0f, lineHeight, false);
	}
	else
	{
		font = new atresttf::FontResourceTtf(fontFilename, fontName, height, 1.0f, lineHeight, descender, false);
	}
	if (!font->isLoaded())
	{
		delete font;
		return;
	}
	atres::renderer->registerFontResource(font);
	// add custom code ranges
	hlog::write(LOG_TAG, "- adding custom code ranges");
	for_iter (i, 0, rangeStarts.size())
	{
		hlog::write(LOG_TAG, hsprintf("    - adding code range: %X-%X", rangeStarts[i], rangeEnds[i]));
		for_itert (unsigned int, code, rangeStarts[i], rangeEnds[i] + 1)
		{
			font->hasChar(code);
		}
	}
	// check for more than one texture
	harray<april::Texture*> textures = font->getTextures();
	bool multiTexture = (textures.size() > 1);
	// export definition
	hlog::write(LOG_TAG, "- exporting definition");
	hfile definition(fontName + ".font", hfile::WRITE);
	definition.write_line("# -----------------------------------");
	definition.write_line("# ATRES Font definition file");
	definition.write_line("# -----------------------------------");
	definition.write_line("Name=" + fontName);
	if (!multiTexture)
	{
		definition.write_line("Texture=" + fontName);
	}
	else
	{
		harray<hstr> textureNames;
		for_iter (i, 0, textures.size())
		{
			textureNames += fontName + "_" + hstr(i);
		}
		definition.write_line("MultiTexture=" + textureNames.join("\t"));
	}
	definition.write_line("Height=" + hsprintf("%g", height));
	definition.write_line("Scale=" + hsprintf("%g", scale));
	if (lineHeight != 0)
	{
		definition.write_line("LineHeight=" + hsprintf("%g", lineHeight));
	}
	definition.write_line("Descender=" + hsprintf("%g", font->getDescender()));
	definition.write_line("# -------------------------------------------------------------------------");
	definition.write_line("# Code|X|Y|Width");
	definition.write_line("# Code|X|Y|Width|Advance Width");
	definition.write_line("# Code|X|Y|Width|Advance Width|Bearing X");
	definition.write_line("# Code|X|Y|Width|Height|Advance Width|Bearing X");
	definition.write_line("# Code|X|Y|Width|Height|Advance Width|Bearing X|Bearing Y");
	definition.write_line("# -------------------------------------------------------------------------");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Advance Width");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Advance Width|Bearing X");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Height|Advance Width|Bearing X");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Height|Advance Width|Bearing X|Bearing Y");
	definition.write_line("# -------------------------------------------------------------------------");
	hmap<unsigned int, atres::CharacterDefinition> characters = font->getCharacters();
	harray<unsigned int> codes = characters.keys().sorted();
	atres::CharacterDefinition c;
	if (!multiTexture)
	{
		foreach (unsigned int, it, codes)
		{
			c = characters[*it];
			definition.write_line(hsprintf("%d %g %g %g %g %g %g %g", (*it), c.x, c.y, c.w, c.h, c.aw, c.bx, c.by));
		}
	}
	else
	{
		int index = 0;
		foreach (unsigned int, it, codes)
		{
			c = characters[*it];
			index = textures.index_of(font->getTexture(*it));
			definition.write_line(hsprintf("%d %d %g %g %g %g %g %g %g", (*it), index, c.x, c.y, c.w, c.h, c.aw, c.bx, c.by));
		}
	}
	definition.close();
	// export texture
	hlog::write(LOG_TAG, "- exporting textures");
	unsigned char preHeader[12] = {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char* buffer = NULL;
	hfile tga;
	april::Texture* texture = NULL;
	TgaHeader header;
	header.width = 0;
	header.height = 0;
	header.bits = 8;
	header.descriptor = 32;
	for_iter (i, 0, textures.size())
	{
		texture = textures[i];
		if (!texture->copyPixelData(&buffer, april::Image::FORMAT_RGBA))
		{
			hlog::error(LOG_TAG, "Could not fetch pixel data!");
			continue;
		}
		header.width = texture->getWidth();
		header.height = texture->getHeight();
		tga.open(fontName + (multiTexture ? "_" + hstr(i) : hstr("")) + ".tga", hfile::WRITE);
		tga.write_raw(preHeader, 12);
		tga.write_raw(&header, sizeof(header));
		tga.write_raw(buffer, header.width * header.height);
		tga.close();
		delete [] buffer;
	}
	hlog::write(LOG_TAG, "- finished");
}

void april_init(const harray<hstr>& argv)
{
#ifndef __APPLE__
	// check args
	if (argv.size() != 1 && argv.size() != 2)
	{
		hlog::error(LOG_TAG, "Wrong number of arguments supplied: [CONFIG_FILENAME]");
		return;
	}
#endif
	hstr cfgname = "ttf_fontgen.cfg";
	if (argv.size() >= 2)
	{
		cfgname = hstr(argv[1]);
		hdir::chdir(hdir::basedir(cfgname));
	}
	// init systems
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow(100, 100, false, "TTF Font Generator");
	atres::init();
	atresttf::init();
	// generate
	hlog::write(LOG_TAG, "- configuration file: " + cfgname);
	generate(cfgname);
}

void april_destroy()
{
	atresttf::destroy();
	atres::destroy();
	april::destroy();
}

