/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#ifdef _ANDROID
#include <android/log.h>
#endif

#include <hltypes/hstring.h>

#include "atres.h"
#include "FontResource.h"
#include "Renderer.h"

namespace atres
{
	void atres_writelog(chstr message)
	{
#ifndef _ANDROID
		printf("%s\n", message.c_str());
#else
		__android_log_print(ANDROID_LOG_INFO, "april", "%s", message.c_str());
#endif
	}
	void (*g_logFunction)(chstr) = atres_writelog;

    void init()
    {
		atres::log("initializing atres");
		renderer = new Renderer();
    }
    
    void destroy()
    {
		delete renderer;
    }

	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction = fnptr;
	}
	
	void log(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
}
