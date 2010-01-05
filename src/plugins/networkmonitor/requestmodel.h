/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_NETWORKMONITOR_REQUESTMODEL_H 
#define PLUGINS_NETWORKMONITOR_REQUESTMODEL_H 
#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class HeaderModel;

			class RequestModel : public QStandardItemModel
			{
				Q_OBJECT

				HeaderModel *RequestHeadersModel_;
				HeaderModel *ReplyHeadersModel_;
				bool Clear_;
			public:
				RequestModel (QObject* = 0);
				HeaderModel* GetRequestHeadersModel () const;
				HeaderModel* GetReplyHeadersModel () const;
			public slots:
				void handleRequest (QNetworkAccessManager::Operation,
						const QNetworkRequest&, QNetworkReply*);
				void handleFinished ();
				void setClear (bool);
				void handleCurrentChanged (const QModelIndex&);
				void handleGonnaDestroy (QObject*);
			};
		};
	};
};

#endif

