/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QList>

namespace LC
{
namespace GmailNotifier
{
	struct ConvInfo
	{
		QString Title_;
		QString Summary_;
		QUrl Link_;

		QDateTime Issued_;
		QDateTime Modified_;

		QString AuthorName_;
		QString AuthorEmail_;
	};

	bool operator== (const ConvInfo&, const ConvInfo&);

	typedef QList<ConvInfo> ConvInfos_t;
}
}
