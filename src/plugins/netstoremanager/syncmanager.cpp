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
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"
#include "fileswatcher.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncManager::SyncManager (AccountsManager *am, QObject *parent)
	: QObject (parent)
	, AM_ (am)
	, Timer_ (new QTimer (this))
	, FilesWatcher_ (0)
	{
// 		connect (Timer_,
// 				SIGNAL (timeout ()),
// 				this,
// 				SLOT (handleTimeout ()));
	}

	void SyncManager::Release ()
	{
		FilesWatcher_->Release ();
	}

	namespace
	{
		QStringList ScanDir (QDir::Filters filter, const QString& path, bool recursive)
		{
			QDir baseDir (path);
			QStringList pathes;
			for (const auto& entry : baseDir.entryInfoList (filter))
			{
				pathes << entry.absoluteFilePath ();
				if (recursive &&
						entry.isDir ())
					pathes << ScanDir (filter, entry.absoluteFilePath (), recursive);
			}
			return pathes;
		}
	}

	void SyncManager::handleDirectoryAdded (const QVariantMap& dirs)
	{
		if (!FilesWatcher_)
		{
			try
			{
				FilesWatcher_ = new FilesWatcher;
			}
			catch (const std::exception& e)
			{
				FilesWatcher_ = 0;
				qWarning () << e.what ();
				return;
			}
		}

		for (const auto& key : dirs.keys ())
		{
			const QString& dirPath = dirs [key].toString ();
			Path2Account_ [dirPath] = AM_->GetAccountFromUniqueID (key);
			qDebug () << "watching directory "
					<< dirPath;

			QStringList pathes = ScanDir (QDir::NoDotAndDotDot | QDir::Dirs, dirPath, true);
			FilesWatcher_->AddPathes (pathes);
			FilesWatcher_->AddPath (dirPath);
			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [dirPath]->GetObject ());
// 			isfl->CheckForSyncUpload (pathes, dirPath);
		}

		// check for changes every minute
// 		Timer_->start (60000);
// 		handleTimeout ();
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
			isfl->RequestFileChanges ();
		}
	}

}
}

