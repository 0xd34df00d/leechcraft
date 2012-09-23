/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2012 Maxim Koltsov
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

#include "stealkilllistfetcher.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>

namespace LeechCraft
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
