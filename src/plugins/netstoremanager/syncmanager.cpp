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

#include "syncmanager.h"
#include <QtDebug>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QThread>
#include <QStandardItemModel>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"
#include "fileswatcher.h"
#include "xmlsettingsmanager.h"
#include "utils.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncManager::SyncManager (AccountsManager *am, QObject *parent)
	: QObject (parent)
	, AM_ (am)
	, Timer_ (new QTimer (this))
	, Thread_ (new QThread (this))
	, FilesWatcher_ (0)
	, QueueCheckTimer_ (new QTimer (this))
	{
// 		connect (Timer_,
// 				SIGNAL (timeout ()),
// 				this,
// 				SLOT (handleTimeout ()));

		connect (QueueCheckTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkApiCallQueue ()));

		XmlSettingsManager::Instance ().RegisterObject ("ExceptionsList",
				this, "handleUpdateExceptionsList");
	}

	void SyncManager::Release ()
	{
		if (FilesWatcher_)
			QMetaObject::invokeMethod (FilesWatcher_,
					"Release");
		Thread_->exit ();
	}

	void SyncManager::handleDirectoryAdded (const QVariantMap& dirs)
	{
		if (!FilesWatcher_)
		{
			try
			{
				FilesWatcher_ = new FilesWatcher;
				FilesWatcher_->moveToThread (Thread_);
				Thread_->start ();

				connect (FilesWatcher_,
						SIGNAL (dirWasCreated (QString)),
						this,
						SLOT (handleDirWasCreated (QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (fileWasCreated (QString)),
						this,
						SLOT (handleFileWasCreated (QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (dirWasRemoved (QString)),
						this,
						SLOT (handleDirWasRemoved (QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (fileWasRemoved (QString)),
						this,
						SLOT (handleFileWasRemoved (QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (entryWasRenamed (QString, QString)),
						this,
						SLOT (handleEntryWasRenamed (QString, QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (entryWasMoved (QString, QString)),
						this,
						SLOT (handleEntryWasMoved (QString, QString)),
						Qt::QueuedConnection);
				connect (FilesWatcher_,
						SIGNAL (fileWasUpdated (QString)),
						this,
						SLOT (handleFileWasUpdated (QString)),
						Qt::QueuedConnection);
			}
			catch (const std::exception& e)
			{
				FilesWatcher_ = 0;
				qWarning () << e.what ();
				return;
			}
		}

		handleUpdateExceptionsList ();
		for (const auto& key : dirs.keys ())
		{
			const QString& dirPath = dirs [key].toString ();
			auto isa = AM_->GetAccountFromUniqueID (key);
			auto isfl = qobject_cast<ISupportFileListings*> (isa->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< isa->GetObject ()
						<< "isn't an ISupportFileListings. Synchronization will not work.";
				continue;
			}

			connect (isa->GetObject (),
					SIGNAL (gotListing (QList<QList<QStandardItem*>>)),
					this,
					SLOT (handleGotListing (QList<QList<QStandardItem*>>)));
			connect (isa->GetObject (),
					SIGNAL (gotNewItem (QList<QStandardItem*>, QStringList)),
					this,
					SLOT (handleGotNewItem (QList<QStandardItem*>, QStringList)));
			isfl->RefreshListing ();
			Path2Account_ [dirPath] = isa;
			qDebug () << "watching directory "
					<< dirPath;
// 			isfl->CheckForSyncUpload (pathes, dirPath);
		}

		// check for changes every minute
// 		Timer_->start (60000);
// 		handleTimeout ();
		QueueCheckTimer_->start (1000);
	}

// 	void SyncManager::handleDirectoryChanged (const QString& path)
// 	{
// 		QStringList pathesInDir = ScanDir (path, false);
// 		qDebug () << "pathes in dir" << pathesInDir;
// // 		QStringList watchedFiles = WatchedPathes_;
// // 		QDir dir (path);
// //
// // 		QStringList baseDirs;
// // 		for (const auto& str : Path2Account_.keys ())
// // 			if (path.contains (str))
// // 			{
// // 				baseDirs << str;
// // 				WatchedPathes_ << ScanDir (str);
// // 			}
// //
// // 		//check for removed files
// // 		QStringList removedFiles;
// // 		std::set_difference (watchedFiles.begin (), watchedFiles.end (),
// // 				WatchedPathes_.begin (), WatchedPathes_.end (),
// // 				std::back_inserter (removedFiles));
// //
// // 		//check for adding files
// // 		for (const auto& info : dir.entryInfoList (QDir::AllEntries | QDir::NoDotAndDotDot))
// // 		{
// // 			if (!watchedFiles.contains (info.absoluteFilePath ()))
// // 				FileSystemWatcher_->addPath (info.absoluteFilePath ());
// // 		}
// //
// // 		for (const auto& str : Path2Account_.keys ())
// // 		{
// // 			if (path.contains (str))
// // 			{
// // 				auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [str]->GetObject ());
// // 				isfl->CheckForSyncUpload (FileSystemWatcher_->files (), str);
// // 			}
// // 		}
// 	}

	void SyncManager::handleTimeout ()
	{
		for (auto account : Path2Account_.values ())
		{
			if (!(account->GetAccountFeatures () & FileListings))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (account->GetObject ());
		}
	}

	void SyncManager::handleUpdateExceptionsList ()
	{
		QStringList masks = XmlSettingsManager::Instance ()
				.property ("ExceptionsList").toStringList ();

		if (FilesWatcher_)
			QMetaObject::invokeMethod (FilesWatcher_,
					"UpdateExceptions",
					Q_ARG (QStringList, masks));
	}

	void SyncManager::handleDirWasCreated (const QString& path)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!path.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}

			QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			auto map = Isfl2PathId_ [isfl];

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			const QString& parentPath = QFileInfo (remotePath).dir ().absolutePath ();
			const QString& dirName = QFileInfo (path).fileName ();

			if (map.contains (parentPath))
				isfl->CreateDirectory (dirName, map [parentPath]);
			else
				ApiCallQueue_ << [this, path] () { handleDirWasCreated (path); };
		}
	}

	void SyncManager::handleFileWasCreated (const QString& path)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!path.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}

			QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			auto map = Isfl2PathId_ [isfl];

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			const QString& parentPath = QFileInfo (remotePath).dir ().absolutePath ();
			if (map.contains (parentPath))
				emit uploadRequested (Path2Account_ [basePath], path, map [parentPath]);
			else
				ApiCallQueue_ << [this, path] () { handleFileWasCreated (path); };
		}
	}

	void SyncManager::handleDirWasRemoved (const QString& path)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!path.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}

			QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			auto map = Isfl2PathId_ [isfl];

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			isfl->GetListingOps () & TrashSupporting ?
				isfl->MoveToTrash ({ map [remotePath] }) :
				isfl->Delete ({ map [remotePath] }, false);
		}
	}

	void SyncManager::handleFileWasRemoved (const QString& path)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!path.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}

			QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			auto map = Isfl2PathId_ [isfl];

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			isfl->GetListingOps () & TrashSupporting ?
				isfl->MoveToTrash ({ map [remotePath] }) :
				isfl->Delete ({ map [remotePath] }, false);
		}
	}

	void SyncManager::handleEntryWasRenamed (const QString& oldPath,
			const QString& newPath)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!oldPath.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}

			QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			auto map = Isfl2PathId_ [isfl];

			QString remotePath = oldPath, newRemotePath = newPath;
			remotePath.remove (0, rootDirPath.length ());
			newRemotePath.remove (0, rootDirPath.length ());
			isfl->Rename (map.take (remotePath), QFileInfo (newPath).fileName ());
		}
	}

	void SyncManager::handleEntryWasMoved (const QString& oldPath,
			const QString& newPath)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (oldPath.startsWith (basePath) &&
					newPath.startsWith (basePath))
			{
				auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
				if (!isfl)
				{
					qWarning () << Q_FUNC_INFO
							<< Path2Account_ [basePath]->GetObject ()
							<< "isn't an ISupportFileListings";
					continue;
				}

				QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
				auto map = Isfl2PathId_ [isfl];

				QString remotePath = oldPath, newRemotePath = newPath;
				remotePath.remove (0, rootDirPath.length ());
				newRemotePath.remove (0, rootDirPath.length ());
				const QString& newParentPath = QFileInfo (newRemotePath).dir ().absolutePath ();

				if (map.contains (remotePath) &&
						map.contains (newParentPath))
				{
					QStringList id = map.take (remotePath);
					isfl->Move (id, map [newParentPath]);
					map [newRemotePath] = id;
				}
				else
					ApiCallQueue_ << [this, oldPath, newPath] ()
						{ handleEntryWasMoved (oldPath, newPath); };
			}
			else if (oldPath.startsWith (basePath) &&
					!newPath.startsWith (basePath))
			{
				QFileInfo (oldPath).isDir () ?
					handleDirWasRemoved (oldPath) :
					handleFileWasRemoved (oldPath);
			}
			else if (!oldPath.startsWith (basePath) &&
					newPath.startsWith (basePath))
			{
				QFileInfo (oldPath).isDir () ?
					handleDirWasCreated (oldPath) :
					handleFileWasCreated (oldPath);
			}
		}
	}

	void SyncManager::handleFileWasUpdated (const QString& path)
	{
		for (const auto& basePath : Path2Account_.keys ())
		{
			if (!path.startsWith (basePath))
				continue;

			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [basePath]->GetObject ());
			if (!isfl)
			{
				qWarning () << Q_FUNC_INFO
						<< Path2Account_ [basePath]->GetObject ()
						<< "isn't an ISupportFileListings";
				continue;
			}
		}
		qDebug () << Q_FUNC_INFO << path;
	}

	void SyncManager::handleGotListing (const QList<QList<QStandardItem*>>& items)
	{
		auto isa = qobject_cast<IStorageAccount*> (sender ());
		if (!isa)
			return;

		auto isfl = qobject_cast<ISupportFileListings*> (sender ());
		if (!isfl)
			return;

		QString dirPath = Path2Account_.key (isa);
		if (dirPath.isEmpty ())
			return;

		QMap<QString, QStringList> map;

		QList<QStandardItem*> parents;
		QList<QStandardItem*> children;

		for (const auto& row : items)
			parents << row [0];

		while (!parents.isEmpty ())
		{
			for (auto parentItem : parents)
			{
				QString path = parentItem->parent () ?
					map.key (parentItem->parent ()->data (ListingRole::ID).toStringList ()):
					QString ();
				if (path.isEmpty ())
					map ["/" + parentItem->text ()] =
							parentItem->data (ListingRole::ID).toStringList ();
				else
					map [path + "/" + parentItem->text ()] =
							parentItem->data (ListingRole::ID).toStringList ();

				for (int i = 0; i < parentItem->rowCount (); ++i)
					children << parentItem->child (i);
			}

			auto tempItems = parents;
			parents = children;
			children = tempItems;

			children.clear ();
		}

		Isfl2PathId_ [isfl] = map;

		QStringList pathes = Utils::ScanDir (QDir::NoDotAndDotDot | QDir::Dirs,
				dirPath, true);
		QMetaObject::invokeMethod (FilesWatcher_,
				"AddPath",
				Q_ARG (QString, dirPath));
		QMetaObject::invokeMethod (FilesWatcher_,
				"AddPathes",
				Q_ARG (QStringList, pathes));
	}

	void SyncManager::handleGotNewItem (const QList<QStandardItem*>& item,
			const QStringList& parentId)
	{
		auto isfl = qobject_cast<ISupportFileListings*> (sender ());
		if (!isfl)
			return;

		auto map = Isfl2PathId_ [isfl];

		if (map.values ().contains (parentId))
		{
			QString path = map.key (parentId);
			map [path + "/" + item [0]->text ()] =
					item [0]->data (ListingRole::ID).toStringList ();
		}

		Isfl2PathId_ [isfl] = map;
	}

	void SyncManager::checkApiCallQueue ()
	{
		if (!ApiCallQueue_.isEmpty ())
			ApiCallQueue_.dequeue() ();
	}

}
}

