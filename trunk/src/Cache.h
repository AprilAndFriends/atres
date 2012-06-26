/// @file
/// @author  Boris Mikic
/// @version 2.5
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

#include <hltypes/hlist.h>
#include <hltypes/hmap.h>

#include "atres.h"
#include "atresExport.h"

namespace atres
{
	/// @note The classes uses a hash value to store objects because it would require the implementation of the
	/// comparison operators if the *Entry objects would be used as keys. This is simply an alternate way to handle
	/// things. Even though it appears unnecessary and hacky, there is no better way.
	template <class T>
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

		void add(T& entry)
		{
			unsigned int hash = entry.hash();
			if (!this->data.has_key(hash))
			{
				this->data[hash] = harray<T>();
			}
			this->data[hash] += entry;
			this->entries += this->data[hash].last();
		}

		bool get(T& entry)
		{
			unsigned int hash = entry.hash();
			if (this->data.has_key(hash))
			{
				foreach (T, it, this->data[hash])
				{
					if (entry == (*it))
					{
						entry.value = (*it).value;
						return true;
					}
				}
			}
			return false;
		}

		void removeEntry(const T& entry)
		{
			unsigned int hash = entry.hash();
			if (this->data.has_key(hash))
			{
				if (this->data[hash].size() == 1)
				{
					this->data.remove_key(hash);
				}
				else
				{
					this->data[hash].remove(entry);
				}
				this->entries.remove(entry);
			}
		}

		void clear()
		{
			this->data.clear();
		}

		int size()
		{
			return this->data.size();
		}

		void update()
		{
			if (this->maxSize >= 0)
			{
				int overSize = this->data.size() - this->maxSize;
				if (overSize > 0)
				{
					hlist<T> removed = this->entries(0, overSize);
					foreach_l (T, it, removed)
					{
						this->removeEntry(*it);
					}
				}
			}
		}

	protected:
		int maxSize;
		hmap<unsigned int, harray<T> > data;
		hlist<T> entries;
		
	};

}

#endif

