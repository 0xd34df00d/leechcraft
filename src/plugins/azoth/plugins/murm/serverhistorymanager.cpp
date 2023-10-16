/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverhistorymanager.h"
#include <QStandardItemModel>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <util/sll/serializejson.h>
#include <util/sll/either.h>
#include <interfaces/azoth/ihaveserverhistory.h>
#include "vkaccount.h"
#include "vkentry.h"
#include "logger.h"
#include "util.h"
#include "servermessagessyncer.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	namespace
	{
		const auto DlgChunkCount = 100;

		enum CustomHistRole
		{
			UserUid = ServerHistoryRole::ServerHistoryRoleMax,
			ChatUid,
			UserName
		};
	}

	ServerHistoryManager::ServerHistoryManager (VkAccount *acc)
	: QObject { acc }
	, Acc_ { acc }
	, ContactsModel_ { new QStandardItemModel { this } }
	{
		ContactsModel_->setHorizontalHeaderLabels ({ tr ("Contact") });
	}

	QAbstractItemModel* ServerHistoryManager::GetModel () const
	{
		return ContactsModel_;
	}

	void ServerHistoryManager::RequestHistory (const QModelIndex& index, int offset, int count)
	{
		if (count < 0)
		{
			count = -count;
			offset = std::max (offset - count, -1);
		}
		else
			++offset;

		if (count > 100)
			count = 100;

		const auto nam = Acc_->GetCoreProxy ()->GetNetworkAccessManager ();

		const auto uidVar = index.data (CustomHistRole::UserUid);
		if (uidVar.isValid ())
		{
			const auto uid = uidVar.toULongLong ();
			auto getter = [=, this] (const QString& key, const VkConnection::UrlParams_t& params)
				{
					QUrl url ("https://api.vk.com/method/messages.getHistory");
					Util::UrlOperator { url }
							("access_token", key)
							("uid", QString::number (uid))
							("count", QString::number (count))
							("offset", QString::number (offset));
					VkConnection::AddParams (url, params);

					LastOffset_ = offset;

					auto reply = nam->get (QNetworkRequest (url));
					MsgRequestState_ [reply] = RequestState { index, offset };
					connect (reply,
							SIGNAL (finished ()),
							this,
							SLOT (handleGotHistory ()));
					return reply;
				};

			Acc_->GetConnection ()->QueueRequest (getter);
		}
		else
		{
			const auto chatId = index.data (CustomHistRole::ChatUid).toULongLong ();
			auto getter = [=, this] (const QString& key, const VkConnection::UrlParams_t& params)
				{
					QUrl url ("https://api.vk.com/method/messages.getHistory");
					Util::UrlOperator { url }
							("access_token", key)
							("chat_id", QString::number (chatId))
							("count", QString::number (count))
							("offset", QString::number (offset));
					VkConnection::AddParams (url, params);

					LastOffset_ = offset;

					auto reply = nam->get (QNetworkRequest (url));
					MsgRequestState_ [reply] = RequestState { index, offset };
					connect (reply,
							SIGNAL (finished ()),
							this,
							SLOT (handleGotChatHistory ()));
					return reply;
				};

			Acc_->GetConnection ()->QueueRequest (getter);
		}
	}

	QFuture<IHaveServerHistory::DatedFetchResult_t> ServerHistoryManager::FetchServerHistory (const QDateTime& since)
	{
		const auto syncer = new ServerMessagesSyncer { since, Acc_ };
		return syncer->GetFuture ();
	}

	void ServerHistoryManager::Request (int offset)
	{
		const auto nam = Acc_->GetCoreProxy ()->GetNetworkAccessManager ();

		auto getter = [offset, nam, this]
				(const QString& key, const VkConnection::UrlParams_t& params) -> QNetworkReply*
			{
				QUrl url ("https://api.vk.com/method/messages.getDialogs");
				Util::UrlOperator { url }
						("access_token", key)
						("count", QString::number (DlgChunkCount))
						("offset", QString::number (offset));
				VkConnection::AddParams (url, params);

				LastOffset_ = offset;

				auto reply = nam->get (QNetworkRequest (url));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotMessagesList ()));
				return reply;
			};

		Acc_->GetConnection ()->QueueRequest (getter);
	}

	void ServerHistoryManager::AddUserItem (const QVariantMap& varmap)
	{
		const auto uid = varmap ["user_id"].toULongLong ();
		const auto ts = varmap ["date"].toULongLong ();

		const auto entry = Acc_->GetEntry (uid);
		if (!entry)
			return;

		auto item = new QStandardItem (entry->GetEntryName ());
		item->setEditable (false);
		item->setData (QDateTime::fromSecsSinceEpoch (ts), ServerHistoryRole::LastMessageDate);
		item->setData (QVariant::fromValue<QObject*> (entry), ServerHistoryRole::CLEntry);
		item->setData (uid, CustomHistRole::UserUid);
		item->setData (entry->GetEntryName (), CustomHistRole::UserName);
		ContactsModel_->appendRow (item);
	}

	void ServerHistoryManager::AddRoomItem (const QVariantMap& varmap)
	{
		const auto chatId = varmap ["chat_id"].toULongLong ();
		const auto ts = varmap ["date"].toULongLong ();
		const auto& title = varmap ["title"].toString ();

		auto item = new QStandardItem { title };
		item->setEditable (false);
		item->setData (QDateTime::fromSecsSinceEpoch (ts), ServerHistoryRole::LastMessageDate);
		item->setData (chatId, CustomHistRole::ChatUid);
		item->setData (title, CustomHistRole::UserName);
		ContactsModel_->appendRow (item);
	}

	void ServerHistoryManager::refresh ()
	{
		if (IsRefreshing_)
			return;

		IsRefreshing_ = true;

		if (const auto rc = ContactsModel_->rowCount ())
			ContactsModel_->removeRows (0, rc);

		MsgCount_ = -1;
		Request (0);
	}

	void ServerHistoryManager::handleGotHistory ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& reqContext = MsgRequestState_.take (reply);

		const auto& replyVar = Util::ParseJson (reply, Q_FUNC_INFO);
		if (replyVar.isNull ())
			return;

		Acc_->GetLogger () << replyVar;

		const auto& varmap = replyVar.toMap ();

		SrvHistMessages_t messages;
		for (const auto& var : varmap ["response"].toMap () ["items"].toList ())
		{
			const auto& map = var.toMap ();
			if (map.isEmpty ())
				continue;

			const auto dir = map ["out"].toInt () ?
					IMessage::Direction::Out :
					IMessage::Direction::In;

			const auto& username = dir == IMessage::Direction::In ?
					reqContext.Index_.data (CustomHistRole::UserName).toString () :
					Acc_->GetSelf ()->GetEntryName ();

			messages.append ({
					dir,
					{},
					username,
					map ["body"].toString (),
					QDateTime::fromSecsSinceEpoch (map ["date"].toULongLong ()),
					{}
				});
		}

		for (int i = 0; i < messages.size (); ++i)
			messages [i].ID_ = QByteArray::number (reqContext.Offset_ + i);

		std::reverse (messages.begin (), messages.end ());

		emit serverHistoryFetched (reqContext.Index_,
				QByteArray::number (reqContext.Offset_), messages);
	}

	void ServerHistoryManager::handleGotChatHistory ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& reqContext = MsgRequestState_.take (reply);

		const auto& replyVar = Util::ParseJson (reply, Q_FUNC_INFO);
		if (replyVar.isNull ())
			return;

		Acc_->GetLogger () << replyVar;

		const auto& varmap = replyVar.toMap ();

		QList<qulonglong> toRequest;
		QList<QPair<SrvHistMessage, qulonglong>> messages;
		for (const auto& var : varmap ["response"].toMap () ["items"].toList ())
		{
			const auto& map = var.toMap ();
			if (map.isEmpty ())
				continue;

			const auto from = map ["from_id"].toULongLong ();

			const auto& username = reqContext.Index_.data (CustomHistRole::UserName).toString ();

			messages.append ({
					{
						IMessage::Direction::In,
						{},
						username,
						map ["body"].toString (),
						QDateTime::fromSecsSinceEpoch (map ["date"].toULongLong ()),
						{}
					},
					from
				});

			if (const auto entry = Acc_->GetEntry (from))
				messages.last ().first.Nick_ = entry->GetEntryName ();
			else
				toRequest << from;
		}

		auto infosHandler = [this, messages, reqContext] (const QList<UserInfo>& infos)
		{
			const auto& infosMap = [&infos]
			{
				QHash<qulonglong, UserInfo> infosMap;
				for (const auto& info : infos)
					infosMap [info.ID_] = info;
				return infosMap;
			} ();

			SrvHistMessages_t result;
			for (int i = 0; i < messages.size (); ++i)
			{
				const auto& pair = messages [i];
				auto message = pair.first;
				message.ID_ = QByteArray::number (reqContext.Offset_ + i);

				if (infosMap.contains (pair.second))
					message.Nick_ = FormatUserInfoName (infosMap.value (pair.second));

				result << message;
			}

			std::reverse (result.begin (), result.end ());

			emit serverHistoryFetched (reqContext.Index_,
					QByteArray::number (reqContext.Offset_), result);
		};

		if (!toRequest.isEmpty ())
			Acc_->GetConnection ()->GetUserInfo (toRequest, infosHandler);
		else
			infosHandler ({});
	}

	void ServerHistoryManager::handleGotMessagesList ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		IsRefreshing_ = false;

		const auto& replyVar = Util::ParseJson (reply, Q_FUNC_INFO);
		if (replyVar.isNull ())
			return;

		Acc_->GetLogger () << replyVar;

		const auto& varmap = replyVar.toMap ();

		auto varlist = varmap ["response"].toMap () ["items"].toList ();
		if (varlist.isEmpty ())
			return;

		if (MsgCount_ == -1)
		{
			bool ok = false;
			const auto count = varlist.first ().toInt (&ok);
			if (ok)
			{
				MsgCount_ = count;
				qDebug () << Q_FUNC_INFO
						<< "detected"
						<< count
						<< "dialogs";
			}
		}

		for (const auto& var : varlist)
		{
			const auto& varmap = var.toMap () ["message"].toMap ();
			if (varmap.isEmpty ())
				continue;

			if (varmap.contains ("admin_id"))
				AddRoomItem (varmap);
			else
				AddUserItem (varmap);
		}

		if (LastOffset_ + DlgChunkCount < MsgCount_)
		{
			IsRefreshing_ = true;
			Request (LastOffset_ + DlgChunkCount);
		}
	}
}
}
}
