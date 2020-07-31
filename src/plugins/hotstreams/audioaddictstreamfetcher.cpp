/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audioaddictstreamfetcher.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <util/sll/parsejson.h>
#include <util/sll/unreachable.h>

namespace LC
{
namespace HotStreams
{
	namespace
	{
		const QString APIUsername = "ephemeron";
		const QString APIPass = "dayeiph0ne@pp";

		QString Service2ID (AudioAddictStreamFetcher::Service service)
		{
			switch (service)
			{
			case AudioAddictStreamFetcher::Service::DI:
				return "di";
			case AudioAddictStreamFetcher::Service::SkyFM:
				return "sky";
			}

			Util::Unreachable ();
		}
	}

	AudioAddictStreamFetcher::AudioAddictStreamFetcher (Service service,
			QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (root, nam, parent)
	, Service_ (service)
	{
		const auto& abbr = Service2ID (service);

		const auto& urlStr = QString ("http://api.audioaddict.com/v1/%1/mobile/batch_update?asset_group_key=mobile_icons&stream_set_key=").arg (abbr);
		QNetworkRequest req { QUrl { urlStr } };
		req.setRawHeader("Authorization",
				"Basic " + QString ("%1:%2")
					.arg (APIUsername)
					.arg (APIPass)
					.toLatin1 ()
					.toBase64 ());
		Request (req);
	}

	QList<StreamListFetcherBase::StreamInfo> AudioAddictStreamFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		const auto& map = Util::ParseJson (data, Q_FUNC_INFO).toMap ();

		if (!map.contains ("channel_filters"))
		{
			qWarning () << Q_FUNC_INFO
					<< "no 'channel_filters' key in the reply, but we have:"
					<< map.keys ();
			return result;
		}

		for (const auto& filterVar : map ["channel_filters"].toList ())
		{
			const auto& filter = filterVar.toMap ();
			if (filter ["name"].toString () != "All")
				continue;

			for (const auto& channelVar : filter.value ("channels").toList ())
			{
				const auto& channel = channelVar.toMap ();

				const auto& key = channel ["key"].toString ();

				const QUrl url (QString ("http://listen.%1.fm/public3/%2.pls")
							.arg (Service2ID (Service_))
							.arg (key));
				const StreamInfo info
				{
					channel ["name"].toString (),
					channel ["description"].toString (),
					QStringList (),
					url,
					channel ["asset_url"].toString (),
					channel ["channel_director"].toString (),
					"pls"
				};
				result << info;
			}

			break;
		}

		return result;
	}
}
}
