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

#include <hltypes/hstring.h>

#include "atres.h"
#include "FontResource.h"
#include "Renderer.h"

namespace atres
{
	void atres_writelog(chstr message)
	{
		printf("%s\n", message.c_str());
	}
	void (*g_logFunction)(chstr) = atres_writelog;

    void init()
    {
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
