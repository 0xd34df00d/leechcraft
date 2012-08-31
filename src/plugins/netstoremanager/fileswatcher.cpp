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

#include "fileswatcher.h"
#include <QtDebug>
#include <QStringList>
#include <QTimer>
#include <sys/inotify.h>
#include <stdexcept>

namespace LeechCraft
{
	namespace NetStoreManager
	{
		FilesWatcher::FilesWatcher (QObject *parent)
		: QObject (parent)
		, INotifyDescriptor_ (inotify_init ())
		, WatchMask_ (IN_ALL_EVENTS)
		, EventSize_ (sizeof (struct inotify_event))
		, BufferLength_ (1024 * (EventSize_ + 16))
		, Timer_ (new QTimer (this))
		{
			if (INotifyDescriptor_ < 0)
				throw std::runtime_error ("inotify_init faild. Synchronization will not work.");

			FD_ZERO (&WatchedDescriptors_);
			WaitTime_.tv_sec = 0;
			WaitTime_.tv_usec = 50;

			connect (Timer_,
					SIGNAL (timeout ()),
					this,
					SLOT (checkNotifications ()),
					Qt::QueuedConnection);
		}

		FilesWatcher::~FilesWatcher ()
		{
			for (auto fd : WatchedPathes2Descriptors_.values ())
				inotify_rm_watch (INotifyDescriptor_, fd);

			close (INotifyDescriptor_);
		}

		void FilesWatcher::AddPath (const QString& path)
		{
			int fd = inotify_add_watch (INotifyDescriptor_, path.toUtf8 (), WatchMask_);
			FD_SET (fd, &WatchedDescriptors_);
			WatchedPathes2Descriptors_ [path] = fd;
			LastDescriptor_ = fd;

			if (!Timer_->isActive ())
				Timer_->start (1000);
		}

		void FilesWatcher::AddPathes (const QStringList& pathes)
		{
			for (const auto & path : pathes)
				AddPath (path);
		}

		void FilesWatcher::HandleNotification (int descriptor)
		{
			char buffer [BufferLength_];
			ssize_t length = read (descriptor, buffer, BufferLength_);

			if (length < 0)
			{
				qDebug () << "read error";
				return;
			}

			int i = 0;

			while (i < length)
			{
				struct inotify_event *event = reinterpret_cast<struct inotify_event*> (&buffer [i]);

				if (event->len)
				{
					if (event->mask & IN_CREATE)
					{
						if (event->mask & IN_ISDIR)
						{
							qDebug () << "The directory"
									<< event->name
									<< "was created";
						}
						else
						{
							qDebug () << "The file"
									<< event->name
									<< "was created";
						}
					}
					else if (event->mask & IN_DELETE)
					{
						if (event->mask & IN_ISDIR)
						{
							qDebug () << "The directory"
									<< event->name
									<< "was deleted";
						}
						else
						{
							qDebug () << "The file"
									<< event->name
									<< "was deleted";
						}
					}
					else if (event->mask & IN_MODIFY)
					{
						if (event->mask & IN_ISDIR)
						{
							qDebug () << "The directory"
									<< event->name
									<< "was modified";
						}
						else
						{
							qDebug () << "The file"
									<< event->name
									<< "was modified";
						}
					}
					else if (event->mask & IN_ATTRIB)
					{
						qDebug () << "The file"
								<< event->name
								<< "change attribute";
					}
					else if (event->mask & IN_MOVED_FROM)
					{
						qDebug () << "The file"
								<< event->name
								<< "change move from with cookie"
								<< event->cookie;
					}
					else if (event->mask & IN_MOVED_TO)
					{
						qDebug () << "The file"
								<< event->name
								<< "change move to with cookie"
								<< event->cookie;
					}
				}

				i += EventSize_ + event->len;
			}
		}

		void FilesWatcher::checkNotifications ()
		{
// 		int res = select (LastDescriptor_ + 1,
// 				&WatchedDescriptors_,
// 				NULL,
// 				NULL,
// 				&WaitTime_);
//
// 		qDebug () << res;
// 		if (res < 0)
// 		{
// 			qDebug () << "error";
// 		}
// 		else if (!res)
// 		{
// 		}
// 		else
// 		{
			qDebug () << "handle";
			HandleNotification (INotifyDescriptor_);
// 		}
		}
	}
}