/************************************************************************************\
This source file is part of the APRIL User Interface Library                         *
For latest info, see http://libaprilui.sourceforge.net/                              *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com),                                 *
                   Ivan Vucica (ivan@vucica.net)
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>

#include <april/RenderSystem.h>
#include <aprilui/AprilUI.h>
#include <aprilui/Dataset.h>
#include <aprilui/Objects.h>
#include <atres/Atres.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

AprilUI::Dataset* dataset;
AprilUI::Object* root;

bool render(float time_increase)
{
	April::rendersys->clear();
	April::rendersys->setOrthoProjection(SCREEN_WIDTH, SCREEN_HEIGHT);

	root->draw();
	root->update(time_increase);
	return true;
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
		April::init("OpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, 0, "demo_simple");
		April::rendersys->registerUpdateCallback(render);
		AprilUI::init();
#ifdef _DEBUG
		AprilUI::setDebugMode(true);
#endif
		Atres::loadFont("../media/arial.font");
		dataset = new AprilUI::Dataset("../media/demo_simple.datadef");
		dataset->load();
		AprilUI::Label* label = (AprilUI::Label*)dataset->getObject("test_4");
		label->setText("This is a vertical test.\nIt really is. Really.");
		label = (AprilUI::Label*)dataset->getObject("test_5");
		label->setText("This is a vertical test.\nIt really is. Really.");
		root = dataset->getObject("root");
		April::rendersys->enterMainLoop();
		delete dataset;
		AprilUI::destroy();
		April::destroy();
	}
	catch (hltypes::exception e)
	{
		std::cout << e.message() << "\n";
	}
	return 0;
}
