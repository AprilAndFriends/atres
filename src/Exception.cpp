/************************************************************************************\
This source file is part of the Advanced Text Rendering System                       *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "Exception.h"

namespace Atres
{
	FontNotFoundException::FontNotFoundException(chstr errorText)
	{
		mErrText = errorText;
	}

	hstr FontNotFoundException::repr()
	{

		return "FontNotFoundException: " + getErrorText();
	}

}
