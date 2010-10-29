/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef ATRES_EXPORT_H
#define ATRES_EXPORT_H

	#ifdef _STATICLIB
		#define AtresExport
		#define AtresFnExport
	#else
		#ifdef _WIN32
			#ifdef ATRES_EXPORTS
				#define AtresExport __declspec(dllexport)
				#define AtresFnExport __declspec(dllexport)
			#else
				#define AtresExport __declspec(dllimport)
				#define AtresFnExport __declspec(dllimport)
			#endif
		#else
			#define AtresExport __attribute__ ((visibility("default")))
			#define AtresFnExport
		#endif
	#endif

#endif

