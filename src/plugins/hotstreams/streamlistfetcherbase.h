/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QIcon>

class QNetworkAccessManager;
class QNetworkRequest;
class QStandardItem;

namespace LC
{
namespace HotStreams
{
	class StreamListFetcherBase : public QObject
	{
	protected:
		QNetworkAccessManager *NAM_;
		QStandardItem *Root_;
		QIcon RadioIcon_;

		struct StreamInfo
		{
			QString Name_;
			QString Description_;
			QStringList Genres_;
			QUrl URL_;
			QUrl IconURL_;
			QString DJ_;
			QString PlaylistFormat_;
		};
	public:
		StreamListFetcherBase (QStandardItem*, QNetworkAccessManager*, QObject* = nullptr);
	protected:
		void Request (const QNetworkRequest&);

		virtual void HandleData (const QByteArray&);
		virtual QList<StreamInfo> Parse (const QByteArray&) = 0;
	};
}
}
