/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/an/ianemitter.h>

namespace LC::AdvancedNotifications
{
	using ValueMatcherOrData = std::variant<AN::ValueMatcher, QVariantMap>;

	struct FieldMatch
	{
		QString PluginID_ {};
		QString Name_;

		QMetaType::Type Type_ = QMetaType::UnknownType;

		ValueMatcherOrData Matcher_;

		bool operator== (const FieldMatch&) const = default;

		void Save (QDataStream&) const;

		[[nodiscard]] static std::optional<FieldMatch> Load (QDataStream&);
	};

	using FieldMatches_t = QList<FieldMatch>;
}
