/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariant>
#include "deliciousaccount.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	class DeliciousApi : public QObject
	{
	public:
		DeliciousApi () = default;

		QString GetAuthUrl (const DeliciousAccount::AuthType& at = DeliciousAccount::ATHttpAuth) const;
		QString GetUploadUrl (const DeliciousAccount::AuthType& at = DeliciousAccount::ATHttpAuth) const;

		QByteArray GetUploadPayload (const QVariant&);
		QString GetDownloadUrl (const DeliciousAccount::AuthType& at = DeliciousAccount::ATHttpAuth) const;

		QByteArray GetDownloadPayload (const QDateTime&);
		QVariantList ParseDownloadReply (const QByteArray&);

		bool ParseAuthReply (const QByteArray&);
		bool ParseUploadReply (const QByteArray&);
	};
}
}
}
}
