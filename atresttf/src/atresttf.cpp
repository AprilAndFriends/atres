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

#if _HL_WINRT && !defined(_NO_SYSTEM_FONTS)
#include <dwrite.h>
using namespace Microsoft::WRL;
#endif

namespace atresttf
{
	hstr logTag = "atresttf";

	FT_Library library = NULL;
	hmap<atres::FontResource*, FT_Face> faces;
	harray<hstr> fonts;
	harray<hstr> fontFiles;
	bool fontsChecked = false;
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

#if defined(_WIN32) && !_HL_WINRT && !defined(_NO_SYSTEM_FONTS)
	int CALLBACK _fontEnumCallback(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
	{
		hstr fontName = hstr::from_unicode(lpelfe->elfLogFont.lfFaceName);
		hstr styleName = hstr::from_unicode(lpelfe->elfStyle);
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
#ifndef _NO_SYSTEM_FONTS
		if (fontsChecked)
		{
			return fonts;
		}
#ifdef _WIN32
#if !_HL_WINRT
		LOGFONT logFont;
		logFont.lfFaceName[0] = NULL;
		logFont.lfCharSet = DEFAULT_CHARSET;
		HWND hWnd = (HWND)april::window->getBackendId();
		HDC hDC = GetDC(hWnd);
		EnumFontFamiliesEx(hDC, &logFont, (FONTENUMPROC)&_fontEnumCallback, 0, 0);
		ReleaseDC(hWnd, hDC);
#else
		IDWriteFactory* dWriteFactory = NULL;
		HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dWriteFactory));
		if (FAILED(hr))
		{
			return fonts;
		}
		IDWriteFontCollection* fontCollection = NULL;
		hr = dWriteFactory->GetSystemFontCollection(&fontCollection);
		if (FAILED(hr))
		{
			_HL_TRY_RELEASE(dWriteFactory);
			return fonts;
		}
		unsigned int familyCount = fontCollection->GetFontFamilyCount();
		unsigned int index = 0;
		BOOL exists = false;
		wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = {0};
		int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);
		for_itert (unsigned int, i, 0, familyCount)
		{
			IDWriteFontFamily* fontFamily = NULL;
			hr = fontCollection->GetFontFamily(i, &fontFamily);
			if (!FAILED(hr))
			{
				IDWriteLocalizedStrings* familyNames = NULL;
				hr = fontFamily->GetFamilyNames(&familyNames);
				if (!FAILED(hr))
				{
					if (defaultLocaleSuccess > 0)
					{
						hr = familyNames->FindLocaleName(localeName, &index, &exists);
					}
					if (!FAILED(hr) && !exists)
					{
						hr = familyNames->FindLocaleName(L"en-us", &index, &exists);
						if (FAILED(hr))
						{
							_HL_TRY_RELEASE(familyNames);
							_HL_TRY_RELEASE(fontFamily);
							continue;
						}
					}
					if (!exists)
					{
						index = 0;
					}
					unsigned int length = 0;
					hr = familyNames->GetStringLength(index, &length);
					if (!FAILED(hr))
					{
						wchar_t* name = new wchar_t[length + 1];
						hr = familyNames->GetString(index, name, length + 1);
						if (!FAILED(hr))
						{
							fonts += hstr::from_unicode(name);
						}
						delete [] name;
					}
					_HL_TRY_RELEASE(familyNames);
				}
				_HL_TRY_RELEASE(fontFamily);
			}
		}
		_HL_TRY_RELEASE(fontCollection);
		_HL_TRY_RELEASE(dWriteFactory);
#endif
#elif defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
#elif defined(__APPLE__) && !TARGET_OS_MAC && TARGET_OS_IPHONE
#elif defined(__UNIX__)
#endif
		fonts.remove_duplicates();
		fonts.sort();
#else
		hlog::warn(atresttf::logTag, "AtresTTF compiled without system font support.");
#endif
		fontsChecked = true;
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
#if !_HL_WINRT
		return (normalize_path(get_environment_variable("WinDir")) + "/Fonts/");
#else
		return "";
#endif
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
