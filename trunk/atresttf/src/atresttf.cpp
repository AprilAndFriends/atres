/// @file
/// @version 3.43
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <april/Window.h>
#include <atres/atres.h>
#include <atres/Font.h>
#include <hltypes/harray.h>
#include <hltypes/hdir.h>
#include <hltypes/hexception.h>
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
	hmap<atres::Font*, FT_Face> faces;
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
		foreach_map (atres::Font*, FT_Face, it, faces)
		{
			FT_Done_Face(it->second);
		}
		faces.clear();
		FT_Error error = FT_Done_FreeType(library);
		if (error == 0)
		{
			library = NULL;
		}
		else
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
#if defined(_WIN32) && !defined(_WINRT)
			HKEY hKey = NULL;
			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				wchar_t wName[MAX_PATH] = {0};
				wchar_t wData[MAX_PATH] = {0};
				DWORD wSize = sizeof(wName) - 1;
				DWORD index = 0;
				int slash = 0;
				hstr name;
				hstr data;
				hstr systemPath = atresttf::getSystemFontsPath();
				while (RegEnumValueW(hKey, index, wName, &wSize, NULL, NULL, (BYTE*)wData, &wSize) == ERROR_SUCCESS)
				{
					name = hstr::from_unicode(wName);
					data = hstr::from_unicode(wData);
					if (name.ends_with(")"))
					{
						slash = name.rfind('(');
						if (slash >= 0)
						{
							name = name(0, slash).rtrim();
						}
					}
					fonts[name] = hdir::joinPath(systemPath, data);
					memset(wName, 0, sizeof(wName));
					memset(wData, 0, sizeof(wData));
					wSize = sizeof(wName) - 1;
					++index;
				}
				RegCloseKey(hKey);
			}
			else
			{
				hlog::error(atresttf::logTag, "Could not open registry!");
			}
#else
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
#endif
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
		return (hdir::normalize(henv("WinDir")) + "/Fonts/");
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
			throw Exception("AtresTTF not initialized!");
		}
		return library;
	}

	FT_Face getFace(atres::Font* font)
	{
		return faces[font];
	}

	void registerFace(atres::Font* font, FT_Face face)
	{
		if (faces.has_key(font))
		{
			hlog::error(atresttf::logTag, "Cannot add Face for Font Resource: " + font->getName());
			return;
		}
		faces[font] = face;
	}

	void destroyFace(atres::Font* font)
	{
		if (!faces.has_key(font))
		{
			hlog::warn(atresttf::logTag, "No Face registered for Font: " + font->getName());
			return;
		}
		FT_Done_Face(faces[font]);
		faces.remove_key(font);
	}

}
