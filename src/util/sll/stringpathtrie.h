/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <type_traits>
#include <QHash>
#include <QString>
#include <QStringView>
#include <QtDebug>

namespace LC::Util
{
	template<typename Cont>
	concept StringViewContainer = std::is_same_v<typename std::decay_t<Cont>::value_type, QStringView>;

	template<typename V>
	class StringPathTrie
	{
		std::optional<V> Value_;

		// TODO C++20 use transparent hashes and unordered_map
		QHash<QString, StringPathTrie> Children_;
	public:
		const std::optional<V>& GetValue () const
		{
			return Value_;
		}

		const StringPathTrie* GetChild (QStringView view) const
		{
			const auto pos = Children_.find (view.toString ());
			if (pos == Children_.end ())
				return nullptr;

			return &*pos;
		}

		template<StringViewContainer Cont>
		void Add (Cont&& path, V value)
		{
			Add (path.begin (), path.end (), std::move (value));
		}

		template<typename It>
		void Add (It begin, It end, V value)
		{
			if (begin == end)
			{
				Value_ = std::move (value);
				return;
			}

			const auto& strRef = begin->toString ();
			auto pos = Children_.find (strRef);
			if (pos == Children_.end ())
				pos = Children_.insert (strRef, {});
			pos->Add (begin + 1, end, std::move (value));
		}

		struct FindResult
		{
			std::optional<V> Value_;
			std::ptrdiff_t Remaining_ = 0;

			inline const static StringPathTrie NullTrie {};
			const StringPathTrie *Rest_ = &NullTrie;

			bool operator== (const FindResult& other) const
			{
				return Value_ == other.Value_ && Remaining_ == other.Remaining_;
			}
		};

		FindResult Find (QStringView single) const
		{
			std::initializer_list<QStringView> dummy { single };
			return Find (dummy.begin (), dummy.end ());
		}

		template<StringViewContainer Cont>
		FindResult Find (Cont&& path) const
		{
			return Find (path.begin (), path.end ());
		}

		template<typename It>
		FindResult Find (It begin, It end) const
		{
			return Find (begin, end, { Value_, end - begin, this });
		}
	private:
		template<typename It>
		FindResult Find (It begin, It end, FindResult lastGood) const
		{
			if (Value_)
				lastGood = { Value_, end - begin, this };

			if (begin == end)
				return lastGood;

			const auto& strRef = begin->toString ();
			const auto pos = Children_.find (strRef);
			if (pos == Children_.end ())
				return lastGood;

			return pos->Find (begin + 1, end, lastGood);
		}
	};
}
