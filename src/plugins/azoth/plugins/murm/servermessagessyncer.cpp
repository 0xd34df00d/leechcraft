/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "servermessagessyncer.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/sll/either.h>
#include <util/sll/slotclosure.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <util/sll/prelude.h>
#include "vkconnection.h"
#include "vkaccount.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	namespace
	{
		const auto RequestSize = 200;
	}

	ServerMessagesSyncer::ServerMessagesSyncer (const QDateTime& since, VkAccount *acc, QObject *parent)
	: QObject { parent }
	, Since_ { since }
	, Acc_ { acc }
	{
		Iface_.reportStarted ();
		Request ();
	}

	QFuture<IHaveServerHistory::DatedFetchResult_t> ServerMessagesSyncer::GetFuture ()
	{
		return Iface_.future ();
	}

	void ServerMessagesSyncer::Request ()
	{
		QPointer<QObject> guard { this };
		auto getter = [=, this] (const QString& key, const VkConnection::UrlParams_t& params) -> QNetworkReply*
		{
			if (!guard)
			{
				qWarning () << Q_FUNC_INFO
						<< "the object is already dead";
				return nullptr;
			}

			const auto gracePeriod = 60 * 10;
			const auto secsDiff = Since_.secsTo (QDateTime::currentDateTime ()) + gracePeriod;

			QUrl url { "https://api.vk.com/method/messages.get" };
			Util::UrlOperator { url }
					("access_token", key)
					("count", RequestSize)
					("offset", Offset_)
					("time_offset", secsDiff);
			VkConnection::AddParams (url, params);

			const auto reply = Acc_->GetCoreProxy ()->
					GetNetworkAccessManager ()->get (QNetworkRequest { url });

			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				[=, this] { HandleFinished (reply); },
				reply,
				SIGNAL (finished ()),
				this
			};

			return reply;
		};

		Acc_->GetConnection ()->QueueRequest (getter);
	}

	void ServerMessagesSyncer::HandleFinished (QNetworkReply *reply)
	{
		const auto& json = Util::ParseJson (reply, Q_FUNC_INFO);
		reply->deleteLater ();

		const auto itemsVar = json.toMap () ["response"].toMap () ["items"];
		if (itemsVar.type () != QVariant::List)
		{
			ReportError ("Unable to parse reply.");
			return;
		}

		const auto& itemsList = itemsVar.toList ();

		const auto& accId = Acc_->GetAccountID ();
		for (const auto& mapVar : itemsList)
		{
			const auto& map = mapVar.toMap ();

			const HistoryItem item
			{
				QDateTime::fromSecsSinceEpoch (map ["date"].toULongLong ()),
				map ["out"].toInt () == 1 ? IMessage::Direction::Out : IMessage::Direction::In,
				map ["body"].toString (),
				{},
				IMessage::Type::ChatMessage,
				{},
				IMessage::EscapePolicy::Escape
			};

			const auto& id = accId + QString::number (map ["user_id"].toLongLong ());
			Messages_ [id].VisibleName_ = id;
			Messages_ [id].Messages_ << item;
		}

		if (itemsList.size () == RequestSize)
		{
			Offset_ += RequestSize;
			Request ();
		}
		else
			HandleDone ();
	}

	void ServerMessagesSyncer::HandleDone ()
	{
		qDebug () << Q_FUNC_INFO
				<< Messages_.size ();

		for (auto& list : Messages_)
			std::sort (list.Messages_.begin (), list.Messages_.end (), Util::ComparingBy (&HistoryItem::Date_));

		const auto res = IHaveServerHistory::DatedFetchResult_t::Right (Messages_);
		Iface_.reportFinished (&res);

		deleteLater ();
	}

	void ServerMessagesSyncer::ReportError (const QString& err)
	{
		const auto res = IHaveServerHistory::DatedFetchResult_t::Left (err);
		Iface_.reportFinished (&res);

		deleteLater ();
	}
}
}
}
