/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>
#include <QStringList>

class IWebFileStorage;

namespace LeechCraft
{
namespace DBusManager
{
	class WebFileStorageAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.WebFileStorage");
		Q_PROPERTY (QStringList ServiceVariants READ GetServiceVariants);

		QObject *WFSObj_;
		IWebFileStorage *WFS_;
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
