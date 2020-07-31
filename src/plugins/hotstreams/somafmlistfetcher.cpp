/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "somafmlistfetcher.h"
#include <algorithm>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <QDomDocument>
#include <QtDebug>

namespace LC
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
				getText ("genre").split ('|', Qt::SkipEmptyParts),
				url,
				QUrl (),
				getText ("dj") + " (" + getText ("djmail") + ")",
				"pls"
			};
			result << info;
			channel = channel.nextSiblingElement ("channel");
		}

		return result;
	}
}
}
