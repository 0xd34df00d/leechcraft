/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMutex>
#include <QMap>

class QTranslator;

namespace LC
{
namespace HttHare
{
	class TrManager : public QObject
	{
		Q_OBJECT

		QMutex MapLock_;
		QMap<Qt::HANDLE, QMap<QString, QTranslator*>> Translators_;
	public:
		TrManager (QObject* = 0);

		QString Translate (const QStringList& locales, const char *context, const char *src);
	private slots:
		void purge ();
	};
}
}
