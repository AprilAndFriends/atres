/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_EXPORT_H
#define ATRES_EXPORT_H

	#ifdef _STATICLIB
		#define atresExport
		#define atresFnExport
	#else
		#ifdef _WIN32
			#ifdef ATRES_EXPORTS
				#define atresExport __declspec(dllexport)
				#define atresFnExport __declspec(dllexport)
			#else
				#define atresExport __declspec(dllimport)
				#define atresFnExport __declspec(dllimport)
			#endif
		#else
			#define atresExport __attribute__ ((visibility("default")))
			#define atresFnExport
		#endif
	#endif

#endif

