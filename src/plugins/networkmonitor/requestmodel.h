/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LC
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class HeaderModel;

			class RequestModel : public QStandardItemModel
			{
				Q_OBJECT

				HeaderModel * const RequestHeadersModel_;
				HeaderModel * const ReplyHeadersModel_;
				bool Clear_ = true;
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
		}
	}
}
