/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "sslerrorsdialog.h"

class QSslError;
class QNetworkReply;
class QSettings;

namespace LC
{
	class SslErrorsHandler : public QObject
	{
		Q_OBJECT
	public:
		SslErrorsHandler (QNetworkReply*, const QList<QSslError>&, QObject* = nullptr);
	private:
		void HandleFinished (int,
				SslErrorsDialog::RememberChoice,
				const QString&,
				const QString&,
				const std::shared_ptr<QSettings>&);
	};
}
