/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxyfwd.h>

class QString;
class QByteArray;

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	QByteArray GetHashedChallenge (const QString& password, const QString& challenge);

	QString GetAccountPassword (const QByteArray& accId, const QString& accName, const ICoreProxy_ptr& proxy);
}
}
}
