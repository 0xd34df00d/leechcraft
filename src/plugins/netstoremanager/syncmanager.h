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
#include <QVariant>
#include <QStringList>
#include <QFileSystemWatcher>

class QTimer;

namespace LeechCraft
{
namespace NetStoreManager
{
	class IStorageAccount;
	class AccountsManager;

	class SyncManager : public QObject
	{
		Q_OBJECT

		AccountsManager *AM_;
		QFileSystemWatcher *FileSystemWatcher_;
		QMap<QString, IStorageAccount*> Path2Account_;
		QTimer *Timer_;
		QStringList WatchedPathes_;
	public:
		SyncManager (AccountsManager *am, QObject *parent = 0);
	private:
		QStringList ScanDir (const QString& path);

	public slots:
		void handleDirectoryAdded (const QVariantMap& dirs);
	private slots:
		void handleDirectoryChanged (const QString& path);
		void handleFileChanged (const QString& path);
		void handleTimeout ();
	};
}
}
