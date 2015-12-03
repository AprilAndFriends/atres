/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a font with a dynamically created character map.

#ifndef ATRESTTF_FONT_DYNAMIC_H
#define ATRESTTF_FONT_DYNAMIC_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "atresExport.h"
#include "Font.h"

namespace april
{
	class Texture;
}

namespace atres
{
	class atresExport FontDynamic : public Font
	{
	public:
		FontDynamic(chstr name);
		~FontDynamic();

		void setBorderMode(BorderMode value);

		april::Texture* getTexture(unsigned int charCode);
		april::Texture* getBorderTexture(unsigned int charCode, float borderThickness);
		april::Texture* getTexture(chstr iconName);
		april::Texture* getBorderTexture(chstr iconName, float borderThickness);
		bool hasCharacter(unsigned int charCode);
		bool hasBorderCharacter(unsigned int charCode, float borderThickness);
		bool hasIcon(chstr iconName);
		bool hasBorderIcon(chstr iconName, float borderThickness);

	protected:
		class StructuringImageContainer
		{
		public:
			april::Image* image;
			BorderMode borderMode;
			float borderThickness;

			StructuringImageContainer(april::Image* image, BorderMode borderMode, float borderThickness);
			~StructuringImageContainer();

		};

		harray<StructuringImageContainer*> structuringImageContainers;

		virtual bool _isAllowAlphaTextures();
		void _tryCreateFirstTextureContainer();
		void _tryCreateFirstBorderTextureContainer(float borderThickness);

		StructuringImageContainer* _findStructuringImageContainer(BorderMode borderMode, float borderThickness);

		april::Texture* _createTexture();
		bool _addCharacterBitmap(unsigned int charCode, bool initial = false);
		bool _addBorderCharacterBitmap(unsigned int charCode, float borderThickness);
		bool _addIconBitmap(chstr iconName, bool initial = false);
		bool _addBorderIconBitmap(chstr iconName, float borderThickness);
		TextureContainer* _addBitmap(harray<TextureContainer*>& textureContainers, bool initial, april::Image* image, int bitmapWidth, int bitmapHeight, chstr symbol, int offsetX = 0, int offsetY = 0, int safeSpace = 0);

		virtual april::Image* _loadCharacterImage(unsigned int charCode, bool initial, int& advance, int& leftOffset, int& topOffset, int& ascender, int& descender, int& bearingX);
		virtual april::Image* _loadBorderCharacterImage(unsigned int charCode, float borderThickness);
		virtual april::Image* _generateBorderCharacterImage(unsigned int charCode, float borderThickness);
		virtual april::Image* _loadIconImage(chstr iconName, bool initial, int& advance);
		virtual april::Image* _loadBorderIconImage(chstr iconName, float borderThickness);
		virtual april::Image* _generateBorderIconImage(chstr iconName, float borderThickness);

		StructuringImageContainer* _createStructuringImageContainer(BorderMode borderMode, float borderThickness);

	};

}

#endif

