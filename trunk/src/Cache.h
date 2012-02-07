/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a cache object.

#ifndef ATRES_CACHE_H
#define ATRES_CACHE_H

#include <hltypes/harray.h>

#include "atres.h"
#include "atresExport.h"

namespace atres
{
	template <class K, class V>
	class Cache
	{
	public:
		Cache()
		{
			this->maxSize = 1000;
		}

		~Cache()
		{
		}

		void setMaxSize(int value)
		{
			this->maxSize = value;
			this->update();
		}

		void set(const K& key, const V& value)
		{
			int index = this->keys.index_of(key);
			if (index >= 0)
			{
				this->keys.remove_at(index);
				this->values.remove_at(index);
			}
			this->keys += key;
			this->values += value;
		}

		bool get(K& key, V* value = NULL) const
		{
			int index = this->keys.index_of(key);
			if (index >= 0)
			{
				if (value != NULL)
				{
					*value = this->values[index];
				}
				return true;
			}
			return false;
		}

		void removeKey(const K& key)
		{
			int index = this->keys.index_of(key);
			if (index >= 0)
			{
				this->keys.remove_at(index);
				this->values.remove_at(index);
			}
		}

		void removeValue(const V& value)
		{
			int index = this->values.index_of(value);
			if (index >= 0)
			{
				this->keys.remove_at(index);
				this->values.remove_at(index);
			}
		}

		void clear()
		{
			this->keys.clear();
			this->values.clear();
		}

		int size()
		{
			return this->keys.size();
		}

		void update()
		{
			if (this->maxSize >= 0)
			{
				int overSize = this->keys.size() - this->maxSize;
				if (overSize > 0)
				{
					this->keys.remove_at(this->maxSize, overSize);
					this->values.remove_at(this->maxSize, overSize);
				}
			}
		}

	protected:
		int maxSize;
		harray<K> keys;
		harray<V> values;
		
	};
}

#endif

