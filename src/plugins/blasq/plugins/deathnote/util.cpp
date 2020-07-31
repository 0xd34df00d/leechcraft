/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <util/xpc/passutils.h>

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	QByteArray GetHashedChallenge (const QString& password, const QString& challenge)
	{
		const auto& hash = QCryptographicHash::hash (password.toUtf8 (), QCryptographicHash::Md5).toHex ();
		return QCryptographicHash::hash ((challenge + hash).toUtf8 (), QCryptographicHash::Md5).toHex ();
	}

	QString GetAccountPassword (const QByteArray& accId, const QString& accName, const ICoreProxy_ptr& proxy)
	{
		const auto& key = "org.LeechCraft.Blasq.PassForAccount/" + accId;
		return Util::GetPassword (key,
				QObject::tr ("Enter password for LiveJournal FotoBilder account %1:")
					.arg ("<em>" + accName + "</em>"),
				proxy);
	}
}
}
}
