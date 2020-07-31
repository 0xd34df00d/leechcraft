/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logger.h"
#include <QDateTime>
#include <QUrl>
#include <QDir>
#include <QtDebug>
#include <util/sll/serializejson.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	Logger::LogProxy::LogProxy (Logger& l, IHaveConsole::PacketDirection dir)
	: L_ (l)
	, Dir_ { dir }
	, File_ { new QFile { l.Filename_ } }
	{
		if (L_.FileEnabled_)
		{
			if (!File_->open (QIODevice::WriteOnly | QIODevice::Append))
				qWarning () << Q_FUNC_INFO
						<< "cannot open log file"
						<< File_->fileName ()
						<< File_->errorString ();
			File_->write ("[" + QDateTime::currentDateTime ().toString (Qt::ISODate).toUtf8 () + "] ");
		}
	}

	Logger::LogProxy::~LogProxy ()
	{
		if (!File_)
			return;

		L_.gotConsolePacket (CurrentString_, Dir_, {});
		WriteImpl ("\n");
	}

	void Logger::LogProxy::Write (const char *msg)
	{
		WriteImpl (msg);
	}

	void Logger::LogProxy::Write (const QString& str)
	{
		WriteImpl (str.toUtf8 ());
	}

	void Logger::LogProxy::Write (qint64 num)
	{
		WriteImpl (QByteArray::number (num));
	}

	void Logger::LogProxy::Write (const QUrl& url)
	{
		WriteImpl (url.toEncoded ());
	}

	void Logger::LogProxy::WriteImpl (const QByteArray& ba)
	{
		CurrentString_ += ba;

		if (L_.FileEnabled_)
			File_->write (ba);
	}

	void Logger::LogProxy::Write (const QVariant& json)
	{
		WriteImpl (Util::SerializeJson (json, false));
	}

	Logger::Logger (const QString& id, QObject *parent)
	: QObject { parent }
	, Filename_ { Util::CreateIfNotExists ("azoth/murm").absoluteFilePath (id) + ".log" }
	{
	}

	void Logger::SetFileEnabled (bool enabled)
	{
		FileEnabled_ = enabled;
	}
}
}
}
