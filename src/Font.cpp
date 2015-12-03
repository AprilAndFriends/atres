/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Font.h"

#define THICKNESS_TOLERANCE 0.01f

namespace atres
{
	static april::Texture* _map_texture(TextureContainer* container)
	{
		return container->texture;
	}

	HL_ENUM_CLASS_DEFINE(Font::BorderMode,
	(
		HL_ENUM_DEFINE(Font::BorderMode, Software);
		HL_ENUM_DEFINE(Font::BorderMode, FontNative);
		HL_ENUM_DEFINE(Font::BorderMode, PrerenderSquare);
		HL_ENUM_DEFINE(Font::BorderMode, PrerenderCircle);
		HL_ENUM_DEFINE(Font::BorderMode, PrerenderDiamond);
	));

	Font::Font(chstr name) : height(0.0f), scale(1.0f), baseScale(1.0f), lineHeight(0.0f), descender(0.0f), internalDescender(0.0f),
		loaded(false), nativeBorderSupported(false), borderMode(BorderMode::Software)
	{
		this->name = name;
	}

	Font::~Font()
	{
		foreach_map (unsigned int, CharacterDefinition*, it, this->characters)
		{
			delete it->second;
		}
		foreach_map (unsigned int, harray<BorderCharacterDefinition*>, it, this->borderCharacters)
		{
			foreach (BorderCharacterDefinition*, it2, it->second)
			{
				delete (*it2);
			}
		}
		foreach_map (hstr, IconDefinition*, it, this->icons)
		{
			delete it->second;
		}
		foreach_map (hstr, harray<BorderIconDefinition*>, it, this->borderIcons)
		{
			foreach (BorderIconDefinition*, it2, it->second)
			{
				delete (*it2);
			}
		}
		foreach (TextureContainer*, it, this->textureContainers)
		{
			delete (*it);
		}
		this->textureContainers.clear();
		foreach (BorderTextureContainer*, it, this->borderTextureContainers)
		{
			delete (*it);
		}
		this->borderTextureContainers.clear();
	}

	bool Font::load()
	{
		if (this->loaded)
		{
			return false;
		}
		this->loaded = this->_load();
		if (!this->loaded)
		{
			hlog::errorf(logTag, "Font '%s' could not be loaded!", this->name.cStr());
		}
		return this->loaded;
	}

	bool Font::_load()
	{
		return true;
	}

	bool Font::_readBasicParameter(chstr line)
	{
		if (line.startsWith("Name="))
		{
			this->name = line.replaced("Name=", "");
			return true;
		}
		if (line.startsWith("Height="))
		{
			this->height = (float)line.replaced("Height=", "");
			return true;
		}
		if (line.startsWith("Scale="))
		{
			this->baseScale = (float)line.replaced("Scale=", "");
			return true;
		}
		if (line.startsWith("LineHeight="))
		{
			this->lineHeight = (float)line.replaced("LineHeight=", "");
			return true;
		}
		if (line.startsWith("Descender="))
		{
			this->descender = (float)line.replaced("Descender=", "");
			return true;
		}
		return false;
	}
	
	float Font::getHeight()
	{
		return (this->height * this->scale * this->baseScale);
	}
	
	float Font::getScale()
	{
		return (this->scale * this->baseScale);
	}
	
	float Font::getLineHeight()
	{
		return (this->lineHeight * this->scale * this->baseScale);
	}

	float Font::getDescender()
	{
		return (this->descender * this->scale * this->baseScale);
	}

	float Font::getInternalDescender()
	{
		return (this->internalDescender * this->scale * this->baseScale);
	}

	void Font::setBorderMode(BorderMode value)
	{
		if (value != BorderMode::Software)
		{
			hlog::warnf(logTag, "BorderModes other than 'Software' are not supported in font '%s'.", this->name.cStr());
			return;
		}
		this->_setBorderMode(value);
	}

	void Font::_setBorderMode(BorderMode value)
	{
		if (this->borderMode != value)
		{
			this->borderMode = value;
			foreach (BorderTextureContainer*, it, this->borderTextureContainers)
			{
				delete (*it);
			}
			this->borderTextureContainers.clear();
		}
	}

	harray<april::Texture*> Font::getTextures()
	{
		return (this->textureContainers + this->borderTextureContainers.cast<TextureContainer*>()).mapped(&_map_texture);
	}
	
	april::Texture* Font::getTexture(unsigned int charCode)
	{
		foreach (TextureContainer*, it, this->textureContainers)
		{
			if ((*it)->characters.has(charCode))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	april::Texture* Font::getBorderTexture(unsigned int charCode, float borderThickness)
	{
		foreach (BorderTextureContainer*, it, this->borderTextureContainers)
		{
			if (heqf((*it)->borderThickness, borderThickness, THICKNESS_TOLERANCE) && (*it)->characters.has(charCode))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	april::Texture* Font::getTexture(chstr iconName)
	{
		foreach (TextureContainer*, it, this->textureContainers)
		{
			if ((*it)->icons.has(iconName))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	april::Texture* Font::getBorderTexture(chstr iconName, float borderThickness)
	{
		foreach (BorderTextureContainer*, it, this->borderTextureContainers)
		{
			if (heqf((*it)->borderThickness, borderThickness, THICKNESS_TOLERANCE) && (*it)->icons.has(iconName))
			{
				return (*it)->texture;
			}
		}
		return NULL;
	}

	bool Font::hasCharacter(unsigned int charCode)
	{
		return this->characters.hasKey(charCode);
	}

	bool Font::hasBorderCharacter(unsigned int charCode, float borderThickness)
	{
		return (this->getBorderCharacter(charCode, borderThickness) != NULL);
	}

	bool Font::hasIcon(chstr iconName)
	{
		return this->icons.hasKey(iconName);
	}

	bool Font::hasBorderIcon(chstr iconName, float borderThickness)
	{
		return (this->getBorderIcon(iconName, borderThickness) != NULL);
	}

	BorderCharacterDefinition* Font::getBorderCharacter(unsigned int charCode, float borderThickness)
	{
		harray<BorderCharacterDefinition*> definitions = this->borderCharacters[charCode];
		foreach (BorderCharacterDefinition*, it, definitions)
		{
			if (heqf((*it)->borderThickness, borderThickness, THICKNESS_TOLERANCE))
			{
				return (*it);
			}
		}
		return NULL;
	}

	BorderIconDefinition* Font::getBorderIcon(chstr iconName, float borderThickness)
	{
		harray<BorderIconDefinition*> definitions = this->borderIcons[iconName];
		foreach (BorderIconDefinition*, it, definitions)
		{
			if (heqf((*it)->borderThickness, borderThickness, THICKNESS_TOLERANCE))
			{
				return (*it);
			}
		}
		return NULL;
	}

	harray<BorderTextureContainer*> Font::_getBorderTextureContainers(float borderThickness)
	{
		harray<BorderTextureContainer*> result;
		foreach (BorderTextureContainer*, it, this->borderTextureContainers)
		{
			if (heqf((*it)->borderThickness, borderThickness, THICKNESS_TOLERANCE))
			{
				result += (*it);
			}
		}
		return result;
	}

	// using static definitions to avoid memory allocation for optimization
	static float _textX = 0.0f;
	static float _textW = 0.0f;
	static float _ax = 0.0f;
	static float _aw = 0.0f;
	static float _scale = 1.0f;
	static CharacterDefinition* _character = NULL;
	static std::basic_string<unsigned int> _chars;

	float Font::getTextWidth(chstr text)
	{
		// using static definitions to avoid memory allocation for optimization
		_textX = 0.0f;
		_textW = 0.0f;
		_ax = 0.0f;
		_aw = 0.0f;
		_scale = this->getScale();
		_chars = text.uStr();
		for_itert (unsigned int, i, 0, _chars.size())
		{
			_character = this->characters[_chars[i]];
			if (_textX < -_character->bearing.x * _scale)
			{
				_ax = (_character->advance - _character->bearing.x) * _scale;
				_aw = _character->rect.w * _scale;
			}
			else
			{
				_ax = _character->advance * _scale;
				_aw = (_character->rect.w + _character->bearing.x) * _scale;
			}
			_textW = _textX + hmax(_ax, _aw);
			_textX += _ax;
		}
		return _textW;
	}
	
	int Font::getTextCount(chstr text, float maxWidth)
	{
		_textX = 0.0f;
		_textW = 0.0f;
		_ax = 0.0f;
		_aw = 0.0f;
		_scale = this->getScale();
		_chars = text.uStr();
		for_itert (unsigned int, i, 0, _chars.size())
		{
			_character = this->characters[_chars[i]];
			if (_textX < -_character->bearing.x * _scale)
			{
				_ax = (_character->advance - _character->bearing.x) * _scale;
				_aw = _character->rect.w * _scale;
			}
			else
			{
				_ax = _character->advance * _scale;
				_aw = (_character->rect.w + _character->bearing.x) * _scale;
			}
			_textW = _textX + hmax(_ax, _aw);
			_textX += _ax;
			if (_textW > maxWidth)
			{
				return text.utf8SubString(0, i).size();
			}
		}
		return text.size();
	}
	
	// using static definitions to avoid memory allocation for optimization
	static RenderRectangle _result;
	static gvec2 _fullSize(1.0f, 1.0f);
	static gvec2 _leftTop;
	static gvec2 _rightBottom;
	static gvec2 _textureInvertedSize;
	static april::Texture* _texture = NULL;

	void Font::_applyCutoff(const grect& rect, const grect& area, const grect& symbolRect, float offsetY)
	{
		// vertical/horizontal cutoff of destination rectangle (using left/right/top/bottom semantics for consistency)
		_leftTop.x = (area.left() < rect.left() ? (area.right() - rect.left()) / area.w : _fullSize.x);
		_leftTop.y = (area.top() < rect.top() ? (area.bottom() - rect.top()) / area.h : _fullSize.y);
		_rightBottom.x = (rect.right() < area.right() ? (rect.right() - area.left()) / area.w : _fullSize.x);
		_rightBottom.y = (rect.bottom() < area.bottom() ? (rect.bottom() - area.top()) / area.h : _fullSize.y);
		// apply cutoff on destination
		_result.dest.setPosition(area.getPosition() + area.getSize() * (_fullSize - _leftTop));
		_result.dest.setSize(area.getSize() * (_leftTop + _rightBottom - _fullSize));
		// apply cutoff on source
		_result.src.setPosition((symbolRect.getPosition() + symbolRect.getSize() * (_fullSize - _leftTop)) * _textureInvertedSize);
		_result.src.y += offsetY;
		_result.src.setSize((symbolRect.getSize() * (_leftTop + _rightBottom - _fullSize)) * _textureInvertedSize);
	}

	RenderRectangle Font::makeRenderRectangle(const grect& rect, grect area, unsigned int charCode)
	{
		_result.src.set(0.0f, 0.0f, 0.0f, 0.0f);
		_result.dest = area;
		// if destination rectangle not entirely inside drawing area
		if (rect.intersects(_result.dest))
		{
			_texture = this->getTexture(charCode);
			_textureInvertedSize.set(1.0f / _texture->getWidth(), 1.0f / _texture->getHeight());
			this->_applyCutoff(rect, area, this->characters[charCode]->rect);
		}
		return _result;
	}

	RenderRectangle Font::makeBorderRenderRectangle(const grect& rect, grect area, unsigned int charCode, float borderThickness)
	{
		_result.src.set(0.0f, 0.0f, 0.0f, 0.0f);
		_result.dest = area;
		// if destination rectangle not entirely inside drawing area
		if (rect.intersects(_result.dest))
		{
			_texture = this->getBorderTexture(charCode, borderThickness);
			_textureInvertedSize.set(1.0f / _texture->getWidth(), 1.0f / _texture->getHeight());
			this->_applyCutoff(rect, area, this->getBorderCharacter(charCode, borderThickness)->rect);
		}
		return _result;
	}

	RenderRectangle Font::makeRenderRectangle(const grect& rect, grect area, chstr iconName)
	{
		_result.src.set(0.0f, 0.0f, 0.0f, 0.0f);
		_result.dest = area;
		// if destination rectangle not entirely inside drawing area
		if (rect.intersects(_result.dest))
		{
			_texture = this->getTexture(iconName);
			_textureInvertedSize.set(1.0f / _texture->getWidth(), 1.0f / _texture->getHeight());
			this->_applyCutoff(rect, area, this->icons[iconName]->rect);
		}
		return _result;
	}

	RenderRectangle Font::makeBorderRenderRectangle(const grect& rect, grect area, chstr iconName, float borderThickness)
	{
		_result.src.set(0.0f, 0.0f, 0.0f, 0.0f);
		_result.dest = area;
		// if destination rectangle not entirely inside drawing area
		if (rect.intersects(_result.dest))
		{
			_texture = this->getBorderTexture(iconName, borderThickness);
			_textureInvertedSize.set(1.0f / _texture->getWidth(), 1.0f / _texture->getHeight());
			this->_applyCutoff(rect, area, this->getBorderIcon(iconName, borderThickness)->rect);
		}
		return _result;
	}

}
