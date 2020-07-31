/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QUrl>
#include <QtDebug>

namespace LC
{
namespace Scroblibre
{
	QUrl ServiceToUrl (const QString& service)
	{
		if (service == "libre.fm")
			return { "http://turtle.libre.fm/" };

		qWarning () << Q_FUNC_INFO
				<< "unknown service"
				<< service;
		return {};
	}

	QString UrlToService (const QUrl& url)
	{
		if (url == QUrl ("http://turtle.libre.fm/"))
			return "libre.fm";

		qWarning () << Q_FUNC_INFO
				<< "unknown url"
				<< url;
		return url.host ();
	}
}
}
