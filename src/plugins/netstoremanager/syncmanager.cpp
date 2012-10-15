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
#include <QDateTime>
#include <QtConcurrentRun>
#include <QCryptographicHash>
#include <QStandardItemModel>
#include <QFutureWatcher>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"
#include "xmlsettingsmanager.h"
#include "utils.h"

#ifdef ENABLE_INOTIFY
#include "fileswatcher_inotify.h"
#else
#include "fileswatcher_dummy.h"
#endif

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
	, RemoteStorageCheckingTimeout_ (10000)
	{
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimeout ()));

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
					"Release",
					Qt::QueuedConnection);
		Thread_->exit ();
	}

	void SyncManager::CreateDirectory (const QString& path)
	{
		QDir ().mkpath (path);
	}

	namespace
	{
		DownloadParams CountFileHash (const QString& path, IStorageAccount *isa, QString remoteHash)
		{
			QFile file (path);
			if (!file.open (QIODevice::ReadOnly))
				return DownloadParams ();

			DownloadParams dp;
			dp.Account_ = isa;
			dp.Path_ = path;
			dp.LocalHash_ = QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5).toHex ();
			dp.RemoteHash_ = remoteHash;

			return dp;
		}
	}

	void SyncManager::DownloadFile (const QString& path, const QStringList& id,
			const QDateTime& modifiedDate, const QString& hash,
			IStorageAccount *isa)
	{
		QFileInfo fi (path);
		if (fi.exists ())
		{
			if (fi.lastModified () > modifiedDate)
			{
				if (!hash.isEmpty ())
				{
					QFutureWatcher<DownloadParams> *watcher = new QFutureWatcher<DownloadParams> (this);
					connect (watcher,
							SIGNAL (finished ()),
							this,
							SLOT (finishedHashCounting ()));
					QFuture<DownloadParams> f = QtConcurrent::run (CountFileHash, path, isa, hash);
					watcher->setFuture (f);
				}
				else
				{
					DownloadParams dp;
					dp.Account_ = isa;
					dp.Path_ = path;
					dp.LocalHash_ = QString ();
					dp.RemoteHash_ = hash;
					finishedHashCounting (dp);
				}
			}
		}
		else
			isa->Download (id, path, true);
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
				qWarning () << Q_FUNC_INFO
						<< e.what ();
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
		}

		Timer_->start (RemoteStorageCheckingTimeout_);
		handleTimeout ();
		QueueCheckTimer_->start (1000);
	}

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

			const QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			const QString& parentPath = QFileInfo (remotePath).dir ().absolutePath ();
			const QString& dirName = QFileInfo (path).fileName ();

			auto map = Isfl2PathId_ [isfl];
			if (map.contains (remotePath))
				continue;

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

			const QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();
			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			auto map = Isfl2PathId_ [isfl];
			if (map.contains (remotePath))
				continue;

			const QString& parentPath = QFileInfo (remotePath).dir ().absolutePath ();
			qDebug () << Q_FUNC_INFO << remotePath << map [remotePath] << parentPath << map [parentPath];
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


			auto map = Isfl2PathId_ [isfl];
			const QString rootDirPath = QFileInfo (basePath).dir ().absolutePath ();

			QString remotePath = path;
			remotePath.remove (0, rootDirPath.length ());
			if (!map.contains (remotePath))
				continue;

			isfl->GetListingOps () & TrashSupporting ?
				isfl->MoveToTrash ({ map.take (remotePath) }) :
				isfl->Delete ({ map.take (remotePath) }, false);
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
			if (!map.contains (remotePath))
				continue;

			isfl->GetListingOps () & TrashSupporting ?
				isfl->MoveToTrash ({ map.take (remotePath) }) :
				isfl->Delete ({ map.take (remotePath) }, false);
		}
	}

	void SyncManager::handleEntryWasRenamed (const QString& oldPath,
			const QString& newPath)
	{
		//TODO check on double action
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
		//TODO check on double action
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

		QMap<QString, QStringList>& map = Isfl2PathId_ [isfl];

		QList<QStandardItem*> parents;
		QList<QStandardItem*> children;

		for (const auto& row : items)
			parents << row.value (0);

		while (!parents.isEmpty ())
		{
			for (auto parentItem : parents)
			{
				QString path = parentItem->parent () ?
					map.key (parentItem->parent ()->data (ListingRole::ID).toStringList ()):
					QString ();

				const QString relativePath = path.isEmpty () ?
					"/" + parentItem->text () :
					path + "/" + parentItem->text ();

				QFileInfo baseDirFI (dirPath);
				const QString baseDir = "/" + baseDirFI.fileName ();
				if (relativePath != baseDir &&
						!relativePath.startsWith (baseDir + "/"))
					continue;

				const QString resultPath = baseDirFI.dir ().absolutePath () +
						relativePath;
				map [relativePath] = parentItem->data (ListingRole::ID).toStringList ();

				if (relativePath != baseDir)
					parentItem->data (ListingRole::Directory).toBool () ?
						CreateDirectory (resultPath) :
						DownloadFile (resultPath,
								parentItem->data (ListingRole::ID).toStringList (),
								parentItem->data (ListingRole::ModifiedDate).toDateTime (),
								parentItem->data (ListingRole::Hash).toString (),
								isa);

				for (int i = 0; i < parentItem->rowCount (); ++i)
					children << parentItem->child (i);
			}
			std::swap (parents, children);
			children.clear ();
		}

		QStringList paths = Utils::ScanDir (QDir::NoDotAndDotDot | QDir::Dirs,
				dirPath, true);
		QMetaObject::invokeMethod (FilesWatcher_,
				"AddPath",
				Q_ARG (QString, dirPath));
		QMetaObject::invokeMethod (FilesWatcher_,
				"AddPathes",
				Q_ARG (QStringList, paths));
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

	void SyncManager::finishedHashCounting (const DownloadParams& downloadParams)
	{
		auto watcher = dynamic_cast<QFutureWatcher<DownloadParams>*> (sender ());
		DownloadParams dp = !watcher ?
			downloadParams :
			watcher->result ();
		if (!dp.RemoteHash_.isEmpty () &&
				dp.RemoteHash_ == dp.LocalHash_)
			return;

		IStorageAccount *isa = dp.Account_;
		QString path = dp.Path_;

		auto isfl = qobject_cast<ISupportFileListings*> (isa->GetObject ());
		if (!isfl)
			return;

		const QString basePath = Path2Account_.key (isa);
		QString remotePath = path;
		remotePath.remove (0, basePath.length ());
		QFileInfo fi (remotePath);
		isa->Upload (path,
				Isfl2PathId_ [isfl] [fi.dir ().absolutePath ()],
				UploadType::Update, Isfl2PathId_ [isfl] [remotePath]);
	}

}
}

