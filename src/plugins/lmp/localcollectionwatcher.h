/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QHash>
#include <QSet>
#include <QStringList>

class QFileSystemWatcher;
class QTimer;

namespace LeechCraft
{
namespace LMP
{
	class LocalCollectionWatcher : public QObject
	{
		Q_OBJECT

		QFileSystemWatcher *Watcher_;
		QHash<QString, QStringList> Dir2Subdirs_;

		QList<QString> ScheduledDirs_;
		QTimer *ScanTimer_;
	public:
		LocalCollectionWatcher (QObject* = 0);

		void AddPath (const QString&);
		void RemovePath (const QString&);
	private:
		void ScheduleDir (const QString&);
	private slots:
		void handleSubdirsCollected ();
		void handleDirectoryChanged (const QString&);
		void rescanQueue ();
	};
}
}
