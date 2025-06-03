/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serializejson.h"
#include <QFile>
#include <QJsonDocument>
#include <QtDebug>
#include "either.h"

namespace LC
{
namespace Util
{
	QByteArray SerializeJson (const QVariant& var, bool compact)
	{
		return QJsonDocument::fromVariant (var)
				.toJson (compact ? QJsonDocument::Compact : QJsonDocument::Indented);
	}

	Either<QString, Void> SerializeJsonToFile (const QString& filename, const QVariant& var, bool compact)
	{
		QFile file { filename };
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << "unable to open file" << file.fileName () << "for writing:" << file.errorString ();
			return Left { file.errorString () };
		}

		if (!file.write (SerializeJson (var, compact)))
		{
			qWarning () << "unable to write to file" << file.fileName () << ":" << file.errorString ();
			return Left { file.errorString () };
		}

		return Void {};
	}
}
}
