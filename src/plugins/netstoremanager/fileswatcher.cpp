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
#include <sys/poll.h>
#include <stdexcept>

namespace LeechCraft
{
	namespace NetStoreManager
	{
		FilesWatcher::FilesWatcher (QObject *parent)
		: QObject (parent)
		, WatchMask_ (IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO )
		, WaitMSecs_ (50)
		, Timer_ (new QTimer (this))
		{
			INotifyDescriptor_ = inotify_init ();
			if (INotifyDescriptor_ < 0)
				throw std::runtime_error ("inotify_init faild. Synchronization will not work.");

			EventSize_ = sizeof (struct inotify_event);
			BufferLength_ = 1024 * (EventSize_ + 16);

			connect (Timer_,
					SIGNAL (timeout ()),
					this,
					SLOT (checkNotifications ()));
		}

		void FilesWatcher::AddPath (const QString& path)
		{
			int fd = inotify_add_watch (INotifyDescriptor_, path.toUtf8 (), WatchMask_);
			WatchedPathes2Descriptors_.insert (descriptorsMap::value_type (path, fd));

			qDebug () << "added path"
					<< path;

			if (!Timer_->isActive ())
				Timer_->start (1000);
		}

		void FilesWatcher::AddPathes (const QStringList& pathes)
		{
			for (const auto & path : pathes)
				AddPath (path);
		}

		void FilesWatcher::Release ()
		{
			for (auto map : WatchedPathes2Descriptors_.left)
				inotify_rm_watch (INotifyDescriptor_, map.second);

			close (INotifyDescriptor_);
		}

		void FilesWatcher::UpdateExceptions (const QStringList& masks)
		{
			ExceptionMasks_ = masks;
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

			QList<inotify_event*> eventsBuffer;
			while (i < length)
			{
				struct inotify_event *event = reinterpret_cast<struct inotify_event*> (&buffer [i]);

				QString path = WatchedPathes2Descriptors_.right.at (event->wd);
				QString fullPath = path + "/" + QString (event->name);

				//TODO if in exception list - ignore

				if (event->mask & IN_CREATE)
				{
					if (event->mask & IN_ISDIR)
					{
						if (!path.isEmpty ())
						{
							AddPath (fullPath);
							emit dirWasCreated (fullPath);
						}
					}
					else
					{
						emit fileWasCreated (fullPath);
					}
				}
				else if (event->mask & IN_DELETE)
				{
					if (event->mask & IN_ISDIR)
					{
						emit dirWasRemoved (fullPath);
					}
					else
					{
						emit fileWasRemoved (fullPath);
					}
				}
				else if (event->mask & IN_MODIFY)
				{
					if (event->mask & IN_ISDIR)
					{
						//TODO modify directory
						qDebug () << "modify dir"
								<< fullPath
								<< event->wd;
					}
					else
					{
						emit fileWasUpdated (fullPath);
					}
				}
				else if (event->mask & IN_MOVED_FROM)
				{
					eventsBuffer << event;
				}
				else if (event->mask & IN_MOVED_TO)
				{
					if (!eventsBuffer.isEmpty ())
						for (auto e : eventsBuffer)
							if (e->cookie == event->cookie &&
									e->wd == event->wd)
							{
								emit entryWasRenamed (path + "/" + QString (e->name),
										fullPath);
								break;
							}
							else if (e->cookie == event->cookie)
							{
								QString oldPrePath = WatchedPathes2Descriptors_.right.at (e->wd);
								emit entryWasMoved (oldPrePath + "/" + QString (e->name),
										fullPath);
								break;
							}
				}
				else if (event->mask & IN_DELETE_SELF)
				{
					inotify_rm_watch (INotifyDescriptor_, event->wd);
					WatchedPathes2Descriptors_.right.erase (event->wd);
					emit dirWasRemoved (fullPath);
				}
				else if (event->mask & IN_MOVE_SELF)
				{
				}

				i += EventSize_ + event->len;
			}
		}

		void FilesWatcher::checkNotifications ()
		{
			struct pollfd pfd = { INotifyDescriptor_, POLLIN, 0 };
			int res = poll (&pfd, 1, WaitMSecs_);

			if (res < 0)
				qDebug () << "error";
			else if (!res)
			{
			}
			else
				HandleNotification (INotifyDescriptor_);
		}
	}
}