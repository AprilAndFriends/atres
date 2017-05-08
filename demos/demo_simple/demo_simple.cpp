/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef _ANDROID
#ifndef _WINRT
#define RESOURCE_PATH "../../demos/media/"
#else
#define RESOURCE_PATH "media/"
#endif
#elif defined(__APPLE__)
#define RESOURCE_PATH "media/"
#else
#define RESOURCE_PATH "./"
#endif

//#define _ATRESTTF // uncomment for testing if needed
//#define _ATRESTTF_LOCAL_FONT // uncomment for testing if needed

#define LOG_TAG "demo_simple"

#include <april/april.h>
#include <april/KeyboardDelegate.h>
#include <april/Keys.h>
#include <april/main.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <atres/atres.h>
#include <atres/FontBitmap.h>
#include <atres/FontIconMap.h>
#include <atres/Renderer.h>
#include <atresttf/atresttf.h>
#include <atresttf/FontTtf.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define TEXT_0 "This is blasphemy.[i:icon_font]red[/i]"
#define TEXT_1 "This is [i:icon_font]xmen[/i]madness."
#define TEXT_2 "No. [i:icon_font]neon[/i] This is Sparta!"
#define TEXT_3 "This []]is] a [b=7F00FF][0]test [n]that[/n] [f:test]is[/f] [x=0.5]supp[/x]osed[/b] " \
	"[b][c=fake]to[/c][/b]   [c=FFFFFF7F]show[/c] [f Arial:0.5][b]whe[/b]ther[/f] [s]X_horz_formatting " \
	"[c=00FF00]wrapped[/c][/s] [b][c=purple][i:icon_font]neon[/i][/c]w[c=purple][i:icon_font]neon[/i]rk[/c]s[/b] [f Arial:1.3]right[/f] [-][b]or[/b][/k] not."
#define TEXT_4 "Another [b][0]test [n]that[/n] [f:test]is[/f] [x=0.5]supp[/x]osed[/b] [b][c=fake]to[/c][/b] " \
	"[c=FFFFFF7F]sh[i:icon_font]blue[/i][i:icon_font]orange[/i]w[/c] [f Arial:0.5][b]whe[/b]ther[/f] [s=00FF00]X_horz_formatting [c=00FF00]justified[/c][/s] " \
	"[b]w[c=purple]ork[/c]s[/b] [f Arial:1.3]right[/f] or\nnot."
#define TEXT_5 "[b:,3]This is a vertical test.\nIt really is. Really."
#define TEXT_6 "This is [c:FFFF00][t:FF0000]a [b=007FFF,2]strike-[i:icon_font]blue[/i]-[c:00FF00]through[/c]\ntest[/b][/t] [t]an[/t][/c][t]d[/t] " \
	"this[u:,4]\nis an [s=007FFF,2,2]under[c:FF0000]line[/c]\nte[/s]st[/u]."

grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
grect viewport(0.0f, 0.0f, 1024.0f, 768.0f);
grect textArea0(60.0f, 24.0f, 640.0f, 64.0f);
grect textArea1(70.0f, 144.0f, 864.0f, 32.0f);
grect textArea2(120.0f, 400.0f, 600.0f, 128.0f);
grect textArea3(80.0f, 550.0f, 360.0f, 184.0f);
grect textArea4(768.0f, 200.0f, 160.0f, 384.0f);
grect textArea5(700.0f, 600.0f, 240.0f, 76.0f);
grect textArea6(60.0f, 200.0f, 400.0f, 160.0f);
april::Color backgroundColor = april::Color(0, 0, 0, 128);

class KeyboardDelegate : public april::KeyboardDelegate
{
public:
	void onKeyDown(april::Key keyCode)
	{
	}

	void onKeyUp(april::Key keyCode)
	{
		if (keyCode == april::Key::Backspace)
		{
			atres::renderer->setBorderColor(april::Color(hrand(256), hrand(256), hrand(256)));
		}
		else if (keyCode == april::Key::Space)
		{
			atres::renderer->setBorderThickness(hrandf(1.0f, 5.0f));
		}
	}

	void onChar(unsigned int charCode)
	{
	}

};

static KeyboardDelegate* keyboardDelegate = NULL;

class MouseDelegate : public april::MouseDelegate
{
public:
	gvec2 offset;

	MouseDelegate() : april::MouseDelegate(), clicked(false)
	{
	}

	void onMouseDown(april::Key key)
	{
		this->position = april::window->getCursorPosition() + this->offset;
		this->clicked = true;
	}

	void onMouseUp(april::Key key)
	{
		this->clicked = false;
	}

	void onMouseMove()
	{
		if (this->clicked)
		{
			this->offset = (this->position - april::window->getCursorPosition());
		}
	}

	void onMouseScroll(float x, float y)
	{
	}

	void onMouseCancel(april::Key key)
	{
	}

protected:
	bool clicked;
	gvec2 position;

};

static MouseDelegate* mouseDelegate = NULL;

class UpdateDelegate : public april::UpdateDelegate
{
public:
	april::Color color;

	UpdateDelegate() : april::UpdateDelegate(), time(0.0f)
	{
	}
	
	bool onUpdate(float timeDelta)
	{
		this->time += timeDelta;
		this->color.a = 191 + (unsigned char)(64 * hsin(this->time * 360.0f));
		// rendering
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(viewport);
		april::rendersys->drawFilledRect(viewport, april::Color::DarkGrey);
		// backgrounds
		april::rendersys->drawFilledRect(textArea0, backgroundColor);
		april::rendersys->drawFilledRect(textArea1, backgroundColor);
		april::rendersys->drawFilledRect(textArea2, backgroundColor);
		april::rendersys->drawFilledRect(textArea3, backgroundColor);
		april::rendersys->drawFilledRect(textArea4, backgroundColor);
		april::rendersys->drawFilledRect(textArea5, backgroundColor);
		april::rendersys->drawFilledRect(textArea6, backgroundColor);
		// texts
		atres::renderer->drawText(textArea0, TEXT_0, atres::Horizontal::Center, atres::Vertical::Top);
		atres::renderer->drawText("Arial:0.8", textArea1, TEXT_1, atres::Horizontal::LeftWrapped);
		atres::renderer->drawText("Arial:2.0", textArea2, TEXT_2, atres::Horizontal::RightWrapped, atres::Vertical::Bottom);
		atres::renderer->drawText(textArea3, TEXT_3, atres::Horizontal::CenterWrapped, atres::Vertical::Center);
		atres::renderer->drawText("Arial:0.8", textArea4, TEXT_4, atres::Horizontal::Justified, atres::Vertical::Center, this->color);
		atres::renderer->drawText(textArea5, TEXT_5, atres::Horizontal::Center, atres::Vertical::Center, april::Color::White, mouseDelegate->offset);
		atres::renderer->drawText(textArea6, TEXT_6, atres::Horizontal::Center, atres::Vertical::Center, april::Color::White, mouseDelegate->offset);
		return true;
	}

protected:
	float time;

};

UpdateDelegate* updateDelegate = NULL;

void april_init(const harray<hstr>& args)
{
#ifdef __APPLE__
	// On MacOSX, the current working directory is not set by
	// the Finder, since you are expected to use Core Foundation
	// or ObjC APIs to find files. 
	// So, when porting you probably want to set the current working
	// directory to something sane (e.g. .../Resources/ in the app
	// bundle).
	// In this case, we set it to parent of the .app bundle.
	{	// curly braces in order to localize variables 

		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		// let's hope chdir() will be happy with utf8 encoding
		const char* cpath = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
		char* cpath_alloc = NULL;
		if (cpath == NULL)
		{
			// CFStringGetCStringPtr is allowed to return NULL. bummer.
			// we need to use CFStringGetCString instead.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			CFStringGetCString(path, cpath_alloc, CFStringGetLength(path) + 1, kCFStringEncodingUTF8);
		}
		else
		{
			// even though it didn't return NULL, we still want to slice off bundle name.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			strcpy(cpath_alloc, cpath);
		}
		// just in case / is appended to .app path for some reason
		if (cpath_alloc[CFStringGetLength(path) - 1] == '/')
		{
			cpath_alloc[CFStringGetLength(path) - 1] = 0;
		}
		// replace pre-.app / with a null character, thus
		// cutting off .app's name and getting parent of .app.
		strrchr(cpath_alloc, '/')[0] = 0;
		// change current dir using posix api
		chdir(cpath_alloc);
		free(cpath_alloc); // even if null, still ok
		CFRelease(path);
		CFRelease(url);
	}
#endif
	keyboardDelegate = new KeyboardDelegate();
	mouseDelegate = new MouseDelegate();
	updateDelegate = new UpdateDelegate();
	updateDelegate->color = april::Color::White;
	try
	{
#if defined(_ANDROID) || defined(_IOS)
		drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
		april::init(april::RenderSystemType::Default, april::WindowType::Default);
		april::createRenderSystem();
		april::createWindow((int)drawRect.w, (int)drawRect.h, false, "demo_simple");
		april::window->setUpdateDelegate(updateDelegate);
		april::window->setKeyboardDelegate(keyboardDelegate);
		april::window->setMouseDelegate(mouseDelegate);
		atres::init();
#ifndef _ATRESTTF
		atres::renderer->registerFont(new atres::FontBitmap(RESOURCE_PATH "arial.font"));
#else
		atresttf::init();
		hlog::writef(LOG_TAG, "Found %d fonts installed on the system.", atresttf::getSystemFonts().size());
#ifdef _ATRESTTF_LOCAL_FONT
		hstr ttfFilename = RESOURCE_PATH "arial.ttf"; // invokes a provided font
#else
		hstr ttfFilename; // invokes the installed system font Arial
#endif
		atresttf::FontTtf* fontTtf = new atresttf::FontTtf(ttfFilename, "Arial", 32.0f, 1.0f);
		fontTtf->setBorderMode(atres::Font::BorderMode::PrerenderDiamond);
		atres::renderer->registerFont(fontTtf);
#endif
		atres::renderer->registerFont(new atres::FontIconMap(RESOURCE_PATH "icon_font", "icon_font", 1.0f, 0.0f, 4.0f));
		atres::renderer->setShadowColor(april::Color::Red);
		atres::renderer->setBorderColor(april::Color::Aqua);
		atres::renderer->setStrikeThroughThickness(2.0f);
	}
	catch (hexception& e)
	{
		hlog::error(LOG_TAG, e.getMessage());
	}
}

void april_destroy()
{
	try
	{
#ifdef _ATRESTTF
		atresttf::destroy();
#endif
		atres::destroy();
		april::destroy();
		delete keyboardDelegate;
		keyboardDelegate = NULL;
		delete mouseDelegate;
		mouseDelegate = NULL;
		delete updateDelegate;
		updateDelegate = NULL;
	}
	catch (hexception& e)
	{
		hlog::error(LOG_TAG, e.getMessage());
	}
}
