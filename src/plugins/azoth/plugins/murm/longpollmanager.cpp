/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "longpollmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QtDebug>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	LongPollManager::LongPollManager (VkConnection *conn, ICoreProxy_ptr proxy)
	: QObject (conn)
	, Conn_ (conn)
	, Proxy_ (proxy)
	{
	}

	void LongPollManager::ForceServerRequery ()
	{
		LPServer_.clear ();
		ShouldStop_ = false;
	}

	void LongPollManager::Stop ()
	{
		ShouldStop_ = true;

		if (CurrentPollReply_)
			CurrentPollReply_->abort ();
	}

	QUrl LongPollManager::GetURLTemplate () const
	{
		QUrl url = LPURLTemplate_;
		return Util::UrlOperator { url }
				("ts", QString::number (LPTS_))
				("wait", QString::number (WaitTimeout_))
				();
	}

	void LongPollManager::HandlePollError (QNetworkReply *pollReply)
	{
		++PollErrorCount_;
		qWarning () << Q_FUNC_INFO
				<< "network error:"
				<< pollReply->error ()
				<< pollReply->errorString ()
				<< "; error count:"
				<< PollErrorCount_;

		switch (pollReply->error ())
		{
		case QNetworkReply::RemoteHostClosedError:
		{
			const auto diff = LastPollDT_.secsTo (QDateTime::currentDateTime ());
			const auto newTimeout = std::max<decltype (diff)> ((diff + WaitTimeout_) / 2 - 1, 5);
			qWarning () << Q_FUNC_INFO
					<< "got timeout with"
					<< WaitTimeout_
					<< diff
					<< "; new timeout:"
					<< newTimeout;
			WaitTimeout_ = newTimeout;
			break;
		}
		case QNetworkReply::HostNotFoundError:
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot find host"
					<< LPURLTemplate_
					<< "scheduling requerying server...";
			ForceServerRequery ();
			QTimer::singleShot (1000,
					this,
					SLOT (start ()));
			return;
		}
		default:
			if (PollErrorCount_ == 4)
				emit pollError ();
			break;
		}

		if (ShouldStop_)
		{
			qWarning () << Q_FUNC_INFO
					<< "got poll error while waiting for stop";
			return;
		}

		QTimer::singleShot (1000,
				this,
				SLOT (poll ()));
	}

	void LongPollManager::start ()
	{
		if (!LPServer_.isEmpty ())
			return;

		auto nam = Proxy_->GetNetworkAccessManager ();
		auto req = [this, nam] (const QString& key, const VkConnection::UrlParams_t& params) -> QNetworkReply*
		{
			QUrl lpUrl ("https://api.vk.com/method/messages.getLongPollServer");
			Util::UrlOperator { lpUrl }
					("access_token", key)
					("use_ssl", "1");

			VkConnection::AddParams (lpUrl, params);

			auto reply = nam->get (QNetworkRequest (lpUrl));
			connect (reply,
					SIGNAL (finished ()),
					this,
					SLOT (handleGotLPServer ()));
			return reply;
		};
		Conn_->QueueRequest (req);
	}

	void LongPollManager::poll ()
	{
		if (CurrentPollReply_)
		{
			qWarning () << Q_FUNC_INFO
					<< "already polling";
			return;
		}

		if (ShouldStop_)
			return;

		const auto& url = GetURLTemplate ();

		LastPollDT_ = QDateTime::currentDateTime ();
		CurrentPollReply_ = Proxy_->
				GetNetworkAccessManager ()->get (QNetworkRequest (url));
		connect (CurrentPollReply_,
				SIGNAL (finished ()),
				this,
				SLOT (handlePollFinished ()));
	}

	void LongPollManager::handlePollFinished ()
	{
		CurrentPollReply_->deleteLater ();
		const auto currentReply = CurrentPollReply_;
		CurrentPollReply_ = nullptr;

		if (currentReply->error () != QNetworkReply::NoError && !ShouldStop_)
			return HandlePollError (currentReply);
		else if (PollErrorCount_)
		{
			qDebug () << Q_FUNC_INFO
					<< "finally successful network reply after"
					<< PollErrorCount_
					<< "errors";
			PollErrorCount_ = 0;

			emit listening ();
		}

		const auto& data = Util::ParseJson (currentReply, Q_FUNC_INFO);
		const auto& rootMap = data.toMap ();
		if (rootMap.contains ("failed"))
		{
			ForceServerRequery ();
			start ();
			return;
		}

		emit gotPollData (rootMap);

		if (rootMap.contains ("ts"))
			LPTS_ = rootMap ["ts"].toULongLong ();

		if (!ShouldStop_)
		{
			if (!LPServer_.isEmpty ())
				poll ();
			else
				start ();
		}
		else
		{
			qDebug () << Q_FUNC_INFO
					<< "should stop polling, stopping...";
			emit stopped ();
			ShouldStop_ = false;
		}
	}

	void LongPollManager::handleGotLPServer()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			qWarning () << Q_FUNC_INFO
					<< "error getting poll server:"
					<< reply->errorString ();
			QTimer::singleShot (15000,
					this,
					SLOT (start ()));
			return;
		}

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		const auto& map = data.toMap () ["response"].toMap ();

		LPKey_ = map ["key"].toString ();
		LPServer_ = map ["server"].toString ();
		LPTS_ = map ["ts"].toULongLong ();

		LPURLTemplate_ = QUrl ("https://" + LPServer_);
		Util::UrlOperator { LPURLTemplate_ }
				("act", "a_check")
				("key", LPKey_)
				("mode", "2");

		emit listening ();

		poll ();
	}
}
}
}
