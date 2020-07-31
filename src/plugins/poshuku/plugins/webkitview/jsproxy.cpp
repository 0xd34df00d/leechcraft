/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jsproxy.h"
#include <QString>
#include <QVariant>
#include <QTextCodec>
#include <QtDebug>
#include <util/util.h>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	void JSProxy::debug (const QString& string)
	{
		qDebug () << Q_FUNC_INFO << string;
	}

	void JSProxy::warning (const QString& string)
	{
		qWarning () << Q_FUNC_INFO << string;
	}
}
}
}
