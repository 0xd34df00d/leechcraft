/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H

#include <QObject>
#include <QUrl>
#include <QVariant>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	class ReadItLaterApi : public QObject
	{
		Q_OBJECT
	public:
		QString GetAuthUrl () const;
		QByteArray GetAuthPayload (const QString&, const QString&);
		QString GetRegisterUrl () const;
		QByteArray GetRegisterPayload (const QString&, const QString&);
		QString GetUploadUrl () const;
		QByteArray GetUploadPayload (const QString&,
				const QString&, const QVariantList&);
		QString GetDownloadUrl () const;
		QByteArray GetDownloadPayload (const QString&,
				const QString&, const QDateTime&);
		QVariantList GetDownloadedBookmarks (const QByteArray&);
		
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H
