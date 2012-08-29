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
#include <boost/concept_check.hpp>
#include "accountsmanager.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncManager::SyncManager (AccountsManager *am, QObject *parent)
	: QObject (parent)
	, AM_ (am)
	, FileSystemWatcher_ (new QFileSystemWatcher (this))
	, Timer_ (new QTimer (this))
	{
		connect (FileSystemWatcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (handleDirectoryChanged (QString)));
		connect (FileSystemWatcher_,
				SIGNAL (fileChanged (QString)),
				this,
				SLOT (handleFileChanged (QString)));
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimeout ()));
	}

	QStringList SyncManager::ScanDir (const QString& path)
	{
		QDir baseDir (path);
		QStringList pathes;
		for (const auto& entry : baseDir.entryInfoList (QDir::AllEntries | QDir::NoDotAndDotDot))
		{
			pathes << entry.absoluteFilePath ();
			if (entry.isDir ())
				pathes << ScanDir (entry.absoluteFilePath ());
		}
		return pathes;
	}

	void SyncManager::handleDirectoryAdded (const QVariantMap& dirs)
	{
		if (!FileSystemWatcher_->directories ().isEmpty ())
			FileSystemWatcher_->removePaths (FileSystemWatcher_->directories ());

		for (const auto& key : dirs.keys ())
		{
			const QString& dirPath = dirs [key].toString ();
			Path2Account_ [dirPath] = AM_->GetAccountFromUniqueID (key);
			qDebug () << "watching directory "
					<< dirPath;
			QStringList pathes = ScanDir (dirPath);
			FileSystemWatcher_->addPaths (pathes);
			auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [dirPath]->GetObject ());
			isfl->CheckForSyncUpload (pathes, dirPath);
			WatchedPathes_ << pathes;
		}

		// check for changes every minute
		Timer_->start (60000);
		handleTimeout ();
	}

	void SyncManager::handleDirectoryChanged (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
		QStringList watchedFiles = WatchedPathes_;
		QDir dir (path);

		QStringList baseDirs;
		for (const auto& str : Path2Account_.keys ())
			if (path.contains (str))
			{
				baseDirs << str;
				WatchedPathes_ << ScanDir (str);
			}

		//check for removed files
		QStringList removedFiles;
		std::set_difference (watchedFiles.begin (), watchedFiles.end (),
				WatchedPathes_.begin (), WatchedPathes_.end (),
				std::back_inserter (removedFiles));

		//check for adding files
		for (const auto& info : dir.entryInfoList (QDir::AllEntries | QDir::NoDotAndDotDot))
		{
			if (!watchedFiles.contains (info.absoluteFilePath ()))
				FileSystemWatcher_->addPath (info.absoluteFilePath ());
		}

		for (const auto& str : Path2Account_.keys ())
		{
			if (path.contains (str))
			{
				auto isfl = qobject_cast<ISupportFileListings*> (Path2Account_ [str]->GetObject ());
				isfl->CheckForSyncUpload (FileSystemWatcher_->files (), str);
			}
		}
	}

	void SyncManager::handleFileChanged (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

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

