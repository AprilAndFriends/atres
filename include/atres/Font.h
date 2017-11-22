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
/// Defines a font object.

#ifndef ATRES_FONT_H
#define ATRES_FONT_H

#include <hltypes/henum.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "atres.h"
#include "atresExport.h"
#include "Utility.h"

namespace atres
{
	/// @brief Special handler that provides and loads font data from a source.
	class atresExport Font
	{
	public:
		/// @class Type
		/// @brief Defines border rendering mode.
		HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, BorderMode,
		(
			/// @var static const BorderMode BorderMode::Software
			/// @brief Manual rendering using multiple renders of the font.
			HL_ENUM_DECLARE(BorderMode, Software);
			/// @var static const BorderMode BorderMode::FontNative
			/// @brief Border rendering which is integrated into the font.
			HL_ENUM_DECLARE(BorderMode, FontNative);
			/// @var static const BorderMode BorderMode::PrerenderSquare
			/// @brief Manual generation of a border image with a square structuring image which is then used for border rendering.
			HL_ENUM_DECLARE(BorderMode, PrerenderSquare);
			/// @var static const BorderMode BorderMode::PrerenderCircle
			/// @brief Manual generation of a border image with a circle structuring image which is then used for border rendering.
			HL_ENUM_DECLARE(BorderMode, PrerenderCircle);
			/// @var static const BorderMode BorderMode::PrerenderDiamond
			/// @brief Manual generation of a border image with a diamond structuring image which is then used for border rendering.
			HL_ENUM_DECLARE(BorderMode, PrerenderDiamond);
		));

		/// @brief Basic constructor.
		/// @param[in] name Name of the font.
		Font(chstr name);
		/// @brief Destructor.
		virtual ~Font();

		/// @brief Loads the font definition.
		/// @return True if successfully loaded.
		bool load();

		/// @brief The font name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @brief Gets the calculated height of the font.
		/// @return The calculated height of the font.
		float getHeight() const;
		/// @brief Gets the scale of the font.
		/// @return The scale of the font.
		float getScale() const;
		/// @brief Set the scale of the font.
		HL_DEFINE_SET(float, scale, Scale);
		/// @brief Internal base scale of the font.
		HL_DEFINE_GETSET(float, baseScale, BaseScale);
		/// @brief Gets the line-height of the font.
		/// @return The line-height of the font.
		float getLineHeight() const;
		/// @brief Gets the descender of the font.
		/// @return The descender of the font.
		float getDescender() const;
		/// @brief Gets the internal descender of the font.
		/// @return The internal descender of the font.
		float getInternalDescender() const;
		/// @brief Gets the vertical strike-through offset of the font.
		/// @return The vertical strike-through offset of the font.
		float getStrikeThroughOffset() const;
		/// @brief Gets the vertical underline offset of the font.
		/// @return The vertical underline offset of the font.
		float getUnderlineOffset() const;
		/// @brief Gets the border rendering mode.
		HL_DEFINE_GET(BorderMode, borderMode, BorderMode);
		/// @brief Sets the border rendering mode.
		/// @param[in] value The border rendering mode.
		virtual void setBorderMode(const BorderMode& value);
		/// @brief Whether the font is loaded.
		HL_DEFINE_IS(loaded, Loaded);
		/// @brief Gets all character definitions.
		/// @return All character definitions.
		inline hmap<unsigned int, CharacterDefinition*>& getCharacters() { return this->characters; }
		/// @brief Gets all border character definitions.
		/// @return All border character definitions.
		inline hmap<unsigned int, harray<BorderCharacterDefinition*> >& getBorderCharacters() { return this->borderCharacters; }
		/// @brief Gets all icon definitions.
		/// @return All icon definitions.
		inline hmap<hstr, IconDefinition*>& getIcons() { return this->icons; }
		/// @brief Gets all border icon definitions.
		/// @return All border icon definitions.
		inline hmap<hstr, harray<BorderIconDefinition*> >& getBorderIcons() { return this->borderIcons; }
		/// @brief Gets all internal textures.
		/// @return All internal textures.
		harray<april::Texture*> getTextures() const;

		/// @brief Get the texture where the character definition for a specific char code is currently contained.
		/// @param[in] charCode Character unicode value.
		/// @return The texture.
		virtual april::Texture* getTexture(unsigned int charCode);
		/// @brief Get the texture where the border character definition for a specific char code is currently contained.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The texture.
		virtual april::Texture* getBorderTexture(unsigned int charCode, float borderThickness);
		/// @brief Get the texture where the icon definition for a specific icon name is currently contained.
		/// @param[in] iconName Icon name.
		/// @return The texture.
		virtual april::Texture* getTexture(chstr iconName);
		/// @brief Get the texture where the border icon definition for a specific icon name is currently contained.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The texture.
		virtual april::Texture* getBorderTexture(chstr iconName, float borderThickness);
		/// @brief Checks if a character definition has been loaded already.
		/// @param[in] charCode Character unicode value.
		/// @return True if character is loaded.
		virtual bool hasCharacter(unsigned int charCode);
		/// @brief Checks if a border character definition has been loaded already.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if border character is loaded.
		virtual bool hasBorderCharacter(unsigned int charCode, float borderThickness);
		/// @brief Checks if a icon definition has been loaded already.
		/// @param[in] iconName Icon name.
		/// @return True if icon is loaded.
		virtual bool hasIcon(chstr iconName);
		/// @brief Checks if a border icon definition has been loaded already.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return True if border icon is loaded.
		virtual bool hasBorderIcon(chstr iconName, float borderThickness);
		/// @brief Get the border character definition for a specific char code.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The border character definition.
		BorderCharacterDefinition* getBorderCharacter(unsigned int charCode, float borderThickness);
		/// @brief Get the border icon definition for a specific icon name.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The border icon definition.
		BorderIconDefinition* getBorderIcon(chstr iconName, float borderThickness);

		/// @brief Gets kerning between two char codes.
		/// @param[in] previousCharCode Character unicode value of the preceding character.
		/// @param[in] charCode Character unicode value.
		/// @return The kerning value.
		virtual float getKerning(unsigned int previousCharCode, unsigned int charCode);

		/// @brief Creates RenderRectangle definition for a character.
		/// @param[in] rect Container rect for character (used for clipping).
		/// @param[in] area Rect where text should be rendered.
		/// @param[in] charCode Character unicode value.
		/// @return The RenderRectangle definition.
		RenderRectangle makeRenderRectangle(cgrect rect, cgrect area, unsigned int charCode);
		/// @brief Creates RenderRectangle definition for a character.
		/// @param[in] rect Container rect for character (used for clipping).
		/// @param[in] area Rect where text should be rendered.
		/// @param[in] charCode Character unicode value.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The RenderRectangle definition.
		RenderRectangle makeBorderRenderRectangle(cgrect rect, cgrect area, unsigned int charCode, float borderThickness);
		/// @brief Creates RenderRectangle definition for an icon.
		/// @param[in] rect Container rect for text (used for clipping).
		/// @param[in] area Rect where text should be rendered.
		/// @param[in] iconName Icon name.
		/// @return The RenderRectangle definition.
		RenderRectangle makeRenderRectangle(cgrect rect, cgrect area, chstr iconName);
		/// @brief Creates RenderRectangle definition for an icon.
		/// @param[in] rect Container rect for text (used for clipping).
		/// @param[in] area Rect where text should be rendered.
		/// @param[in] iconName Icon name.
		/// @param[in] borderThickness Thickness of the border.
		/// @return The RenderRectangle definition.
		RenderRectangle makeBorderRenderRectangle(cgrect rect, cgrect area, chstr iconName, float borderThickness);

		/// @brief Loads basic ASCII range of characters.
		/// @param[in] iconName Icon name.
		/// @return True if icon is loaded.
		virtual void loadBasicAsciiCharacters();
		/// @brief Loads basic ASCII range of border characters.
		/// @param[in] borderThickness Thickness of the border.
		virtual void loadBasicAsciiBorderCharacters(float borderThickness);

		/// @brief The default border rendering mode for all fonts.
		static BorderMode defaultBorderMode;

	protected:
		/// @brief Font name.
		hstr name;
		/// @brief Font internal height.
		float height;
		/// @brief Font scale.
		float scale;
		/// @brief Font base scale.
		float baseScale;
		/// @brief Font line height.
		float lineHeight;
		/// @brief Font descender.
		float descender;
		/// @brief Font intenr descender.
		float internalDescender;
		/// @brief Font vertical strike-through offset.
		float strikeThroughOffset;
		/// @brief Font vertical underline offset.
		float underlineOffset;
		/// @brief Whether font is loaded.
		bool loaded;
		/// @brief Font border rendering mode.
		BorderMode borderMode;
		/// @brief All character definitions.
		hmap<unsigned int, CharacterDefinition*> characters;
		/// @brief All border character definitions.
		hmap<unsigned int, harray<BorderCharacterDefinition*> > borderCharacters;
		/// @brief All icon definitions.
		hmap<hstr, IconDefinition*> icons;
		/// @brief All border icon definitions.
		hmap<hstr, harray<BorderIconDefinition*> > borderIcons;
		/// @brief Texture containers for all loaded characters and icons.
		harray<TextureContainer*> textureContainers;
		/// @brief Texture containers for all loaded border characters and border icons.
		harray<BorderTextureContainer*> borderTextureContainers;

		/// @brief Gets the texture container for a given border thickness.
		/// @param[in] borderThickness border thickness.
		/// @return The texture container.
		harray<BorderTextureContainer*> _getBorderTextureContainers(float borderThickness) const;

		/// @brief Loads the font definition.
		/// @return True if successfully loaded.
		virtual bool _load();
		
		/// @brief Reads a basic parameter from an external font definition file.
		/// @param[in] line A line in the definition.
		/// @return True if reading and processing was successful.
		bool _readBasicParameter(chstr line);
		/// @brief Sets the border rendering mode.
		/// @param[in] value The border rendering mode.
		/// @note This actually sets the border rendering mode.
		/// @see setBorderMode()
		void _setBorderMode(const BorderMode& value);

		/// @brief Applies the cut-off to the rendering rect and area.
		/// @param[in] rect Container rect for text (used for clipping).
		/// @param[in] area Rect where text should be rendered.
		/// @param[in] symbolRect Rect of symbol used.
		/// @param[in] offsetY Vertical offset.
		void _applyCutoff(cgrect rect, cgrect area, cgrect symbolRect, float offsetY = 0.0f) const;
		
	};

}
#endif

