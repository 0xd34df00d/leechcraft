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
#include <util/sll/eitherfwd.h>
#include "types.h"

namespace LC::Aggregator
{
	using OPMLInfo = QHash<QString, QString>;

	struct OPMLParseResult
	{
		OPMLInfo Info_;
		QList<OPMLItem> Items_;
	};

	Util::Either<QString, OPMLParseResult> ParseOPML (const QString& filename);
}
