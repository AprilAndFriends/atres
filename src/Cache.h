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
/// Defines a cache object.

#ifndef ATRES_CACHE_H
#define ATRES_CACHE_H

#include <hltypes/harray.h>
#include <hltypes/hlist.h>
#include <hltypes/hmap.h>

#include "atres.h"
#include "atresExport.h"

namespace atres
{
	/// @brief Special object that caches calculated text entries.
	/// @note The classes uses a hash value to store objects because it would require the implementation of the
	/// comparison operators if the Entry objects would be used as keys. This is simply an alternate way to handle
	/// things. Even though it appears unnecessary and hacky, there is no better way.
	template <typename T>
	class Cache
	{
		/// @brief Alias for simpler code.
		typedef typename std::vector<T>::iterator iterator_t;
		/// @brief Alias for simpler code.
		typedef typename std::list<T>::iterator list_iterator_t;
	public:
		/// @brief Constructor.
		inline Cache()
		{
			this->maxSize = 1000;
		}
		/// @brief Sets max size for cache.
		/// @param[in] value New max size.
		inline void setMaxSize(int value)
		{
			this->maxSize = value;
			this->update();
		}
		/// @brief Adds a cache entry.
		/// @param[in] entry The cache entry.
		inline void add(T& entry)
		{
			unsigned int hash = entry.hash();
			if (!this->data.hasKey(hash))
			{
				this->data[hash] = harray<T>();
			}
			this->data[hash] += entry;
			this->entries += this->data[hash].last();
		}
		/// @brief Gets a cache entry.
		/// @param[out] entry The output cache entry. Will only be filled with data if return is true.
		/// @return True if entry could be found.
		inline bool get(T& entry)
		{
			unsigned int hash = entry.hash();
			if (this->data.hasKey(hash))
			{
				harray<T>& array = this->data[hash];
				for (iterator_t it = array.begin(); it != array.end(); ++it)
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
		/// @brief Removes a cache entry.
		/// @param[in] entry The cache entry.
		inline void removeEntry(const T& entry)
		{
			unsigned int hash = entry.hash();
			if (this->data.hasKey(hash))
			{
				if (this->data[hash].size() == 1)
				{
					this->data.removeKey(hash);
				}
				else
				{
					this->data[hash].remove(entry);
				}
				this->entries.remove(entry);
			}
		}
		/// @brief Clears cache.
		inline void clear()
		{
			this->data.clear();
			this->entries.clear();
		}
		/// @brief Gets the current size of the cache.
		/// @return The current size of the cache.
		inline int getSize() const
		{
			return this->data.size();
		}
		/// @brief Updates all cache entries.
		inline void update()
		{
			if (this->maxSize >= 0)
			{
				int overSize = this->data.size() - this->maxSize;
				if (overSize > 0)
				{
					hlist<T> removed = this->entries(0, overSize);
					for (list_iterator_t it = removed.begin(); it != removed.end(); ++it)
					{
						this->removeEntry(*it);
					}
				}
			}
		}
		
	protected:
		/// @brief Max size of the cache.
		int maxSize;
		/// @brief The cache entries.
		hmap<unsigned int, harray<T> > data;
		/// @brief A list of all hashes.
		/// @note Using hlist because add/remove has a constant complexity while harray would have to reorder/resize all elements.
		hlist<T> entries;
		
	};
	
}
#endif

