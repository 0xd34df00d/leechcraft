/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QQueue>

namespace LC
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
