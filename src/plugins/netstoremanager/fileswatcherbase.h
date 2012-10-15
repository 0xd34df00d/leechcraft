/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012	Georg Rudoy
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

#pragma once

#include <QObject>

namespace LeechCraft
{
namespace NetStoreManager
{
	class FilesWatcherBase : public QObject
	{
		Q_OBJECT
	public:
		FilesWatcherBase (QObject* = 0);
	public slots:
		virtual void checkNotifications () = 0;
		virtual bool addPath (QString path) = 0;
		virtual void addPathes (QStringList paths) = 0;
		virtual void release () = 0;
		virtual void updateExceptions (QStringList masks) = 0;
	signals:
		void dirWasCreated (const QString& path);
		void fileWasCreated (const QString& path);
		void dirWasRemoved (const QString& path);
		void fileWasRemoved (const QString& path);
		void fileWasUpdated (const QString& path);
		void entryWasRenamed (const QString& oldName, const QString& newName);
		void entryWasMoved (const QString& oldPath, const QString& newPath);
	};
}
}
