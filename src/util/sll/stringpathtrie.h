/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <optional>
#include <QHash>
#include <QString>
#include <QtDebug>

namespace LC::Util
{
	template<typename V>
	class StringPathTrie
	{
		std::optional<V> Value_;

		// TODO C++20 use transparent hashes and unordered_map
		QHash<QString, StringPathTrie> Children_;
	public:
		void Add (const QVector<QStringRef>& path, V value)
		{
			Add (path.begin (), path.end (), std::move (value));
		}

		struct FindResult
		{
			std::optional<V> Value_;
			std::ptrdiff_t Remaining_ = 0;

			bool operator<=> (const FindResult&) const = default;
		};

		FindResult Find (const QVector<QStringRef>& path) const
		{
			return Find (path.begin (), path.end ());
		}
	private:
		template<typename It>
		void Add (It begin, It end, V value)
		{
			if (begin == end)
			{
				Value_ = std::move (value);
				return;
			}

			const auto& strRef = QString::fromRawData (begin->constData (), begin->size ());
			auto pos = Children_.find (strRef);
			if (pos == Children_.end ())
				pos = Children_.insert (begin->toString (), {});
			pos->Add (begin + 1, end, std::move (value));
		}

		template<typename It>
		FindResult Find (It begin, It end) const
		{
			if (begin == end)
				return { Value_, 0 };

			const auto& strRef = QString::fromRawData (begin->constData (), begin->size ());
			const auto pos = Children_.find (strRef);
			return pos == Children_.end () ?
					FindResult { Value_, end - begin } :
					pos->Find (begin + 1, end);
		}
	};
}
