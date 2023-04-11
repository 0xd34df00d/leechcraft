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

	struct Tag
	{
		QString Name_;
		TagAttrs Attrs_;

		Nodes Children_;

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

	inline Nodes operator+ (Node&& node, Nodes&& nodes)
	{
		nodes.prepend (std::move (node));
		return nodes;
	}

	inline Nodes operator+ (Nodes&& nodes, Node&& node)
	{
		nodes.push_back (std::move (node));
		return nodes;
	}

	inline Nodes operator+ (Node&& n1, Node&& n2)
	{
		return { std::move (n1), std::move (n2) };
	}
}
