/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QQueue>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class DriveManager;
	struct DriveChanges;
	struct DriveItem;

	class Syncer : public QObject
	{
		Q_OBJECT

		qlonglong LastChangesId_;
		DriveManager *DM_;

		QString BaseDir_;
		QStringList Paths_;
		QMap<QString, DriveItem> RealPath2Item_;
		QQueue<QString> RealPathQueue_;
		QList<DriveItem> Items_;
	public:
		Syncer (DriveManager *dm, QObject *parent = 0);

		void CheckRemoteStorage ();
		void CheckLocalStorage (const QStringList& paths, const QString& baseDir);
	private:
		void ContinueLocalStorageChecking ();

	private slots:
		void handleGotDriveChanges (const QList<DriveChanges>& changes, qlonglong);
		void handleGotFiles (const QList<DriveItem>& files);
		void handleGotNewItem (const DriveItem& item);
	};
}
}
}
