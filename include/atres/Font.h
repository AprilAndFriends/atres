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
	class atresExport Font
	{
	public:
		HL_ENUM_CLASS_PREFIX_DECLARE(atresExport, BorderMode,
		(
			HL_ENUM_DECLARE(BorderMode, Software);
			HL_ENUM_DECLARE(BorderMode, FontNative);
			HL_ENUM_DECLARE(BorderMode, PrerenderSquare);
			HL_ENUM_DECLARE(BorderMode, PrerenderCircle);
			HL_ENUM_DECLARE(BorderMode, PrerenderDiamond);
		));

		Font(chstr name);
		virtual ~Font();

		bool load();

		HL_DEFINE_GET(hstr, name, Name);
		float getHeight() const;
		float getScale() const;
		HL_DEFINE_SET(float, scale, Scale);
		HL_DEFINE_GETSET(float, baseScale, BaseScale);
		float getLineHeight() const;
		float getDescender() const;
		float getInternalDescender() const;
		float getStrikeThroughOffset() const;
		float getUnderlineOffset() const;
		HL_DEFINE_GET(BorderMode, borderMode, BorderMode);
		virtual void setBorderMode(BorderMode value);
		HL_DEFINE_IS(loaded, Loaded);
		inline hmap<unsigned int, CharacterDefinition*>& getCharacters() { return this->characters; }
		inline hmap<unsigned int, harray<BorderCharacterDefinition*> >& getBorderCharacters() { return this->borderCharacters; }
		inline hmap<hstr, IconDefinition*>& getIcons() { return this->icons; }
		inline hmap<hstr, harray<BorderIconDefinition*> >& getBorderIcons() { return this->borderIcons; }
		harray<april::Texture*> getTextures() const;

		virtual april::Texture* getTexture(unsigned int charCode);
		virtual april::Texture* getBorderTexture(unsigned int charCode, float borderThickness);
		virtual april::Texture* getTexture(chstr iconName);
		virtual april::Texture* getBorderTexture(chstr iconName, float borderThickness);
		virtual bool hasCharacter(unsigned int charCode);
		virtual bool hasBorderCharacter(unsigned int charCode, float borderThickness);
		virtual bool hasIcon(chstr iconName);
		virtual bool hasBorderIcon(chstr iconName, float borderThickness);
		BorderCharacterDefinition* getBorderCharacter(unsigned int charCode, float borderThickness);
		BorderIconDefinition* getBorderIcon(chstr iconName, float borderThickness);

		/// @note Not thread-safe!
		float getTextWidth(chstr text);
		/// @note Not thread-safe!
		int getTextCount(chstr text, float maxWidth);
		virtual float getKerning(unsigned int previousCode, unsigned int code);

		/// @note Not thread-safe!
		RenderRectangle makeRenderRectangle(cgrect rect, cgrect area, unsigned int charCode);
		RenderRectangle makeBorderRenderRectangle(cgrect rect, cgrect area, unsigned int charCode, float borderThickness);
		RenderRectangle makeRenderRectangle(cgrect rect, cgrect area, chstr iconName);
		RenderRectangle makeBorderRenderRectangle(cgrect rect, cgrect area, chstr iconName, float borderThickness);

		static BorderMode defaultBorderMode;

	protected:
		hstr name;
		float height;
		float scale;
		float baseScale;
		float lineHeight;
		float descender;
		float internalDescender;
		float strikeThroughOffset;
		float underlineOffset;
		bool loaded;
		bool nativeBorderSupported;
		BorderMode borderMode;
		hmap<unsigned int, CharacterDefinition*> characters;
		hmap<unsigned int, harray<BorderCharacterDefinition*> > borderCharacters;
		hmap<hstr, IconDefinition*> icons;
		hmap<hstr, harray<BorderIconDefinition*> > borderIcons;
		harray<TextureContainer*> textureContainers;
		harray<BorderTextureContainer*> borderTextureContainers;

		harray<BorderTextureContainer*> _getBorderTextureContainers(float borderThickness) const;

		virtual bool _load();
		
		bool _readBasicParameter(chstr line);
		void _setBorderMode(BorderMode value);

		void _applyCutoff(cgrect rect, cgrect area, cgrect symbolRect, float offsetY = 0.0f) const;
		
	};

}

#endif

