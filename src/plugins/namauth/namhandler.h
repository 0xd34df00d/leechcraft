/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QCoreApplication>
#include <QHash>
#include <QObject>
#include <QSet>
#include <util/threads/coro/taskfwd.h>
#include <util/threads/coro/corocontext.h>
#include "sqlstoragebackend.h"

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkProxy;
class QAuthenticator;

namespace LC::NamAuth
{
	class NamHandler : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::NamAuth::NamHandler)

		struct Site
		{
			QString Realm_;
			QString Context_;

			bool operator== (const Site&) const = default;

			friend size_t qHash (const Site& site, size_t seed = 0) { return qHashMulti (seed, site.Realm_, site.Context_); }
		};

		SQLStorageBackend * const SB_;

		QHash<Site, SQLStorageBackend::AuthRecord> SessionCredentials_;
		QSet<Site> OpenQuestions_;
	public:
		NamHandler (SQLStorageBackend*, QNetworkAccessManager*);
	private:
		void DoCommonAuth (const QString&, QAuthenticator*);
		std::optional<SQLStorageBackend::AuthRecord> GetCredentials (const Site&) const;
		Util::ContextTask<void> Ask (Site, QString login, QString password);

		LC_CORO_CONTEXT
	};
}
