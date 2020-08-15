/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "sslerrorsdialog.h"

class QSslError;
class QNetworkReply;

namespace LC
{
	class SslErrorsHandler : public QObject
	{
	public:
		SslErrorsHandler (QNetworkReply*, const QList<QSslError>&, QObject* = nullptr);
	};
}
