/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/RenderSystem.h>

#include "Utility.h"

namespace atres
{
	CharacterDefinition::CharacterDefinition() : x(0.0f), y(0.0f), w(0.0f), h(0.0f), bx(0.0f), aw(0.0f)
	{
	}

	static int ind = 0;
	RenderSequence::RenderSequence() : texture(NULL), vertexes(NULL), vertexesSize(0)
	{
		ind++;
		this->index = ind;
		//printf("    --- CONSTRUCT %d!\n", this->index);
	}

	RenderSequence::RenderSequence(const RenderSequence& other) : texture(NULL), vertexes(NULL), vertexesSize(0)
	{
		ind++;
		this->index = ind;
		//printf("    --- COPY %d!\n", this->index);
		this->texture = other.texture;
		this->color = other.color;
		this->vertexes = NULL;
		this->vertexesSize = other.vertexesSize;
		if (other.vertexes != NULL)
		{
			this->vertexes = new april::TexturedVertex[this->vertexesSize];
			for_iter (i, 0, this->vertexesSize)
			{
				this->vertexes[i] = other.vertexes[i];
			}
		}
	}
	
	RenderSequence& RenderSequence::operator=(const RenderSequence& other)
	{
		ind++;
		this->index = ind;
		//printf("    --- OPERATOR %d!\n", this->index);
		this->texture = other.texture;
		this->color = other.color;
		this->vertexes = NULL;
		this->vertexesSize = other.vertexesSize;
		if (other.vertexes != NULL)
		{
			this->vertexes = new april::TexturedVertex[this->vertexesSize];
			memcpy(this->vertexes, other.vertexes, this->vertexesSize * sizeof(april::TexturedVertex));
		}
		return *this;
	}
	
	RenderSequence::~RenderSequence()
	{
		//printf("    --- DESTRUCT %d!\n", this->index);
		if (this->vertexes != NULL)
		{
			//printf("    --- DESTROY %d!\n", this->index);
			delete [] this->vertexes;
			this->vertexes = NULL;
		}
	}
	
	void RenderSequence::setRenderRectangles(harray<RenderRectangle> rectangles)
	{
		if (this->vertexes != NULL)
		{
			//printf("    --- DELETE RECT %d!\n", this->index);
			delete [] this->vertexes;
			this->vertexes = NULL;
		}
		this->vertexesSize = rectangles.size() * 6;
		if (this->vertexesSize == 0)
		{
			return;
		}
		this->vertexes = new april::TexturedVertex[this->vertexesSize];
		//printf("    --- CREATE RECT %d!\n", this->index);
		int i = 0;
		foreach (RenderRectangle, it, rectangles)
		{
			this->vertexes[i].x = (*it).dest.x;					this->vertexes[i].y = (*it).dest.y;					this->vertexes[i].u = (*it).src.x;					this->vertexes[i].v = (*it).src.y;					i++;
			this->vertexes[i].x = (*it).dest.x + (*it).dest.w;	this->vertexes[i].y = (*it).dest.y;					this->vertexes[i].u = (*it).src.x + (*it).src.w;	this->vertexes[i].v = (*it).src.y;					i++;
			this->vertexes[i].x = (*it).dest.x;					this->vertexes[i].y = (*it).dest.y + (*it).dest.h;	this->vertexes[i].u = (*it).src.x;					this->vertexes[i].v = (*it).src.y + (*it).src.h;	i++;
			this->vertexes[i].x = (*it).dest.x + (*it).dest.w;	this->vertexes[i].y = (*it).dest.y;					this->vertexes[i].u = (*it).src.x + (*it).src.w;	this->vertexes[i].v = (*it).src.y;					i++;
			this->vertexes[i].x = (*it).dest.x + (*it).dest.w;	this->vertexes[i].y = (*it).dest.y + (*it).dest.h;	this->vertexes[i].u = (*it).src.x + (*it).src.w;	this->vertexes[i].v = (*it).src.y + (*it).src.h;	i++;
			this->vertexes[i].x = (*it).dest.x;					this->vertexes[i].y = (*it).dest.y + (*it).dest.h;	this->vertexes[i].u = (*it).src.x;					this->vertexes[i].v = (*it).src.y + (*it).src.h;	i++;
		}
	}
	
	void RenderSequence::addVertexes(harray<april::TexturedVertex*> vertexesCollections, harray<int> vertexesSizes)
	{
		if (vertexesCollections.size() == 0)
		{
			return;
		}
		if (this->vertexes != NULL)
		{
			vertexesCollections.push_first(this->vertexes);
			vertexesSizes.push_first(this->vertexesSize);
		}
		int newVertexesSize = 0;
		foreach (int, it, vertexesSizes)
		{
			newVertexesSize += (*it);
		}
		april::TexturedVertex* newVertexes = new april::TexturedVertex[newVertexesSize];
		int c = 0;
		for_iter (i, 0, vertexesCollections.size())
		{
			memcpy(newVertexes + c, vertexesCollections[i], vertexesSizes[i] * sizeof(april::TexturedVertex));
			c += vertexesSizes[i];
		}
		if (this->vertexes != NULL)
		{
			//printf("    --- DELETE VERT %d!\n", this->index);
			delete [] this->vertexes;
			this->vertexes = NULL;
		}
		//printf("    --- ASSIGN VERT %d!\n", this->index);
		this->vertexes = newVertexes;
		this->vertexesSize = newVertexesSize;
	}

	RenderWord::RenderWord() : start(0), spaces(0), fullWidth(0.0f)
	{
	}
	
	RenderLine::RenderLine() : spaces(0), terminated(false)
	{
	}
	
	FormatTag::FormatTag() : type(ESCAPE), start(0), count(0)
	{
	}

	CacheKeySequence::CacheKeySequence() : horizontal(CENTER_WRAPPED), vertical(CENTER)
	{
	}

	void CacheKeySequence::set(hstr text, hstr fontName, grect rect, Alignment horizontal, Alignment vertical, april::Color color, gvec2 offset)
	{
		this->text = text;
		this->fontName = fontName;
		this->rect = rect;
		this->horizontal = horizontal;
		this->vertical = vertical;
		this->color = color;
		this->offset = offset;
	}

	bool CacheKeySequence::operator==(const CacheKeySequence& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->rect == other.rect &&
			this->horizontal == other.horizontal &&
			this->vertical == other.vertical &&
			this->color == other.color &&
			this->offset == other.offset);
	}
	
	void CacheKeyLine::set(hstr text, hstr fontName, gvec2 size)
	{
		this->text = text;
		this->fontName = fontName;
		this->size = size;
	}

	bool CacheKeyLine::operator==(const CacheKeyLine& other) const
	{
		return (this->text == other.text &&
			this->fontName == other.fontName &&
			this->size == other.size);
	}
	
}
