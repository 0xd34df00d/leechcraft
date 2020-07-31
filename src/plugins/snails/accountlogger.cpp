/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountlogger.h"
#include <QFile>
#include <QDateTime>
#include <QtDebug>
#include <QDir>
#include <util/sys/paths.h>
#include "account.h"

namespace LC
{
namespace Snails
{
	AccountLogger::AccountLogger (const QString& accName, QObject *parent)
	: QObject { parent }
	, AccName_ { accName }
	{
	}

	void AccountLogger::SetEnabled (bool enabled)
	{
		Enabled_ = enabled;
	}

	void AccountLogger::Log (const QString& context, int connId, const QString& msg)
	{
		const auto& now = QDateTime::currentDateTime ();
		const auto& str = QString { "[%1] [%2] [%3]: %4" }
				.arg (now.toString ("dd.MM.yyyy HH:mm:ss.zzz"))
				.arg (context)
				.arg (connId)
				.arg (msg);

		QMetaObject::invokeMethod (this,
				"writeLog",
				Qt::QueuedConnection,
				Q_ARG (QString, str));

		emit gotLog (now, context, connId, msg);
	}

	void AccountLogger::writeLog (const QString& log)
	{
		if (!Enabled_)
			return;

		if (!File_)
		{
			const auto& path = Util::CreateIfNotExists ("snails/logs").filePath (AccName_ + ".log");
			File_ = std::make_shared<QFile> (path);
			if (!File_->open (QIODevice::WriteOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< path
						<< "for writing, error:"
						<< File_->errorString ();
				return;
			}
		}

		if (File_->isOpen ())
		{
			File_->write (log.toUtf8 () + "\n");
			File_->flush ();
		}
	}
}
}
