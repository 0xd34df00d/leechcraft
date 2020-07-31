/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QList>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <interfaces/structures.h>

class IEntityManager;

namespace LC
{
namespace Nacheku
{
	/** Watches a given directory for files which could be handled.
	 */
	class DirectoryWatcher : public QObject
	{
		Q_OBJECT

		IEntityManager * const IEM_;
		std::unique_ptr<QFileSystemWatcher> Watcher_;
		QList<QFileInfo> Olds_;
	public:
		/** Constructs the watcher, registers it as a client for
		 * "WatchDirectory" setting in the XML settings manager (the
		 * settingsChanged() is used for that).
		 *
		 * @param[in] parent The parent object.
		 */
		DirectoryWatcher (IEntityManager*, QObject *parent = 0);
	private slots:
		/** Handles the changed settings. Removes previous directory
		 * from the watch list and adds a new one. Checks the new
		 * directory for entities.
		 */
		void settingsChanged ();

		/** Handles the changes in the directory. Compares the
		 * QFileInfos from previous run with current ones and emits
		 * an entity for each file which has been added or modified
		 * since the previous check.
		 *
		 * @param[in] dir The directory that changed.
		 */
		void handleDirectoryChanged (const QString& dir);
	};
}
}

