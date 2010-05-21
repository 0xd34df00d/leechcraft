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

#ifndef DIRECTORYWATCHER_H
#define DIRECTORYWATCHER_H
#include <memory>
#include <QObject>
#include <QList>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include "interfaces/structures.h"

namespace LeechCraft
{
	/** Watches a given directory for files which could be handled.
	 */
	class DirectoryWatcher : public QObject
	{
		Q_OBJECT

		std::auto_ptr<QFileSystemWatcher> Watcher_;
		QList<QFileInfo> Olds_;
	public:
		/** Constructs the watcher, registers it as a client for
		 * "WatchDirectory" setting in the XML settings manager (the
		 * settingsChanged() is used for that).
		 *
		 * @param[in] parent The parent object.
		 */
		DirectoryWatcher (QObject *parent = 0);
	private slots:
		/** Handles the changed settings. Removes previous directory
		 * from the watch list and adds a new one. Checks the new
		 * directory for entities.
		 */
		void settingsChanged ();

		/** Handles the changes in the directory. Compares the
		 * QFileInfos from previous run with current ones and emits
		 * gotEntity() for each file which has been added or modified
		 * since the previous check.
		 *
		 * @param[in] dir The directory that changed.
		 */
		void handleDirectoryChanged (const QString& dir);
	signals:
		/** Emitted when a new or modified file is detected.
		 */
		void gotEntity (const LeechCraft::Entity& entity);
	};
};

#endif

