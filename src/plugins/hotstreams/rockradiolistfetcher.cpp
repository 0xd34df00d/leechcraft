/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rockradiolistfetcher.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <util/sll/parsejson.h>

namespace LC
{
namespace HotStreams
{
	RockRadioListFetcher::RockRadioListFetcher (QStandardItem *item, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (item, nam, parent)
	{
		Request (QNetworkRequest (QUrl ("http://listen.rockradio.com/public3/")));
	}

	QList<StreamListFetcherBase::StreamInfo> RockRadioListFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		const auto& map = Util::ParseJson (data, Q_FUNC_INFO);

		for (const auto& var : map.toList ())
		{
			const auto& map = var.toMap ();

			const auto& key = map ["key"].toByteArray ();
			StreamInfo info =
			{
				map ["name"].toString (),
				map ["description"].toString (),
				QStringList (),
				QUrl (map ["playlist"].toByteArray ()),
				QUrl ("http://www.rockradio.com/images/channels/" + key + ".jpg"),
				QString (),
				"pls"
			};

			result << info;
		}

		return result;
	}
}
}
