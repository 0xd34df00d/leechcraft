/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_COOKIESEDITMODEL_H
#define PLUGINS_POSHUKU_COOKIESEDITMODEL_H
#include <QStandardItemModel>
#include <QNetworkCookie>
#include <QMap>

namespace LC
{
namespace Util
{
	class CustomCookieJar;
}

namespace Poshuku
{
	class CookiesEditModel : public QStandardItemModel
	{
		Q_OBJECT

		Util::CustomCookieJar *Jar_;
		QMap<int, QNetworkCookie> Cookies_;
	public:
		CookiesEditModel (QObject* = 0);
		QNetworkCookie GetCookie (const QModelIndex&) const;
		void SetCookie (const QModelIndex&, const QNetworkCookie&);
		void RemoveCookie (const QModelIndex&);
	private:
		void AddCookie (const QNetworkCookie&);
	};
}
}

#endif
