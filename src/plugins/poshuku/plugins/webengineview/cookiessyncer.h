/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>

class QWebEngineCookieStore;
class QNetworkCookie;

namespace LC::Util
{
	class CustomCookieJar;
}

namespace LC::Poshuku::WebEngineView
{
	class CookiesSyncer : public QObject
	{
		Util::CustomCookieJar * const LCJar_;
		QWebEngineCookieStore * const WebEngineStore_;

		QList<QNetworkCookie> WebEngine2LCQueue_;
	public:
		CookiesSyncer (Util::CustomCookieJar*, QWebEngineCookieStore*);
	private:
		void HandleWebEngineCookieAdded (const QNetworkCookie&);
		void HandleWebEngineCookieRemoved (const QNetworkCookie&);
	};
}
