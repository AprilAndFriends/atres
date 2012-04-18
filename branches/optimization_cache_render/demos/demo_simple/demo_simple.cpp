/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.43
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#define RESOURCE_PATH "./"
#else
#define RESOURCE_PATH "../media/"
#endif

//#define _ATRESTTF

#include <april/april.h>
#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Keys.h>
#include <april/Window.h>
#include <aprilui/aprilui.h>
#include <aprilui/Dataset.h>
#include <aprilui/Objects.h>
#include <atres/atres.h>
#include <atres/FontResourceBitmap.h>
#include <atresttf/atresttf.h>
#include <atresttf/FontResourceTtf.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hltypesUtil.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

grect drawRect(0.0f, 0.0f, 1024.0f, 768.0f);
#ifndef _ANDROID
grect viewport = drawRect;
#else
grect viewport(0.0f, 0.0f, 480.0f, 320.0f);
#endif
grect textArea(700.0f, 600.0f, 240.0f, 76.0f);
aprilui::Dataset* dataset;
aprilui::Object* root;

bool clicked;
gvec2 position;
gvec2 offset;

#include <atres/Renderer.h>

bool render(float k)
{
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	aprilui::updateCursorPosition();
	april::rendersys->drawColoredQuad(drawRect, april::Color(128, 128, 0));
	root->update(k);
	root->draw();
	april::rendersys->drawColoredQuad(textArea, april::Color(0, 0, 0, 128));
	atres::renderer->drawText(textArea, "[b]This is a vertical test.\nIt really is. Really.",
		atres::CENTER, atres::CENTER, APRIL_COLOR_WHITE, offset);
	return true;
}

void onMouseDown(float x, float y, int button)
{
	aprilui::updateCursorPosition();
	gvec2 cursorPosition = aprilui::getCursorPosition();
	aprilui::onMouseDown(cursorPosition.x, cursorPosition.y, button);
	position = cursorPosition + offset;// * viewport.getSize() / drawRect.getSize();
	clicked = true;
}

void onMouseUp(float x, float y, int button)
{
	aprilui::updateCursorPosition();
	gvec2 cursorPosition = aprilui::getCursorPosition();
	aprilui::onMouseUp(cursorPosition.x, cursorPosition.y, button);
	clicked = false;
}

void onMouseMove(float x, float y)
{
	aprilui::updateCursorPosition();
	gvec2 cursorPosition = aprilui::getCursorPosition();
	aprilui::onMouseMove(cursorPosition.x, cursorPosition.y);
	if (clicked)
	{
		offset = (position - cursorPosition);// * drawRect.getSize() / viewport.getSize();
	}
}

void onKeyDown(unsigned int keycode)
{
	aprilui::onKeyDown(keycode);
}

void onKeyUp(unsigned int keycode)
{
	if (keycode == april::AK_BACK)
	{
		atres::renderer->setBorderColor(april::Color(hrand(256), hrand(256), hrand(256)));
	}
	if (keycode == april::AK_SPACE)
	{
		atres::renderer->setBorderOffset(hrandf(1.0f, 5.0f));
	}
	aprilui::onKeyUp(keycode);
}

void onChar(unsigned int charcode)
{
	aprilui::onChar(charcode);
}

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
		const char* cpath=CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
		char* cpath_alloc=0;
		if(!cpath)
		{
			// CFStringGetCStringPtr is allowed to return NULL. bummer.
			// we need to use CFStringGetCString instead.
			cpath_alloc = (char*)malloc(CFStringGetLength(path)+1);
			CFStringGetCString(path, cpath_alloc, CFStringGetLength(path)+1, kCFStringEncodingUTF8);
		}
		else {
			// even though it didn't return NULL, we still want to slice off bundle name.
			cpath_alloc = (char*)malloc(CFStringGetLength(path)+1);
			strcpy(cpath_alloc, cpath);
		}
		// just in case / is appended to .app path for some reason
		if(cpath_alloc[CFStringGetLength(path)-1]=='/')
			cpath_alloc[CFStringGetLength(path)-1] = 0;
		
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
	try
	{
		april::init();
		april::createRenderSystem("");
		april::createRenderTarget((int)viewport.w, (int)viewport.h, false, "demo_simple");
		april::window->setUpdateCallback(render);
		april::window->setMouseCallbacks(&onMouseDown, &onMouseUp, &onMouseMove);
		april::window->setKeyboardCallbacks(&onKeyDown, &onKeyUp, &onChar);
		atres::init();
#ifdef _ATRESTTF
		atresttf::init();
#endif
#ifndef _ATRESTTF
		atres::renderer->registerFontResource(new atres::FontResourceBitmap(RESOURCE_PATH "arial.font"));
#else
		atres::renderer->registerFontResource(new atresttf::FontResourceTtf(RESOURCE_PATH "arial.ttfdef"));
#endif
		atres::renderer->setShadowColor(APRIL_COLOR_RED);
		atres::renderer->setBorderColor(APRIL_COLOR_AQUA);
		aprilui::init();
#ifdef _DEBUG
		aprilui::setDebugEnabled(true);
#endif
		aprilui::setViewport(viewport);
		aprilui::setScreenViewport(drawRect);
		dataset = new aprilui::Dataset(RESOURCE_PATH "demo_simple.dts");
		dataset->load();
		aprilui::Label* label = dataset->getObject<aprilui::Label*>("test_4");
		label->setText("This is a vertical test.\nIt really is. Really.");
		root = dataset->getObject("root");
#ifdef _ANDROID
		aprilui::Object* editbox = dataset->getObject("editbox");
		editbox->setSize(editbox->getSize() * drawRect.getSize() / viewport.getSize());
#endif
	}
	catch (hltypes::exception e)
	{
		printf("%s\n", e.message().c_str());
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
		printf("%s\n", e.message().c_str());
	}
}
