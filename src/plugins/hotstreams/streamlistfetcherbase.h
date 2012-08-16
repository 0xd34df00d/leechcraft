/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#pragma once

#include <QObject>
#include <QUrl>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkRequest;
class QStandardItem;

namespace LeechCraft
{
namespace HotStreams
{
	class StreamListFetcherBase : public QObject
	{
		Q_OBJECT
	protected:
		QNetworkAccessManager *NAM_;
		QStandardItem *Root_;

		struct StreamInfo
		{
			QString Name_;
			QString Description_;
			QStringList Genres_;
			QUrl URL_;
			QUrl IconURL_;
		};
	public:
		StreamListFetcherBase (QStandardItem*, QNetworkAccessManager*, QObject* = 0);
	protected:
		void Request (const QNetworkRequest&);

		virtual void HandleData (const QByteArray&);
		virtual QList<StreamInfo> Parse (const QByteArray&) = 0;
	protected slots:
		void handleReplyFinished ();
	};
}
}
