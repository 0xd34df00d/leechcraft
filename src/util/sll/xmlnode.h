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

class QSize;

namespace LC::Util
{
	using TagAttrs = QVector<std::pair<QByteArray, QString>>;

	struct Tag;

	struct NoNode {};

	using Node = std::variant<Tag, QString, NoNode>;
	using Nodes = QVector<Node>;

	UTIL_SLL_API Nodes operator+ (Node&&, Nodes&&);
	UTIL_SLL_API Nodes operator+ (Nodes&&, Node&&);
	UTIL_SLL_API Nodes operator+ (Node&&, Node&&);

	template<typename T>
	concept XmlRepr = std::is_same_v<T, QString> || std::is_same_v<T, QByteArray>;

	struct Tag
	{
		QByteArray Name_;
		TagAttrs Attrs_ {};

		Nodes Children_ {};

		UTIL_SLL_API static Tag WithText (const QByteArray& name, const QString& contents);
		UTIL_SLL_API static Node WithTextNonEmpty (const QByteArray& name, const QString& contents);

		template<XmlRepr T = QString>
		[[nodiscard]]
		UTIL_SLL_API T Serialize (T prefix = {}) const;

		UTIL_SLL_API Tag& WithAttr (QByteArray, QString) &&;
	};

	namespace Tags
	{
		extern const Tag Br;

		UTIL_SLL_API Tag Html (Nodes&& children);
		UTIL_SLL_API Tag Charset (const QString& charset);
		UTIL_SLL_API Tag Title (const QString& title);
		UTIL_SLL_API Tag Style (const QString& style);

		UTIL_SLL_API Tag Body (Nodes&& children);

		UTIL_SLL_API Tag Image (const QString& url);
		UTIL_SLL_API Tag Image (const QString& url, const QSize&);
		UTIL_SLL_API Tag Li (Nodes&& children);
		UTIL_SLL_API Tag Ul (Nodes&& children);

		UTIL_SLL_API Tag P (Nodes&& children);

		UTIL_SLL_API Nodes TableGrid (size_t rows, size_t cols, const std::function<Nodes (size_t, size_t)>& cell);
	}
}
