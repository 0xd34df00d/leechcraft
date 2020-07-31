/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>

class QNetworkAccessManager;
class QDomDocument;

namespace LC
{
namespace Lastfmscrobble
{
	class Authenticator : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager * const NAM_;
		const ICoreProxy_ptr Proxy_;
		bool IsAuthenticated_;
	public:
		Authenticator (QNetworkAccessManager*, const ICoreProxy_ptr&, QObject* = 0);

		void Init ();
		bool IsAuthenticated () const;
	private:
		void FeedPassword (bool);
		bool CheckError (const QDomDocument&);
	private slots:
		void getSessionKey ();
		void handleAuth ();
	signals:
		void authenticated ();
	};
}
}
