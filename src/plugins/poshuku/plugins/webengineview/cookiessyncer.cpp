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
		WebEngineStore_->loadAllCookies ();

		connect (WebEngineStore_,
				&QWebEngineCookieStore::cookieAdded,
				this,
				&CookiesSyncer::HandleWebEngineCookieAdded);
		connect (WebEngineStore_,
				&QWebEngineCookieStore::cookieRemoved,
				this,
				&CookiesSyncer::HandleWebEngineCookieRemoved);
	}

	void CookiesSyncer::HandleWebEngineCookieAdded (const QNetworkCookie& cookie)
	{
		using namespace std::chrono_literals;

		if (WebEngine2LCQueue_.isEmpty ())
			QTimer::singleShot (1s, Qt::VeryCoarseTimer, this,
					[this]
					{
						for (const auto& cookie : WebEngine2LCQueue_)
							LCJar_->insertCookie (cookie);
						WebEngine2LCQueue_.clear ();
					});

		WebEngine2LCQueue_.append (cookie);
	}

	void CookiesSyncer::HandleWebEngineCookieRemoved (const QNetworkCookie& cookie)
	{
		WebEngine2LCQueue_.removeAll (cookie);
		LCJar_->deleteCookie (cookie);
	}
}
