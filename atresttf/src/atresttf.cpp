/// @file
/// @author  Boris Mikic
/// @version 3.01
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
	harray<hstr> fonts;
	harray<hstr> fontFiles;
	int textureSize = 1024;

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

#ifdef _WIN32
	int CALLBACK _fontEnumCallback(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
	{
		hstr fontName = unicode_to_utf8(lpelfe->elfLogFont.lfFaceName);
		hstr styleName = unicode_to_utf8(lpelfe->elfStyle);
		if (styleName != "" && styleName != "Regular")
		{
			fontName += " " + styleName;
		}
		fonts += fontName;
		return 1;
	}
#endif

	harray<hstr> getSystemFonts()
	{
		if (fonts.size() > 0)
		{
			return fonts;
		}
#ifdef _WIN32
		LOGFONT logFont;
		logFont.lfFaceName[0] = NULL;
		logFont.lfCharSet = DEFAULT_CHARSET;
		HWND hWnd = (HWND)april::window->getBackendId();
		HDC hDC = GetDC(hWnd);
		EnumFontFamiliesEx(hDC, &logFont, (FONTENUMPROC)&_fontEnumCallback, 0, 0);
		ReleaseDC(hWnd, hDC);
#elif defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
#elif defined(__APPLE__) && !TARGET_OS_MAC && TARGET_OS_IPHONE
#elif defined(__UNIX__)
#endif
		fonts.remove_duplicates();
		fonts.sort();
		return fonts;
	}

	hstr findSystemFontFilename(chstr name)
	{
		if (fontFiles.size() == 0)
		{
			fontFiles = hdir::files(atresttf::getSystemFontsPath(), true);
		}
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
				if (name == fontName)
				{
					return (*it);
				}
			}
		}
		return "";
	}

	hstr getSystemFontsPath()
	{
#ifdef _WIN32
		return (normalize_path(get_environment_variable("WinDir")) + "/Fonts/");
#elif defined(__APPLE__)
		return "/Library/Fonts/";
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
