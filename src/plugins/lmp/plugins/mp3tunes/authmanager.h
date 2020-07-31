/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QSet>
#include <interfaces/core/icoreproxy.h>

class QNetworkAccessManager;

namespace LC
{
struct Entity;

namespace LMP
{
namespace MP3Tunes
{
	class AuthManager : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;

		QMap<QString, QString> Login2Sid_;
		QSet<QString> FailedAuth_;

		const ICoreProxy_ptr Proxy_;
	public:
		AuthManager (QNetworkAccessManager*, const ICoreProxy_ptr&, QObject* = 0);

		QString GetSID (const QString&);
	private slots:
		void handleAuthReplyFinished ();
		void handleAuthReplyError ();
	signals:
		void sidReady (const QString&);
		void sidError (const QString&, const QString&);
	};
}
}
}
