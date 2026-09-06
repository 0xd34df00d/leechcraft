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
		QString Service2Domain (AudioAddictStreamFetcher::Service service)
		{
			switch (service)
			{
			case AudioAddictStreamFetcher::Service::DI:
				return "di.fm";
			case AudioAddictStreamFetcher::Service::RadioTunes:
				return "radiotunes.com";
			case AudioAddictStreamFetcher::Service::RockRadio:
				return "rockradio.com";
			}

			Util::Unreachable ();
		}
	}

	AudioAddictStreamFetcher::AudioAddictStreamFetcher (Service service,
			QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (root, nam, parent)
	{
		const auto& urlStr = QString ("http://listen.%1/public3/").arg (Service2Domain (service));
		Request (QNetworkRequest { QUrl { urlStr } });
	}

	QList<StreamListFetcherBase::StreamInfo> AudioAddictStreamFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		for (const auto& var : Util::ParseJson (data, Q_FUNC_INFO).toList ())
		{
			const auto& map = var.toMap ();

			result << StreamInfo
			{
				map ["name"].toString (),
				map ["description"].toString (),
				QStringList (),
				QUrl (map ["playlist"].toByteArray ()),
				QUrl (),
				QString (),
				"pls"
			};
		}

		return result;
	}
}
}
