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

	void SyncManager::handleDirectoryAdded (const QVariantMap& dirs)
	{
		FileSystemWatcher_->removePaths (FileSystemWatcher_->directories ());

		for (const auto& key : dirs.keys ())
		{
			const QString& path = dirs [key].toString ();
			Path2Account_ [path] = AM_->GetAccountFromUniqueID (key);
			FileSystemWatcher_->addPath (path);
			qDebug () << "watching directory "
					<< path;
		}

		// check for changes every minute
// 		Timer_->start (60000);
// 		handleTimeout ();
	}

	void SyncManager::handleDirectoryChanged (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
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

