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
#include <QMultiMap>
#include "fileswatcherbase.h"

class QTimer;

namespace LeechCraft
{
namespace NetStoreManager
{
	class FilesWatcherInotify : public FilesWatcherBase
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
		QTimer *Timer_;
	public:
		FilesWatcherInotify (QObject *parent = 0);
	private:
		void HandleNotification (int descriptor);
		void AddPathWithNotify (const QString& path);
		bool IsInExceptionList (const QString& path) const;
		void RemoveWatchingPath (int descriptor);

	public slots:
		void checkNotifications ();
		bool addPath (QString path);
		void addPathes (QStringList paths);
		void release ();
		void updateExceptions (QStringList masks);
	};

	typedef FilesWatcherInotify FilesWatcher;
}
}
