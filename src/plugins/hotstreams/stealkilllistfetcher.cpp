/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012 Maxim Koltsov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stealkilllistfetcher.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>

namespace LC
{
namespace HotStreams
{
StealKillListFetcher::StealKillListFetcher (QStandardItem *item, QNetworkAccessManager *nam, QObject *parent)
: StreamListFetcherBase (item, nam, parent)
{
	Request (QNetworkRequest (QUrl ("http://listen.42fm.ru:8000/")));
}

QList<StreamListFetcherBase::StreamInfo> StealKillListFetcher::Parse (const QByteArray& /*data*/)
{
	QList<StreamInfo> result;
	QStringList genres;
	genres << "Rock" << "Metal" << "Other";

	StreamInfo mp3Info =
	{
		QString::fromUtf8 ("Радио «42fm» (MP3)"),
		QString::fromUtf8 ("Классика рока и отличное настроение"),
		genres,
		QUrl ("http://listen.42fm.ru:8000/stealkill.m3u"),
		QUrl (),
		QString (),
		"m3u"
	};

	StreamInfo oggInfo =
	{
		QString::fromUtf8 ("Радио «42fm» (OGG)"),
		QString::fromUtf8 ("Классика рока и отличное настроение"),
		genres,
		QUrl ("http://listen.42fm.ru:8000/stealkill-8.0.ogg.m3u"),
		QUrl (),
		QString (),
		"m3u"
	};

	result << mp3Info << oggInfo;

	return result;
}

}
}
