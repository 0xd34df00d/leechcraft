/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkAccessManager>
#include <QLocale>
#include "interfaces/core/ihookproxy.h"

class QTimer;

namespace LC
{
	class SslErrorsDialog;

	namespace Util
	{
		class CustomCookieJar;
	}

	class NetworkAccessManager : public QNetworkAccessManager
	{
		Q_OBJECT

		Util::CustomCookieJar *CookieJar_;
	public:
		NetworkAccessManager (QObject* = 0);
		virtual ~NetworkAccessManager ();
	protected:
		QNetworkReply* createRequest (Operation,
				const QNetworkRequest&, QIODevice*);
	private slots:
		void saveCookies () const;
		void handleFilterTrackingCookies ();
		void setCookiesEnabled ();
		void setMatchDomainExactly ();
		void setCookiesLists ();

		void handleCacheSize ();
	signals:
		void requestCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);
		void acceptableLanguagesChanged ();

		void hookNAMCreateRequest (LC::IHookProxy_ptr proxy,
					QNetworkAccessManager *manager,
					QNetworkAccessManager::Operation *op,
					QIODevice **dev);
	};
};
