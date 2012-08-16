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

#include "somafmlistfetcher.h"
#include <algorithm>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <QDomDocument>
#include <QtDebug>

namespace LeechCraft
{
namespace HotStreams
{
	SomaFMListFetcher::SomaFMListFetcher (QStandardItem *item, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (item, nam, parent)
	{
		Request (QNetworkRequest (QUrl ("http://somafm.com/channels.xml")));
	}

	QList<StreamListFetcherBase::StreamInfo> SomaFMListFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			return result;
		}

		auto channel = doc.documentElement ().firstChildElement ("channel");
		while (!channel.isNull ())
		{
			QUrl url;

			auto pls = channel.firstChildElement ("fastpls");
			while (!pls.isNull ())
			{
				if (pls.attribute ("format") == "mp3")
				{
					url = pls.text ();
					break;
				}
				pls = pls.nextSiblingElement ("fastpls");
			}

			auto getText = [&channel] (const QString& name)
			{
				return channel.firstChildElement (name).text ();
			};

			StreamInfo info =
			{
				getText ("title"),
				getText ("description"),
				getText ("genre").split ('|', QString::SkipEmptyParts),
				url,
				QUrl (),
				getText ("dj") + " (" + getText ("djmail") + ")"
			};
			result << info;
			channel = channel.nextSiblingElement ("channel");
		}

		return result;
	}
}
}
