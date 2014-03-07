/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines macros for DLL exports/imports.

#ifndef ATRESTTF_EXPORT_H
#define ATRESTTF_EXPORT_H

	/// @def atresttfExport
	/// @brief Macro for DLL exports/imports.
	/// @def atresttfFnExport
	/// @brief Macro for function DLL exports/imports.
	#ifdef _LIB
		#define atresttfExport
		#define atresttfFnExport
	#else
		#ifdef _WIN32
			#ifdef ATRESTTF_EXPORTS
				#define atresttfExport __declspec(dllexport)
				#define atresttfFnExport __declspec(dllexport)
			#else
				#define atresttfExport __declspec(dllimport)
				#define atresttfFnExport __declspec(dllimport)
			#endif
		#else
			#define atresttfExport __attribute__ ((visibility("default")))
			#define atresttfFnExport __attribute__ ((visibility("default")))
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE __declspec(deprecated("function is deprecated"))
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif

