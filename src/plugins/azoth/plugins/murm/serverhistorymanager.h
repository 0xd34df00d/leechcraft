/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QModelIndex>
#include <interfaces/azoth/ihaveserverhistory.h>

class QNetworkReply;
class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;

	class ServerHistoryManager : public QObject
	{
		Q_OBJECT

		VkAccount * const Acc_;
		QStandardItemModel * const ContactsModel_;

		int MsgCount_ = -1;
		int LastOffset_ = 0;

		bool IsRefreshing_ = false;

		struct RequestState
		{
			QModelIndex Index_;
			int Offset_;
		};
		QHash<QNetworkReply*, RequestState> MsgRequestState_;
	public:
		ServerHistoryManager (VkAccount*);

		QAbstractItemModel* GetModel () const;
		void RequestHistory (const QModelIndex&, int, int);

		QFuture<IHaveServerHistory::DatedFetchResult_t> FetchServerHistory (const QDateTime&);
	private:
		void Request (int);

		void AddUserItem (const QVariantMap&);
		void AddRoomItem (const QVariantMap&);
	public slots:
		void refresh ();
	private slots:
		void handleGotHistory ();
		void handleGotChatHistory ();
		void handleGotMessagesList ();
	signals:
		void serverHistoryFetched (const QModelIndex&,
				const QByteArray&, const SrvHistMessages_t&);
	};
}
}
}
