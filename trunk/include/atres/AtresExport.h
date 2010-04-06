/************************************************************************************
This source file is part of the Advanced Text Rendering System
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
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

