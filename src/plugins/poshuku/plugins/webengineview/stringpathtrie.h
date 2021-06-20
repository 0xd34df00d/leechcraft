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

namespace LC::Poshuku::WebEngineView
{
	template<typename V>
	class StringPathTrie
	{
		std::optional<V> Value_;

		// TODO C++20 use transparent hashes and unordered_map
		QHash<QString, StringPathTrie> Children_;
	public:
		void Mark (const QVector<QStringRef>& path, V value)
		{
			Mark (path.begin (), path.end (), std::move (value));
		}

		const std::optional<V>& BestMatch (const QVector<QStringRef>& path) const
		{
			return BestMatch (path.begin (), path.end ());
		}
	private:
		template<typename It>
		void Mark (It begin, It end, V value)
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
			pos->Mark (begin + 1, end, std::move (value));
		}

		template<typename It>
		const std::optional<V>& BestMatch (It begin, It end) const
		{
			if (begin == end)
				return Value_ ? Value_ : WalkArbitraryChild ();

			const auto& strRef = QString::fromRawData (begin->constData (), begin->size ());
			const auto pos = Children_.find (strRef);
			return pos == Children_.end () ?
					Value_ :
					pos->BestMatch (begin + 1, end);
		}

		const std::optional<V>& WalkArbitraryChild () const
		{
			if (Value_)
				return Value_;

			if (Children_.isEmpty ())
			{
				qCritical () << Q_FUNC_INFO << "empty children";
				throw std::runtime_error { "empty children" };
			}

			return Children_.begin ()->WalkArbitraryChild ();
		}
	};
}
