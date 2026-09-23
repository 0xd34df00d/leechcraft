/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QStringView>
#include "channel.h"

class QDomDocument;

namespace LC::Aggregator::Parsers
{
	bool IsRss20Root (QStringView rootName);
	bool IsRss10Root (QStringView rootName);

	/** @brief Parses an RSS 2.0 document, that is, anything with an `<rss>` root.
	 *
	 * RSS 2.0 is forward-compatible with the 0.9x, so this parser covers those
	 * and versionless documents as well. RSS 1.0 is a different, RDF-based
	 * format, parsed by `Rss10()`.
	 *
	 * @sa Rss10
	 */
	std::optional<channels_container_t> Rss20 (const QDomDocument& doc, IDType_t feedId);

	/** @brief Parses RSS 1.0 RDF-based format.
	 *
	 * @sa Rss20
	 */
	std::optional<channels_container_t> Rss10 (const QDomDocument& doc, IDType_t feedId);
}
