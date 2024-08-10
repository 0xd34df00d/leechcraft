/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QNetworkCookie;

namespace LC::Util
{
	class CustomCookieJar;
}

namespace LC
{
	class CookieSaver : public QObject
	{
		Util::CustomCookieJar& Jar_;

		QList<QNetworkCookie> AppendQueue_;

		bool SaveScheduled_ = false;
		bool HasRemovedCookies_ = false;
	public:
		explicit CookieSaver (Util::CustomCookieJar&, QObject* = nullptr);
		~CookieSaver () override;
	private:
		void ScheduleSave ();
		void Save ();

		void FullSave ();
	};
}
