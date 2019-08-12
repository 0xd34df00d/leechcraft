/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/bimap.hpp>
#include <boost/container/allocator.hpp>
#include <QObject>
#include <QStringList>
#include <QMultiMap>
#include "fileswatcherbase.h"

class QTimer;

namespace LC
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

		typedef boost::bimaps::bimap<QString, int, boost::container::allocator<void>> descriptorsMap;
		descriptorsMap WatchedPathes2Descriptors_;

		QStringList ExceptionMasks_;
		QTimer *Timer_;
	public:
		FilesWatcherInotify (QObject *parent = 0);
	private:
		void AddPath (const QString& path);
		void RenamePath (const QString& oldPath, const QString& newPath);
		void HandleNotification (int descriptor);
		void AddPathWithNotify (const QString& path);
		bool IsInExceptionList (const QString& path) const;
		void RemoveWatchingPath (int descriptor);
		void RemoveWatchingPath (const QString& path);

	public slots:
		void updatePaths (const QStringList& paths);

		void checkNotifications ();
		void release ();
		void updateExceptions (QStringList masks);
	};
}
}
