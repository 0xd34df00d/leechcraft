/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_UTILPROXY_H
#define PLUGINS_QROSP_WRAPPERS_UTILPROXY_H
#include <QObject>
#include <QDir>
#include <QUrl>
#include <interfaces/structures.h>
#include "wrappers/entitywrapper.h"

class QTranslator;

namespace LC
{
namespace Qrosp
{
	class UtilProxy : public QObject
	{
		Q_OBJECT
	public:
		UtilProxy (QObject* = 0);
	public slots:
		QString MakePrettySize (qint64 size) const;
		QString MakeTimeFromLong (ulong time) const;
		QTranslator* InstallTranslator (const QString& base,
				const QString& prefix, const QString& appname) const;
		QString GetLocaleName () const;
		QString GetLanguage () const;
		QDir CreateIfNotExists (const QString& path) const;
		QString GetTemporaryName (const QString& pattern) const;
		QObject* MakeEntity (const QVariant& entity,
				const QString& location,
				LC::TaskParameters tp,
				const QString& mime) const;
		QObject* MakeNotification (const QString& header,
				const QString& text,
				QString priority) const;
	};
}
}

#endif
