/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include "common.h"

class QDomElement;
class QStringList;

namespace LC::Aggregator
{
	struct Enclosure;

	struct Item;
	using Item_ptr = std::shared_ptr<Item>;
}

namespace LC::Aggregator::Parsers
{
	Item_ptr ParseCommonItem (const QDomElement& entry, IDType_t channelId);

	std::optional<QString> GetFirstNodeText (const QDomElement& parent,
			const QString& ns,
			const QString& nodeName);

	QString GetLink (const QDomElement&);

	QDomElement GetBestDescription (const QDomElement&, const QStringList&);

	QStringList GetAllCategories (const QDomElement&);

	QString GetAuthor (const QDomElement&);

	int GetNumComments (const QDomElement&);
	QString GetCommentsRSS (const QDomElement&);
	QString GetCommentsLink (const QDomElement&);

	QPair<double, double> GetGeoPoint (const QDomElement&);

	QDateTime GetDCDateTime (const QDomElement&);

	QString GetITunesDuration (const QDomElement&);

	QList<Enclosure> GetEncEnclosures (const QDomElement& entry, IDType_t itemId);

	Q_DECL_EXPORT QString UnescapeHTML (QString&&);
}

namespace LC::Aggregator::Parsers::Atom
{
	QString ParseEscapeAware (const QDomElement&);

	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId);
}

namespace LC::Aggregator::Parsers::RSS
{
	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId);
}
