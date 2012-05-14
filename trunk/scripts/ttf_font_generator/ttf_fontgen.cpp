/// @file
/// @author  Boris Mikic
/// @version 2.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
#include <hltypes/hfile.h>
#include <hltypes/hdir.h>
#include <hltypes/hltypesUtil.h>
#include <april/main.h>
#include <april/april.h>
#include <april/Window.h>
#include <april/Texture.h>
#include <atres/atres.h>
#include <atresttf/atresttf.h>
#include <atresttf/FontResourceTtf.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

struct TgaHeader
{
	short w;
	short h;
	unsigned char bpp;
	unsigned char Flags;
};

void generate(chstr cfgname)
{
	// default parameters
	hstr fontFilename;
	hstr fontName;
	float height = 32.0f;
	float scale = 1.0f;
	float lineHeight = 0.0f;
	float correctedHeight = 0.0f;
	harray<unsigned int> rangeStarts;
	harray<unsigned int> rangeEnds;
	unsigned int rangeSegment;
	// read CFG file
	atres::log("- reading configuration file");
	if (!hfile::exists(cfgname))
	{
		atres::log("ERROR! Configuration file not found!");
		return;
	}
	harray<hstr> lines = hfile::hread(cfgname).split('\n');
	foreach (hstr, it, lines)
	{
		if ((*it).lower().starts_with("fontfile:"))
		{
			atres::log("    - found: " + (*it));
			fontFilename = (*it)(9, (*it).size() - 9);
		}
		else if ((*it).lower().starts_with("name:"))
		{
			atres::log("    - found: " + (*it));
			fontName = (*it)(5, (*it).size() - 5);
		}
		else if ((*it).lower().starts_with("height:"))
		{
			atres::log("    - found: " + (*it));
			height = (float)(*it)(7, (*it).size() - 7);
		}
		else if ((*it).lower().starts_with("scale:"))
		{
			atres::log("    - found: " + (*it));
			scale = (float)(*it)(6, (*it).size() - 6);
		}
		else if ((*it).lower().starts_with("lineheight:"))
		{
			atres::log("    - found: " + (*it));
			lineHeight = (float)(*it)(11, (*it).size() - 11);
		}
		else if ((*it).lower().starts_with("correctedheight:"))
		{
			atres::log("    - found: " + (*it));
			correctedHeight = (float)(*it)(16, (*it).size() - 16);
		}
		else if ((*it).lower().starts_with("coderanges:"))
		{
			atres::log("    - found: " + (*it));
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
	// create font
	atres::log("- creating font");
	atres::FontResource* font = new atresttf::FontResourceTtf(fontFilename, fontName, height, scale, lineHeight, correctedHeight);
	atres::renderer->registerFontResource(font);
	// add custom code ranges
	atres::log("- adding custom code ranges");
	for_iter (i, 0, rangeStarts.size())
	{
		atres::log(hsprintf("    - adding code range: %x-%x", rangeStarts[i], rangeEnds[i]));
		for_itert (unsigned int, code, rangeStarts[i], rangeEnds[i] + 1)
		{
			font->hasChar(code);
		}
	}
	// check for more than one texture
	harray<april::Texture*> textures = font->getTextures();
	bool multiTexture = (textures.size() > 1);
	// export definition
	atres::log("- exporting definition");
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
	if (correctedHeight != 0)
	{
		definition.write_line("CorrectedHeight=" + hsprintf("%g", correctedHeight));
	}
	definition.write_line("# -----------------------------------");
	definition.write_line("# Code|X|Y|Width");
	definition.write_line("# Code|X|Y|Width|Advance Width");
	definition.write_line("# Code|X|Y|Width|Advance Width|Bearing X");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Advance Width");
	definition.write_line("# Code|MultiTextureIndex|X|Y|Width|Advance Width|Bearing X");
	definition.write_line("-------------------------------------");
	hmap<unsigned int, atres::CharacterDefinition> characters = font->getCharacters();
	harray<unsigned int> codes = characters.keys().sorted();
	atres::CharacterDefinition c;
	if (!multiTexture)
	{
		foreach (unsigned int, it, codes)
		{
			c = characters[*it];
			definition.write_line(hsprintf("%d %g %g %g %g %g", (*it), c.x, c.y, c.w, c.aw, c.bx));
		}
	}
	else
	{
		int index;
		foreach (unsigned int, it, codes)
		{
			c = characters[*it];
			index = textures.index_of(font->getTexture(*it));
			definition.write_line(hsprintf("%d %d %g %g %g %g %g", (*it), index, c.x, c.y, c.w, c.aw, c.bx));
		}
	}
	definition.close();
	// export texture
	atres::log("- exporting textures");
	unsigned char preHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char* buffer;
	hfile tga;
	april::Texture* texture = NULL;
	for_iter (i, 0, textures.size())
	{
		texture = textures[i];
		if (!texture->copyPixelData(&buffer))
		{
			atres::log("ERROR! Could not fetch pixel data!");
			continue;
		}
		TgaHeader header;
		header.bpp = 32;
		header.w = texture->getWidth();
		header.h = texture->getHeight();
		header.Flags = 33;
		tga.open(fontName + (multiTexture ? "_" + hstr(i) : hstr("")) + ".tga", hfile::WRITE);
		tga.write_raw(preHeader, 12);
		tga.write_raw(&header, sizeof(header));
		tga.write_raw(buffer, header.w * header.h * 4);
		delete [] buffer;
		tga.close();
	}
}

void april_destroy()
{
	// destroy systems
	atresttf::destroy();
	atres::destroy();
	april::destroy();
}

bool update(float k)
{
	return 0;
}

void april_init(const harray<hstr>& argv)
{
#ifndef __APPLE__
	// check args
	if (argv.size() != 1 && argv.size() != 2)
	{
		printf("Wrong number of arguments supplied: [CONFIG_FILENAME]\n");
		return 1;
	}
#endif
	hstr cfgname = "ttf_fontgen.cfg";
	if (argv.size() >= 2)
	{
		cfgname = hstr(argv[1]);
		hdir::chdir(get_basedir(cfgname));
	}
	// init systems
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget(100, 100, false, "TTF Font Generator");
	atres::init();
	atresttf::init();
	// generate
	atres::log("- configuration file: " + cfgname);
	generate(cfgname);
	april::window->setUpdateCallback(update);
}
