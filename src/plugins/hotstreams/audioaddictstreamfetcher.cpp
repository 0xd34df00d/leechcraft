/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "audioaddictstreamfetcher.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <qjson/parser.h>

namespace LeechCraft
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

			qWarning () << Q_FUNC_INFO
					<< "unknown service"
					<< static_cast<int> (service);
			return QString ();
		}
	}

	AudioAddictStreamFetcher::AudioAddictStreamFetcher (Service service,
			QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (root, nam, parent)
	, Service_ (service)
	{
		const auto& abbr = Service2ID (service);
		if (abbr.isEmpty ())
		{
			deleteLater ();
			return;
		}

		const auto& urlStr = QString ("http://api.v2.audioaddict.com/v1/%1/mobile/batch_update?asset_group_key=mobile_icons&stream_set_key=").arg (abbr);
		auto req = QNetworkRequest (QUrl (urlStr));
		req.setRawHeader("Authorization",
				"Basic " + QString ("%1:%2")
					.arg (APIUsername)
					.arg (APIPass)
					.toAscii ()
					.toBase64 ());
		Request (req);
	}

	QList<StreamListFetcherBase::StreamInfo> AudioAddictStreamFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		const auto& map = QJson::Parser ().parse (data).toMap ();

		if (!map.contains ("channel_filters"))
			return result;

		Q_FOREACH (const auto& filterVar, map ["channel_filters"].toList ())
		{
			const auto& filter = filterVar.toMap ();
			if (filter ["name"].toString () != "All")
				continue;

			Q_FOREACH (const auto& channelVar, filter.value ("channels").toList ())
			{
				const auto& channel = channelVar.toMap ();

				const auto& key = channel ["key"].toString ();

				const QUrl url (QString ("http://listen.%1.fm/public3/%2.pls")
							.arg (Service2ID (Service_))
							.arg (key));
				StreamInfo info =
				{
					channel ["name"].toString (),
					channel ["description"].toString (),
					QStringList (),
					url,
					channel ["asset_url"].toString ()
				};
				result << info;
			}

			break;
		}

		return result;
	}
}
}
