/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <variant>
#include <QString>
#include <QVector>
#include "sllconfig.h"

namespace LC::Util
{
	using TagAttrs = QVector<std::pair<QString, QString>>;

	struct Tag;

	using Node = std::variant<Tag, QString>;
	using Nodes = QVector<Node>;

	UTIL_SLL_API Nodes operator+ (Node&&, Nodes&&);
	UTIL_SLL_API Nodes operator+ (Nodes&&, Node&&);
	UTIL_SLL_API Nodes operator+ (Node&&, Node&&);

	struct Tag
	{
		QString Name_;
		TagAttrs Attrs_ {};

		Nodes Children_ {};

		UTIL_SLL_API static Tag WithText (const QString& name, const QString& contents);

		[[nodiscard]]
		UTIL_SLL_API QString ToHtml () const;
	};

	namespace Tags
	{
		extern const Tag Br;

		UTIL_SLL_API Tag Image (const QString& url);
		UTIL_SLL_API Tag Li (Nodes&& children);
		UTIL_SLL_API Tag Ul (Nodes&& children);
	}
}
