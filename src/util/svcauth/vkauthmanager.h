/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <variant>
#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/util.h>
#include <util/sll/eitherfwd.h>
#include "svcauthconfig.h"

class QTimer;

template<typename>
class QFuture;

namespace LC::Util
{
class QueueManager;
enum class QueuePriority;

class CustomCookieJar;

namespace SvcAuth
{
	class UTIL_SVCAUTH_API VkAuthManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		const QString AccountHR_;

		QNetworkAccessManager * const AuthNAM_;
		Util::CustomCookieJar * const Cookies_;

		QueueManager * const Queue_;

		QString Token_;
		QDateTime ReceivedAt_;
		qint32 ValidFor_ = 0;

		bool IsRequesting_ = false;

		const QString ID_;
		QUrl URL_;

		bool IsRequestScheduled_ = false;
		QTimer * const ScheduleTimer_;

		bool SilentMode_ = false;

		bool HasTracked_ = false;
	public:
		using RequestQueue_t = QList<std::function<void (QString)>> ;
		using RequestQueue_ptr = RequestQueue_t*;

		using PrioRequestQueue_t = QList<QPair<std::function<void (QString)>, QueuePriority>>;
		using PrioRequestQueue_ptr = PrioRequestQueue_t*;

		using ScheduleGuard_t = Util::DefaultScopeGuard;
	private:
		QList<RequestQueue_ptr> ManagedQueues_;
		QList<PrioRequestQueue_ptr> PrioManagedQueues_;
	public:
		VkAuthManager (const QString& accountName, const QString& clientId,
				const QStringList& scope, const QByteArray& cookies,
				ICoreProxy_ptr, QueueManager* = nullptr, QObject* = nullptr);

		bool IsAuthenticated () const;
		bool HadAuthentication () const;

		void UpdateScope (const QStringList&);

		void GetAuthKey ();

		struct SilentMode {};
		using AuthKeyError_t = std::variant<SilentMode>;
		using AuthKeyResult_t = Either<AuthKeyError_t, QString>;

		[[nodiscard]] QFuture<AuthKeyResult_t> GetAuthKeyFuture ();

		[[nodiscard]] ScheduleGuard_t ManageQueue (RequestQueue_ptr);
		[[nodiscard]] ScheduleGuard_t ManageQueue (PrioRequestQueue_ptr);

		void SetSilentMode (bool);
	private:
		void InvokeQueues (const QString&);

		void RequestURL (const QUrl&);
		void RequestAuthKey ();
		bool CheckReply (QUrl);
		bool CheckError (const QUrl&);

		void ScheduleTrack (const QString&);
	public slots:
		void clearAuthData ();
		void reauth ();
	private slots:
		void execScheduledRequest ();
		void handleGotForm ();
		void handleViewUrlChanged (const QUrl&);
	signals:
		void gotAuthKey (const QString&);
		void cookiesChanged (const QByteArray&);
		void authCanceled ();
		void justAuthenticated ();
	};
}
}
