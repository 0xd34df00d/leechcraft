/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <algorithm>
#include <QHash>

namespace LC
{
namespace Util
{
	namespace CacheStrat
	{
		class LRU
		{
			size_t Current_ = 0;
		public:
			struct ValueAddon
			{
				size_t LastAccess_ = 0;

				ValueAddon () = default;

				ValueAddon (size_t la)
				: LastAccess_ { la }
				{
				}
			};

			ValueAddon CreateInfo ()
			{
				return { ++Current_ };
			}

			void Clear ()
			{
				Current_ = 0;
			}

			void Touch (ValueAddon& add)
			{
				add.LastAccess_ = ++Current_;
			}
		};

		inline bool operator< (const LRU::ValueAddon& v1, const LRU::ValueAddon& v2)
		{
			return v1.LastAccess_ < v2.LastAccess_;
		}
	}

	template<typename K, typename V, typename CS = CacheStrat::LRU>
	class AssocCache
	{
		struct ValueHolder
		{
			V V_;
			size_t Cost_;
			typename CS::ValueAddon CacheInfo_;
		};

		QHash<K, ValueHolder> Hash_;

		size_t CurrentCost_ = 0;
		const size_t MaxCost_;

		CS CacheStratState_;
	public:
		AssocCache (size_t maxCost)
		: MaxCost_ { maxCost }
		{
		}

		size_t size () const;
		void clear ();
		bool contains (const K&) const;

		V& operator[] (const K&);
	private:
		void CheckShrink ();
	};

	template<typename K, typename V, typename CS>
	size_t AssocCache<K, V, CS>::size () const
	{
		return Hash_.size ();
	}

	template<typename K, typename V, typename CS>
	void AssocCache<K, V, CS>::clear ()
	{
		Hash_.clear ();
		CacheStratState_.Clear ();
	}

	template<typename K, typename V, typename CS>
	bool AssocCache<K, V, CS>::contains (const K& k) const
	{
		return Hash_.contains (k);
	}

	template<typename K, typename V, typename CS>
	V& AssocCache<K, V, CS>::operator[] (const K& key)
	{
		if (!Hash_.contains (key))
		{
			Hash_.insert (key, { {}, 1, CacheStratState_.CreateInfo () });
			++CurrentCost_;

			CheckShrink ();
		}
		else
			CacheStratState_.Touch (Hash_ [key].CacheInfo_);

		return Hash_ [key].V_;
	}

	template<typename K, typename V, typename CS>
	void AssocCache<K, V, CS>::CheckShrink ()
	{
		while (CurrentCost_ > MaxCost_)
		{
			const auto pos = std::min_element (Hash_.begin (), Hash_.end (),
					[] (const ValueHolder& left, const ValueHolder& right)
						{ return left.CacheInfo_ < right.CacheInfo_; });
			CurrentCost_ -= pos->Cost_;
			Hash_.erase (pos);
		}
	}
}
}
