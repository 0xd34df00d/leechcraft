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

#pragma once

#include <boost/bimap.hpp>
#include <QObject>
#include <QStringList>

class QTimer;

namespace LeechCraft
{
namespace NetStoreManager
{
	class FilesWatcher : public QObject
	{
		Q_OBJECT

		int INotifyDescriptor_;
		const uint32_t WatchMask_;
		const int  WaitMSecs_;
		size_t BufferLength_;
		size_t EventSize_;

		typedef boost::bimaps::bimap<QString, int> descriptorsMap;
		descriptorsMap WatchedPathes2Descriptors_;

		QStringList ExceptionMasks_;
	public:
		FilesWatcher (QObject *parent = 0);

		void AddPath (const QString& path);
		void AddPathes (const QStringList& pathes);

		void Release ();
		void UpdateExceptions (const QStringList& masks);
	private:
		void HandleNotification (int descriptor);
		void AddPathWithNotify (const QString& path);

	public slots:
		void checkNotifications ();
		void handleThreadStarted ();

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
