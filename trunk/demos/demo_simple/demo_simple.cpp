/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hplatform.h>
#ifndef _ANDROID
#if !_HL_WINRT
#define RESOURCE_PATH "../media/"
#else
#define RESOURCE_PATH "media/"
#endif
#else
#define RESOURCE_PATH "./"
#endif

#define LOG_TAG "demo_simple"

//#define _ATRESTTF

#include <april/april.h>
#include <april/KeyboardDelegate.h>
#include <april/Keys.h>
#include <april/main.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <aprilui/aprilui.h>
#include <aprilui/Dataset.h>
#include <aprilui/Objects.h>
#include <atres/atres.h>
#include <atres/FontResourceBitmap.h>
#include <atres/Renderer.h>
#include <atresttf/atresttf.h>
#include <atresttf/FontResourceTtf.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
grect viewport(0.0f, 0.0f, 1024.0f, 768.0f);
grect textArea(700.0f, 600.0f, 240.0f, 76.0f);
aprilui::Dataset* dataset;
aprilui::Object* root;
aprilui::Label* specialText;

class KeyboardDelegate : public april::KeyboardDelegate
{
public:
	void onKeyDown(april::Key keyCode)
	{
		aprilui::onKeyDown(keyCode);
	}

	void onKeyUp(april::Key keyCode)
	{
		switch (keyCode)
		{
		case april::AK_BACK:
			atres::renderer->setBorderColor(april::Color(hrand(256), hrand(256), hrand(256)));
			break;
		case april::AK_SPACE:
			atres::renderer->setBorderOffset(hrandf(1.0f, 5.0f));
			break;
		default:
			break;
		}
		aprilui::onKeyUp(keyCode);
	}

	void onChar(unsigned int charCode)
	{
		aprilui::onChar(charCode);
	}

};

class MouseDelegate : public april::MouseDelegate
{
public:
	gvec2 offset;

	MouseDelegate::MouseDelegate() : clicked(false)
	{
	}

	void onMouseDown(april::Key button)
	{
		aprilui::updateCursorPosition();
		aprilui::onMouseDown(button);
		this->position = aprilui::getCursorPosition() + this->offset;
		this->clicked = true;
	}

	void onMouseUp(april::Key button)
	{
		aprilui::updateCursorPosition();
		aprilui::onMouseUp(button);
		this->clicked = false;
	}

	void onMouseMove()
	{
		aprilui::updateCursorPosition();
		aprilui::onMouseMove();
		if (this->clicked)
		{
			this->offset = (this->position - aprilui::getCursorPosition());
		}
	}

	void onMouseScroll(float x, float y)
	{
		aprilui::onMouseScroll(x, y);
	}

protected:
	bool clicked;
	gvec2 position;

};

static KeyboardDelegate* keyboardDelegate = NULL;
static MouseDelegate* mouseDelegate = NULL;

class UpdateDelegate : public april::UpdateDelegate
{
public:
	UpdateDelegate::UpdateDelegate() : time(0.0f)
	{
		this->color = april::Color::White;
	}

	bool UpdateDelegate::onUpdate(float timeSinceLastFrame)
	{
		this->time += timeSinceLastFrame;
		this->color.a = 191 + (unsigned char)(64 * dsin(this->time * 360.0f));
		specialText->setTextColor(this->color);
		// rendering
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(viewport);
		aprilui::updateCursorPosition();
		april::rendersys->drawFilledRect(viewport, april::Color(128, 128, 0));
		root->update(timeSinceLastFrame);
		root->draw();
		april::rendersys->drawFilledRect(textArea, april::Color(0, 0, 0, 128));
		atres::renderer->drawText(textArea, "[b]This is a vertical test.\nIt really is. Really.",
			atres::CENTER, atres::CENTER, april::Color::White, mouseDelegate->offset);
		return true;
	}

protected:
	float time;
	april::Color color;

};

static UpdateDelegate* updateDelegate = NULL;

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
	updateDelegate = new UpdateDelegate();
	keyboardDelegate = new KeyboardDelegate();
	mouseDelegate = new MouseDelegate();
	try
	{
#if defined(_ANDROID) || defined(_IOS)
		drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
		april::init(april::RS_DEFAULT, april::WS_DEFAULT);
		april::createRenderSystem();
		april::createWindow((int)drawRect.w, (int)drawRect.h, false, "demo_simple");
		april::window->setUpdateDelegate(updateDelegate);
		april::window->setKeyboardDelegate(keyboardDelegate);
		april::window->setMouseDelegate(mouseDelegate);
		atres::init();
#ifndef _ATRESTTF
		atres::renderer->registerFontResource(new atres::FontResourceBitmap(RESOURCE_PATH "arial.font"));
#else
		atresttf::init();
		hlog::writef(LOG_TAG, "Found %d fonts installed on the system.", atresttf::getSystemFonts().size());
		atres::renderer->registerFontResource(new atresttf::FontResourceTtf("arial.ttf", "Arial", 32, 1.0f)); // invokes a provided font
		//atres::renderer->registerFontResource(new atresttf::FontResourceTtf("", "Arial", 32, 1.0f)); // invokes the installed system font Arial
#endif
		atres::renderer->setShadowColor(april::Color::Red);
		atres::renderer->setBorderColor(april::Color::Aqua);
		aprilui::init();
#ifdef _DEBUG
		aprilui::setDebugEnabled(true);
#endif
		dataset = new aprilui::Dataset(RESOURCE_PATH "demo_simple.dts");
		dataset->load();
		aprilui::Label* label = dataset->getObject<aprilui::Label*>("test_4");
		label->setText("This is a vertical test.\nIt really is. Really.");
		root = dataset->getObject("root");
		specialText = dataset->getObject<aprilui::Label*>("test_5");
	}
	catch (hltypes::exception e)
	{
		hlog::error(LOG_TAG, e.message());
	}
}

void april_destroy()
{
	try
	{
		delete dataset;
		aprilui::destroy();
#ifdef _ATRESTTF
		atresttf::destroy();
#endif
		atres::destroy();
		april::destroy();
	}
	catch (hltypes::exception e)
	{
		hlog::error(LOG_TAG, e.message());
	}
	delete updateDelegate;
	updateDelegate = NULL;
	delete keyboardDelegate;
	keyboardDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
}
