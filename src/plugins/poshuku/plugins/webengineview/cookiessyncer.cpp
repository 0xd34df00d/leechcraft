/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cookiessyncer.h"
#include <QWebEngineCookieStore>
#include <QTimer>
#include <QtDebug>
#include <util/network/customcookiejar.h>

namespace LC::Poshuku::WebEngineView
{
	CookiesSyncer::CookiesSyncer (Util::CustomCookieJar *lcJar,
			QWebEngineCookieStore *weStore)
	: LCJar_ { lcJar }
	, WebEngineStore_ { weStore }
	{
		WebEngineStore_->deleteAllCookies ();

		HandleLCCookiesAdded (LCJar_->allCookies ());

		connect (LCJar_,
				&Util::CustomCookieJar::cookiesAdded,
				this,
				&CookiesSyncer::HandleLCCookiesAdded);
		connect (LCJar_,
				&Util::CustomCookieJar::cookiesRemoved,
				this,
				&CookiesSyncer::HandleLCCookiesRemoved);

		connect (WebEngineStore_,
				&QWebEngineCookieStore::cookieAdded,
				this,
				&CookiesSyncer::HandleWebEngineCookieAdded);
		connect (WebEngineStore_,
				&QWebEngineCookieStore::cookieRemoved,
				this,
				&CookiesSyncer::HandleWebEngineCookieRemoved);
	}

	void CookiesSyncer::HandleLCCookiesAdded (const QList<QNetworkCookie>& cookies)
	{
		qDebug () << Q_FUNC_INFO << cookies.size ();
		for (const auto& cookie : cookies)
		{
			CookiesPerDomain_ [cookie.domain ()] << cookie;
			WebEngineStore_->setCookie (cookie);
		}
	}

	void CookiesSyncer::HandleLCCookiesRemoved (const QList<QNetworkCookie>& cookies)
	{
		qDebug () << Q_FUNC_INFO << cookies.size ();
		for (const auto& cookie : cookies)
		{
			CookiesPerDomain_ [cookie.domain ()].removeAll (cookie);
			WebEngineStore_->deleteCookie (cookie);
		}
	}

	void CookiesSyncer::HandleWebEngineCookieAdded (const QNetworkCookie& cookie)
	{
		if (WebEngine2LCQueue_.isEmpty ())
			QTimer::singleShot (1000, Qt::VeryCoarseTimer, this,
					[this]
					{
						for (const auto& cookie : WebEngine2LCQueue_)
						{
							auto& domainCookies = CookiesPerDomain_ [cookie.domain ()];
							if (!domainCookies.contains (cookie))
							{
								domainCookies << cookie;
								LCJar_->insertCookie (cookie);
							}
						}
						WebEngine2LCQueue_.clear ();
					});

		WebEngine2LCQueue_.prepend (cookie);
	}

	void CookiesSyncer::HandleWebEngineCookieRemoved (const QNetworkCookie& cookie)
	{
		CookiesPerDomain_ [cookie.domain ()].removeAll (cookie);
		WebEngine2LCQueue_.removeAll (cookie);
		LCJar_->deleteCookie (cookie);
	}
}
