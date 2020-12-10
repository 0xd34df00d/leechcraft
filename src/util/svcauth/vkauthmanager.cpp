/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkauthmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QtDebug>
#include <QTimer>
#include <QEvent>
#include <QWebView>
#include <QFuture>
#include <QFutureInterface>
#include <util/network/customcookiejar.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <interfaces/core/ientitymanager.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Util::SvcAuth
{
	namespace
	{
		QUrl URLFromClientID (const QString& id, const QStringList& scope)
		{
			auto url = QUrl::fromEncoded ("https://oauth.vk.com/authorize?redirect_uri=http%3A%2F%2Foauth.vk.com%2Fblank.html&response_type=token&state=");
			UrlOperator { url }
					(QStringLiteral ("client_id"), id)
					(QStringLiteral ("scope"), scope.join (','));
			return url;
		}
	}

	VkAuthManager::VkAuthManager (const QString& accName,
			const QString& id, const QStringList& scope,
			const QByteArray& cookies, const ICoreProxy_ptr& proxy,
			QueueManager *queueMgr, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, AccountHR_ (accName)
	, AuthNAM_ (new QNetworkAccessManager (this))
	, Cookies_ (new Util::CustomCookieJar (this))
	, Queue_ (queueMgr)
	, ID_ (id)
	, URL_ (URLFromClientID (ID_, scope))
	, ScheduleTimer_ (new QTimer (this))
	{
		AuthNAM_->setCookieJar (Cookies_);
		Cookies_->Load (cookies);

		ScheduleTimer_->setSingleShot (true);
		connect (ScheduleTimer_,
				&QTimer::timeout,
				this,
				[this]
				{
					IsRequestScheduled_ = false;
					RequestAuthKey ();
				});
	}

	bool VkAuthManager::IsAuthenticated () const
	{
		return !Token_.isEmpty () &&
			(!ValidFor_ || ReceivedAt_.secsTo (QDateTime::currentDateTime ()) < ValidFor_);
	}

	bool VkAuthManager::HadAuthentication () const
	{
		return !Token_.isEmpty () || !Cookies_->allCookies ().isEmpty ();
	}

	void VkAuthManager::UpdateScope (const QStringList& scope)
	{
		const auto& newUrl = URLFromClientID (ID_, scope);
		if (URL_ == newUrl)
			return;

		URL_ = newUrl;
		Token_.clear ();
		ReceivedAt_ = QDateTime ();
		ValidFor_ = 0;
	}

	void VkAuthManager::GetAuthKey ()
	{
		if (!IsAuthenticated ())
		{
			if (!SilentMode_)
				RequestAuthKey ();
			else
			{
				for (const auto& queue : PrioManagedQueues_)
					queue->clear ();
				for (const auto& queue : ManagedQueues_)
					queue->clear ();
			}

			return;
		}

		InvokeQueues (Token_);
		emit gotAuthKey (Token_);
	}

	QFuture<VkAuthManager::AuthKeyResult_t> VkAuthManager::GetAuthKeyFuture ()
	{
		QFutureInterface<AuthKeyResult_t> iface;
		iface.reportStarted ();

		if (SilentMode_ && !IsAuthenticated ())
			ReportFutureResult (iface, AuthKeyError_t { SilentMode {} });
		else
		{
			connect (this,
					&VkAuthManager::gotAuthKey,
					[this, iface] () mutable { ReportFutureResult (iface, Token_); });
			GetAuthKey ();
		}

		return iface.future ();
	}

	auto VkAuthManager::ManageQueue (VkAuthManager::RequestQueue_ptr queue) -> ScheduleGuard_t
	{
		if (!Queue_)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot manage request queue if queue manager wasn't set";
			return {};
		}

		ManagedQueues_ << queue;

		return Util::MakeScopeGuard ([this, queue] { ManagedQueues_.removeAll (queue); });
	}

	auto VkAuthManager::ManageQueue (VkAuthManager::PrioRequestQueue_ptr queue) -> ScheduleGuard_t
	{
		if (!Queue_)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot manage request queue if queue manager wasn't set";
			return {};
		}

		PrioManagedQueues_ << queue;

		return Util::MakeScopeGuard ([this, queue] { PrioManagedQueues_.removeAll (queue); });
	}

	void VkAuthManager::SetSilentMode (bool silent)
	{
		SilentMode_ = silent;
	}

	void VkAuthManager::InvokeQueues (const QString& token)
	{
		ScheduleTrack (token);

		for (auto queue : PrioManagedQueues_)
			while (!queue->isEmpty ())
			{
				const auto& pair = queue->takeFirst ();
				const auto& f = pair.first;
				Queue_->Schedule ([f, token] { f (token); }, nullptr, pair.second);
			}

		for (auto queue : ManagedQueues_)
			while (!queue->isEmpty ())
			{
				const auto& f = queue->takeFirst ();
				Queue_->Schedule ([f, token] { f (token); });
			}
	}

	void VkAuthManager::RequestURL (const QUrl& url)
	{
		qDebug () << Q_FUNC_INFO << url;
		auto reply = AuthNAM_->get (QNetworkRequest (url));
		connect (reply,
				&QNetworkReply::finished,
				this,
				[this, reply] { HandleGotForm (reply); });
	}

	void VkAuthManager::RequestAuthKey ()
	{
		if (IsRequestScheduled_ && ScheduleTimer_->isActive ())
			ScheduleTimer_->stop ();

		if (IsRequesting_)
			return;

		RequestURL (URL_);
		IsRequesting_ = true;
	}

	bool VkAuthManager::CheckReply (QUrl location)
	{
		if (location.path () != "/blank.html"_ql)
			return CheckError (location);

		location = QUrl::fromEncoded (location.toEncoded ().replace ('#', '?'));
		const QUrlQuery query { location };
		Token_ = query.queryItemValue (QStringLiteral ("access_token"));
		ValidFor_ = query.queryItemValue (QStringLiteral ("expires_in")).toInt ();
		ReceivedAt_ = QDateTime::currentDateTime ();
		qDebug () << Q_FUNC_INFO << Token_ << ValidFor_;
		IsRequesting_ = false;

		InvokeQueues (Token_);
		emit gotAuthKey (Token_);
		emit justAuthenticated ();

		return true;
	}

	bool VkAuthManager::CheckError (const QUrl& url)
	{
		if (url.path () != "/error"_ql)
			return false;

		const auto errNum = QUrlQuery { url }.queryItemValue (QStringLiteral ("err")).toInt ();

		IsRequesting_ = false;

		qWarning () << Q_FUNC_INFO
				<< "got error"
				<< errNum;
		if (errNum == 2)
		{
			clearAuthData ();

			RequestAuthKey ();
			return true;
		}

		const auto& e = Util::MakeNotification (QStringLiteral ("VK.com"),
				tr ("VK.com authentication for %1 failed because of error %2. "
					"Report upstream please.")
					.arg (AccountHR_)
					.arg (errNum),
				Priority::Critical);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		return true;
	}

	void VkAuthManager::ScheduleTrack (const QString& key)
	{
		if (HasTracked_)
			return;

		if (!Proxy_->GetSettingsManager ()->property ("TrackVK").toBool ())
			return;

		HasTracked_ = true;

		QUrl url { QStringLiteral ("https://api.vk.com/method/stats.trackVisitor") };
		Util::UrlOperator { url }
				(QStringLiteral ("access_token"), key);

		auto reply = AuthNAM_->get (QNetworkRequest { url });
		connect (reply,
				&QNetworkReply::finished,
				reply,
				&QNetworkReply::deleteLater);
	}

	void VkAuthManager::clearAuthData ()
	{
		Cookies_->Load ({});
		Token_.clear ();
		ReceivedAt_ = QDateTime ();
		ValidFor_ = 0;
	}

	namespace
	{
		template<typename F>
		class CloseEventFilter : public QObject
		{
			const F Handler_;
		public:
			CloseEventFilter (const F& handler, QObject *handlee)
			: QObject { handlee }
			, Handler_ { handler }
			{
				handlee->installEventFilter (this);
			}

			bool eventFilter (QObject*, QEvent *event) override
			{
				if (event->type () == QEvent::Close)
					Handler_ ();
				return false;
			}
		};
	}

	void VkAuthManager::reauth ()
	{
		auto view = new QWebView;
		view->setWindowTitle (tr ("VK.com authentication for %1")
				.arg (AccountHR_));
		view->setWindowFlags (Qt::Window);
		view->resize (800, 600);
		view->page ()->setNetworkAccessManager (AuthNAM_);
		view->show ();

		view->setUrl (URL_);

		connect (view,
				&QWebView::urlChanged,
				this,
				[this, view] (const QUrl& url)
				{

					if (!CheckReply (url))
						return;

					emit cookiesChanged (Cookies_->Save ());
					view->deleteLater ();
				});

		new CloseEventFilter { [this] { emit authCanceled (); }, view };
	}

	void VkAuthManager::HandleGotForm (QNetworkReply *reply)
	{
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			qWarning () << Q_FUNC_INFO
					<< reply->error ()
					<< reply->errorString ();

			IsRequesting_ = false;

			if (!IsRequestScheduled_)
			{
				IsRequestScheduled_ = true;
				ScheduleTimer_->start (30000);
			}

			return;
		}

		const auto& location = reply->header (QNetworkRequest::LocationHeader).toUrl ();
		if (location.isEmpty ())
		{
			reauth ();
			return;
		}

		if (CheckReply (location))
			return;

		RequestURL (location);
	}
}
