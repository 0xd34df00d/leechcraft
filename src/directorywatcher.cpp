/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "directorywatcher.h"
#include <QDir>
#include <QTimer>
#include <QUrl>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	DirectoryWatcher::DirectoryWatcher (QObject *parent)
	: QObject (parent)
	, Watcher_ (new QFileSystemWatcher)
	{
		XmlSettingsManager::Instance ()->RegisterObject ("WatchDirectory",
				this,
				"settingsChanged");

		QTimer::singleShot (5000,
				this,
				SLOT (settingsChanged ()));

		connect (Watcher_.get (),
				SIGNAL (directoryChanged (const QString&)),
				this,
				SLOT (handleDirectoryChanged (const QString&)),
				Qt::QueuedConnection);
	}

	void DirectoryWatcher::settingsChanged ()
	{
		QString path = XmlSettingsManager::Instance ()->
			property ("WatchDirectory").toString ();
		QStringList dirs = Watcher_->directories ();
		if (dirs.size () == 1 &&
				dirs.at (0) == path)
			return;

		if (!dirs.isEmpty ())
			Watcher_->removePaths (dirs);

		if (!path.isEmpty ())
		{
			QDir dir (path);
			Olds_ = dir.entryInfoList (QDir::Files);

			Watcher_->addPath (path);
			handleDirectoryChanged (path);
		}
	}

	void DirectoryWatcher::handleDirectoryChanged (const QString& path)
	{
		QDir dir (path);
		QList<QFileInfo> cur = dir.entryInfoList (QDir::Files);
		QList<QFileInfo> nl = cur;

		Q_FOREACH (const QFileInfo& oldFi, Olds_)
		{
			QString fname = oldFi.absoluteFilePath ();
			Q_FOREACH (const QFileInfo& newFi, nl)
				if (newFi.absoluteFilePath () == fname)
				{
					nl.removeAll (newFi);
					break;
				}
		}

		Olds_ = cur;

		Q_FOREACH (const QFileInfo& newFi, nl)
			emit gotEntity (Util::MakeEntity (QUrl::fromLocalFile (newFi.absoluteFilePath ()),
						path,
						FromUserInitiated));
	}
};

