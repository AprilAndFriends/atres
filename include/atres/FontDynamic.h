/// @file
/// @version 5.0
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
	/// @brief Special font definition type that dynamically loads symbols on demand.
	class atresExport FontDynamic : public Font
	{
	public:
		/// @brief Basic constructor.
		/// @param[in] filename The filename of the font definition.
		FontDynamic(chstr name);
		/// @brief Constructor.
		/// @param[in] filename The filename of the font definition.
		/// @param[in] textureSize The filename texture size.
		FontDynamic(chstr name, int textureSize);
		/// @brief Destructor.
		~FontDynamic();

		/// @brief The texture size of the font.
		HL_DEFINE_GET(int, textureSize, textureSize);
		/// @brief Sets the border rendering mode.
		/// @param[in] value The border rendering mode.
		void setBorderMode(const BorderMode& value);

		/// @brief Get the texture where the character definition for a specific char code is currently contained.
		/// @param[in] charCode Character unicode value.
		/// @return The texture.
		april::Texture* getTexture(unsigned int charCode);
		/// @brief Get the texture where the border character definition for a specific char code is currently contained.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The texture.
		april::Texture* getBorderTexture(unsigned int charCode, float borderThickness);
		/// @brief Get the texture where the icon definition for a specific icon name is currently contained.
		/// @param[in] iconName Icon name.
		/// @return The texture.
		april::Texture* getTexture(chstr iconName);
		/// @brief Get the texture where the border icon definition for a specific icon name is currently contained.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The texture.
		april::Texture* getBorderTexture(chstr iconName, float borderThickness);
		/// @brief Checks if a character definition has been loaded already.
		/// @param[in] charCode Character unicode value.
		/// @return True if character is loaded.
		bool hasCharacter(unsigned int charCode);
		/// @brief Checks if a border character definition has been loaded already.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if border character is loaded.
		bool hasBorderCharacter(unsigned int charCode, float borderThickness);
		/// @brief Checks if a icon definition has been loaded already.
		/// @param[in] iconName Icon name.
		/// @return True if icon is loaded.
		bool hasIcon(chstr iconName);
		/// @brief Checks if a border icon definition has been loaded already.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if border icon is loaded.
		bool hasBorderIcon(chstr iconName, float borderThickness);

		/// @brief Loads basic ASCII range of characters.
		/// @param[in] iconName Icon name.
		/// @return True if icon is loaded.
		void loadBasicAsciiCharacters();
		/// @brief Loads basic ASCII range of border characters.
		/// @param[in] borderThickness Thickness of the border.
		void loadBasicAsciiBorderCharacters(float borderThickness);

	protected:
		/// @brief Helper class for structuring images when using a prerendered border rendering mode.
		class StructuringImageContainer
		{
		public:
			/// @brief The actual structuring image.
			april::Image* image;
			/// @brief The border render mode.
			BorderMode borderMode;
			/// @brief The thickness of the border.
			float borderThickness;

			/// @brief Basic constructor.
			/// @param[in] image The actual structuring image.
			/// @param[in] borderMode The border render mode.
			/// @param[in] borderThickness The thickness of the border.
			StructuringImageContainer(april::Image* image, const BorderMode& borderMode, float borderThickness);
			/// @brief Basic destructor.
			~StructuringImageContainer();

		};

		/// @brief Font texture size.
		int textureSize;
		/// @brief All structuring image containers.
		harray<StructuringImageContainer*> structuringImageContainers;

		/// @brief Checks if alpha-textures can be used for this font.
		/// @return True if alpha-textures can be used for this font.
		virtual bool _isAllowAlphaTextures() const;
		/// @brief Creates a texture contaner if there are none yet.
		void _tryCreateFirstTextureContainer();
		/// @brief Creates a texture contaner for border images if there are none yet.
		/// @param[in] borderThickness The thickness of the border.
		void _tryCreateFirstBorderTextureContainer(float borderThickness);

		/// @brief Finds the given structuring image container for a given border render mode and thickness.
		/// @param[in] borderMode The border render mode.
		/// @param[in] borderThickness The thickness of the border.
		StructuringImageContainer* _findStructuringImageContainer(const BorderMode& borderMode, float borderThickness);

		/// @brief Creates a new texture for the font symbols.
		/// @return A new texture.
		april::Texture* _createTexture();
		/// @brief Attempts to add the character bitmap to the texture.
		/// @param[in] charCode Character unicode value.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @return True if successful.
		/// @note Usually false is returned when the character couldn't be loaded or created properly from the font definition.
		bool _tryAddCharacterBitmap(unsigned int charCode, bool initial = false);
		/// @brief Attempts to add the border character bitmap to the texture.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if successful.
		/// @note Usually false is returned when the border character couldn't be or created loaded properly from the font definition.
		bool _tryAddBorderCharacterBitmap(unsigned int charCode, float borderThickness);
		/// @brief Attempts to add the icon bitmap to the texture.
		/// @param[in] iconName Icon name.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @return True if successful.
		/// @note Usually false is returned when the icon couldn't be loaded or created properly from the font definition.
		bool _tryAddIconBitmap(chstr iconName, bool initial = false);
		/// @brief Attempts to add the border icon bitmap to the texture.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if successful.
		/// @note Usually false is returned when the border icon couldn't be loaded or created properly from the font definition.
		bool _tryAddBorderIconBitmap(chstr iconName, float borderThickness);
		/// @brief Add the symbol bitmap to the texture.
		/// @param[in] textureContainers Proper symbol type texture containers.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @param[in] image The symbol bitmap image.
		/// @param[in] usedWidth The used width on the texture so far.
		/// @param[in] usedHeight The used height on the texture so far.
		/// @param[in] symbol The symbol value.
		/// @param[in] offsetX Horizontal offset used between symbols.
		/// @param[in] offsetY Vertical offset used between symbols.
		/// @param[in] safeSpace Safe space between all symbols used when rendering.
		/// @return The texture container of the texture where the symbol bitmap was written.
		TextureContainer* _addBitmap(harray<TextureContainer*>& textureContainers, bool initial, april::Image* image, int usedWidth, int usedHeight, chstr symbol,
			int offsetX = 0, int offsetY = 0, int safeSpace = 0);

		/// @brief Loads an character image.
		/// @param[in] charCode Character unicode value.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @param[out] advance Horizontal advance value.
		/// @param[out] leftOffset Horizontal offset from the left boundary of the bitmap.
		/// @param[out] topOffset Vertical offset from the top boundary of the bitmap.
		/// @param[out] ascender Ascender value.
		/// @param[out] descender Descender value.
		/// @param[out] bearingX Horizontal bearing.
		/// @return The loaded image.
		virtual april::Image* _loadCharacterImage(unsigned int charCode, bool initial, float& advance, int& leftOffset, int& topOffset, float& ascender, float& descender, float& bearingX);
		/// @brief Loads a border character image.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The loaded image.
		virtual april::Image* _loadBorderCharacterImage(unsigned int charCode, float borderThickness);
		/// @brief Generates a border character image.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The loaded image.
		virtual april::Image* _generateBorderCharacterImage(unsigned int charCode, float borderThickness);
		/// @brief Loads an icon image.
		/// @param[in] iconName Name of the icon image to load.
		/// @param[in] initial Whether this is the first attempt to write on the texture (used for internal optimization).
		/// @param[out] advance Horizontal advance value.
		/// @return The loaded image.
		virtual april::Image* _loadIconImage(chstr iconName, bool initial, float& advance);
		/// @brief Loads a border icon image.
		/// @param[in] iconName Name of the border icon image to load.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The loaded image.
		virtual april::Image* _loadBorderIconImage(chstr iconName, float borderThickness);
		/// @brief Generates a border icon image.
		/// @param[in] iconName Name of the border icon image to load.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The loaded image.
		virtual april::Image* _generateBorderIconImage(chstr iconName, float borderThickness);

		/// @brief Creates a structuring image for a given border rendering mode and thickness.
		/// @param[in] borderThickness Thickness of the border.
		/// @return A container for the structuring image.
		StructuringImageContainer* _createStructuringImageContainer(const BorderMode& borderMode, float borderThickness);

	};

}
#endif

