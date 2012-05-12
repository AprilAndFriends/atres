/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <april/Window.h>
#include <atres/atres.h>
#include <atres/FontResource.h>
#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atresttf.h"
#include "freetype.h"

namespace atresttf
{
	FT_Library library = NULL;
	hmap<atres::FontResource*, FT_Face> faces;
	harray<hstr> fonts;
	harray<hstr> fontFiles;

	void log(chstr message)
	{
		atres::log(message, "[atresttf] ");
	}

    void init()
    {
		atresttf::log("initializing atresttf");
		FT_Error error = FT_Init_FreeType(&library);
		if (error != 0)
		{
			atresttf::log("Error while initializing atresttf!");
		}
	}
    
    void destroy()
    {
		atresttf::log("destroying atresttf");
		foreach_map (atres::FontResource*, FT_Face, it, faces)
		{
			FT_Done_Face(it->second);
		}
		FT_Error error = FT_Done_FreeType(library);
		if (error != 0)
		{
			atresttf::log("Error while destroying atresttf!");
		}
    }

#ifdef _WIN32
	int CALLBACK _fontEnumCallback(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
	{
		fonts += unicode_to_utf8(lpelfe->elfLogFont.lfFaceName);
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
		HWND hWnd = (HWND)april::window->getIDFromBackend();
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
		foreach (hstr, it, fontFiles)
		{
			error = FT_New_Face(library, (*it).c_str(), 0, &face);
			if (error == 0)
			{
				if (name == (char*)face->family_name)
				{
					FT_Done_Face(face);
					return (*it);
				}
				FT_Done_Face(face);
			}
		}
		return "";
	}

	hstr getSystemFontsPath()
	{
#ifdef _WIN32
		return (normalize_path(hstr(getenv("WinDir"))) + "/Fonts/");
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
			hl_exception("Error: atresttf not initialized!");
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
			atresttf::log("Cannot add Face for Font Resource " + fontResource->getName());
			return;
		}
		faces[fontResource] = face;
	}

	void removeFace(atres::FontResource* fontResource, FT_Face face)
	{
		if (!faces.has_key(fontResource))
		{
			atresttf::log("Cannot remove Face for Font Resource " + fontResource->getName());
			return;
		}
		faces.remove_key(fontResource);
	}

}
