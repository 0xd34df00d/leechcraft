/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkProxy;
class QAuthenticator;

namespace LC
{
namespace NamAuth
{
	class SQLStorageBackend;

	class NamHandler : public QObject
	{
		Q_OBJECT

		SQLStorageBackend * const SB_;
		QNetworkAccessManager * const NAM_;
	public:
		NamHandler (SQLStorageBackend*, QNetworkAccessManager*);
	private:
		void DoCommonAuth (const QString&, const QString&, QAuthenticator*);
	private slots:
		void handleAuthentication (QNetworkReply*, QAuthenticator*);
		void handleAuthentication (const QNetworkProxy&, QAuthenticator*);
	};
}
}
