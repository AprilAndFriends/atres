/************************************************************************************\
This source file is part of the APRIL User Interface Library                         *
For latest info, see http://libaprilui.sourceforge.net/                              *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>

#include <april/RenderSystem.h>
#include <april/Keys.h>
#include <april/Window.h>
#include <aprilui/aprilui.h>
#include <aprilui/Dataset.h>
#include <aprilui/Objects.h>
#include <atres/atres.h>
#include <gtypes/Rectangle.h>
#include <hltypes/util.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

grect screen(0, 0, 1024, 768);

grect text_area(700, 600, 240, 76);
aprilui::Dataset* dataset;
aprilui::Object* root;

bool clicked;
gvec2 position;
gvec2 offset;

bool render(float time_increase)
{
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(screen);
	root->update(time_increase);
	root->draw();
	april::rendersys->drawColoredQuad(text_area, april::Color(APRIL_COLOR_BLUE, 128));
	atres::drawText(text_area, "[b]This is a vertical test.\nIt really is. Really.",
		atres::CENTER, atres::CENTER, APRIL_COLOR_WHITE, offset);
	aprilui::updateCursorPosition();
	return true;
}

void onMouseDown(float x, float y, int button)
{
	root->OnMouseDown(x, y, button);
	position = april::rendersys->getWindow()->getCursorPos() + offset;
	clicked = true;
}

void onMouseUp(float x, float y, int button)
{
	root->OnMouseUp(x, y, button);
	clicked = false;
}

void onMouseMove(float x, float y)
{
	if (clicked)
	{
		offset = position - gvec2(x, y);
	}
	root->OnMouseMove(x, y);
}

void onKeyDown(unsigned int keycode)
{
	root->OnKeyDown(keycode);
}

void onKeyUp(unsigned int keycode)
{
	if (keycode == april::AK_BACK)
	{
		atres::setBorderColor(april::Color(hrand(256), hrand(256), hrand(256)));
	}
	if (keycode == april::AK_SPACE)
	{
		atres::setBorderOffset(hrandf(1.0f, 5.0f));
	}
	root->OnKeyUp(keycode);
}

void onChar(unsigned int charcode)
{
	root->OnChar(charcode);
}

int main()
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
		april::init("renderer", (int)screen.w, (int)screen.h, false, "demo_simple");
		april::rendersys->getWindow()->setUpdateCallback(render);
		april::rendersys->getWindow()->setMouseCallbacks(&onMouseDown, &onMouseUp, &onMouseMove);
		april::rendersys->getWindow()->setKeyboardCallbacks(&onKeyDown, &onKeyUp, &onChar);
		aprilui::init();
#ifdef _DEBUG
		aprilui::setDebugMode(true);
#endif
		atres::init();
		atres::loadFont("../media/arial.font");
		atres::setShadowColor(APRIL_COLOR_RED);
		atres::setBorderColor(APRIL_COLOR_AQUA);
		dataset = new aprilui::Dataset("../media/demo_simple.datadef");
		dataset->load();
		aprilui::Label* label = dataset->getObject<aprilui::Label*>("test_4");
		label->setText("This is a vertical test.\nIt really is. Really.");
		root = dataset->getObject("root");
		april::rendersys->getWindow()->enterMainLoop();
		delete dataset;
		aprilui::destroy();
		atres::destroy();
		april::destroy();
	}
	catch (hltypes::exception e)
	{
		std::cout << e.message() << "\n";
	}
	return 0;
}
