/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>
#include <QStringList>

class IWebFileStorage;

namespace LC
{
namespace DBusManager
{
	class WebFileStorageAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.WebFileStorage")
		Q_PROPERTY (QStringList ServiceVariants READ GetServiceVariants)

		IWebFileStorage * const WFS_;
	public:
		WebFileStorageAdaptor (QObject*);

		QStringList GetServiceVariants () const;
	public slots:
		Q_NOREPLY void UploadFile (const QString& filename, const QString& service);
	private slots:
		void handleFileUploaded (const QString& filename, const QUrl& url);
	signals:
		void FileUploaded (const QString& filename, const QString& url);
	};
}
}
