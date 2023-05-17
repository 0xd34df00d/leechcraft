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

namespace LC::Aggregator
{
	using TagAttrs = QVector<std::pair<QString, QString>>;

	struct Tag;

	using Node = std::variant<Tag, QString>;
	using Nodes = QVector<Node>;

	Nodes operator+ (Node&&, Nodes&&);
	Nodes operator+ (Nodes&&, Node&&);
	Nodes operator+ (Node&&, Node&&);

	struct Tag
	{
		QString Name_;
		TagAttrs Attrs_ {};

		Nodes Children_ {};

		static Tag WithText (const QString& name, const QString& contents);

		[[nodiscard]] QString ToHtml () const;
	};

	namespace Tags
	{
		extern const Tag Br;

		Tag Image (const QString& url);
		Tag Li (Nodes&& children);
		Tag Ul (Nodes&& children);
	}
}
