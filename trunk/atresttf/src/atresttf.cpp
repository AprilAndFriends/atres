/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <april/Window.h>
#include <atres/atres.h>
#include <atres/FontResource.h>
#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "freetype.h"

namespace atresttf
{
	hstr logTag = "atresttf";

	FT_Library library = NULL;
	hmap<atres::FontResource*, FT_Face> faces;
	hmap<hstr, hstr> fonts;
	bool fontNamesChecked = false;
	int textureSize = 1024;
	bool allowAlphaTextures = true;
	
	void init()
	{
		hlog::write(atresttf::logTag, "Initializing AtresTTF");
		FT_Error error = FT_Init_FreeType(&library);
		if (error != 0)
		{
			hlog::error(atresttf::logTag, "Could not initialize FreeType library!");
		}
	}
	
	void destroy()
	{
		hlog::write(atresttf::logTag, "Destroying AtresTTF");
		foreach_map (atres::FontResource*, FT_Face, it, faces)
		{
			FT_Done_Face(it->second);
		}
		faces.clear();
		FT_Error error = FT_Done_FreeType(library);
		if (error != 0)
		{
			hlog::error(atresttf::logTag, "Could not finalize FreeType library!");
		}
	}

	int getTextureSize()
	{
		return textureSize;
	}

	void setTextureSize(int value)
	{
		textureSize = value;
	}

	bool isAllowAlphaTextures()
	{
		return allowAlphaTextures;
	}

	void setAllowAlphaTextures(bool value)
	{
		allowAlphaTextures = value;
	}

	harray<hstr> getSystemFonts()
	{
		if (!fontNamesChecked)
		{
			harray<hstr> fontFiles = hdir::files(atresttf::getSystemFontsPath(), true);
			FT_Library library = atresttf::getLibrary();
			FT_Face face;
			FT_Error error;
			hstr fontName;
			hstr styleName;
			foreach (hstr, it, fontFiles)
			{
				error = FT_New_Face(library, (*it).c_str(), 0, &face);
				if (error == 0)
				{
					fontName = hstr((char*)face->family_name);
					styleName = hstr((char*)face->style_name);
					FT_Done_Face(face);
					if (styleName != "" && styleName != "Regular")
					{
						fontName += " " + styleName;
					}
					fonts[fontName] = (*it);
				}
			}
			fontNamesChecked = true;
		}
		return fonts.keys().sorted();
	}

	hstr findSystemFontFilename(chstr name)
	{
		if (!fontNamesChecked)
		{
			getSystemFonts();
		}
		return fonts.try_get_by_key(name, "");
	}

	hstr getSystemFontsPath()
	{
#ifdef _WIN32
#ifndef _WINRT
		return (hdir::normalize(get_environment_variable("WinDir")) + "/Fonts/");
#else
		return "";
#endif
#elif defined(__APPLE__)
#ifdef _IOS
		return "/System/Library/Fonts/";
#else
		return "/Library/Fonts/";
#endif
#elif defined(_ANDROID)
		return "/system/fonts/";
#elif defined(_UNIX)
		return "/usr/share/fonts/";
#else
		return "";
#endif
	}

	FT_Library getLibrary()
	{
		if (library == NULL)
		{
			throw hl_exception("AtresTTF not initialized!");
		}
		return library;
	}

	FT_Face getFace(atres::FontResource* fontResource)
	{
		return faces[fontResource];
	}

	void addFace(atres::FontResource* fontResource, FT_Face face)
	{
		if (faces.has_key(fontResource))
		{
			hlog::error(atresttf::logTag, "Cannot add Face for Font Resource: " + fontResource->getName());
			return;
		}
		faces[fontResource] = face;
	}

	void removeFace(atres::FontResource* fontResource, FT_Face face)
	{
		if (!faces.has_key(fontResource))
		{
			hlog::error(atresttf::logTag, "Cannot remove Face for Font Resource: " + fontResource->getName());
			return;
		}
		faces.remove_key(fontResource);
	}

}
