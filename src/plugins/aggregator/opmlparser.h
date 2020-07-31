/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QHash>
#include <QString>
#include <QDomDocument>
#include <util/sll/eitherfwd.h>
#include "opmlitem.h"

class QDomElement;

namespace LC
{
namespace Aggregator
{
	class OPMLParser
	{
	public:
		typedef QList<OPMLItem> items_container_t;
		typedef QHash<QString, QString> OPMLinfo_t;
	private:
		items_container_t Items_;
		bool CacheValid_ = false;
		const QDomDocument Document_;
	public:
		explicit OPMLParser (const QDomDocument&);

		bool IsValid ();
		OPMLinfo_t GetInfo ();
		items_container_t Parse ();
	private:
		void ParseOutline (const QDomElement&, QStringList = {});
	};

	using OPMLParseResult_t = Util::Either<QString, OPMLParser>;
	OPMLParseResult_t ParseOPML (const QString& filename);

	using OPMLItemsResult_t = Util::Either<QString, QList<OPMLItem>>;
	OPMLItemsResult_t ParseOPMLItems (const QString& filename);
}
}
