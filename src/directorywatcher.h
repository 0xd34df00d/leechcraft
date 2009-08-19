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
	class DirectoryWatcher : public QObject
	{
		Q_OBJECT

		std::auto_ptr<QFileSystemWatcher> Watcher_;
		QList<QFileInfo> Olds_;
	public:
		DirectoryWatcher (QObject* = 0);
	private slots:
		void settingsChanged ();
		void handleDirectoryChanged (const QString&);
	signals:
		void gotEntity (const LeechCraft::DownloadEntity&);
	};
};

#endif

