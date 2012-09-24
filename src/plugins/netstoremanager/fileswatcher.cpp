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
#include <stdexcept>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <QtDebug>
#include <QStringList>
#include <QTimer>
#include "utils.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	FilesWatcher::FilesWatcher (QObject *parent)
	: QObject (parent)
	, INotifyDescriptor_ (inotify_init ())
	, WatchMask_ (IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY |IN_MOVED_FROM | IN_MOVED_TO )
	, WaitMSecs_ (50)
	, Timer_ (new QTimer (this))
	{
		if (INotifyDescriptor_ < 0)
			throw std::runtime_error ("inotify_init failed. Synchronization will not work.");

		EventSize_ = sizeof (struct inotify_event);
		BufferLength_ = 1024 * (EventSize_ + 16);

		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkNotifications ()));
	}

	bool FilesWatcher::AddPath (QString path)
	{
		int fd = inotify_add_watch (INotifyDescriptor_, path.toUtf8 (), WatchMask_);
		WatchedPathes2Descriptors_.insert ({ path, fd });

		if (!Timer_->isActive ())
			Timer_->start (1000);

		return true;
	}

	void FilesWatcher::AddPathes (QStringList paths)
	{
		for (const auto& path : paths)
			AddPath (path);
	}

	void FilesWatcher::Release ()
	{
		for (auto map : WatchedPathes2Descriptors_.left)
			inotify_rm_watch (INotifyDescriptor_, map.second);

		WatchedPathes2Descriptors_.clear ();
		close (INotifyDescriptor_);
	}

	void FilesWatcher::UpdateExceptions (QStringList masks)
	{
		ExceptionMasks_ = masks;
		ExceptionMasks_.removeAll ("");
		ExceptionMasks_.removeDuplicates ();

		for (const auto& pair : WatchedPathes2Descriptors_.left)
			if (IsInExceptionList (pair.first))
				RemoveWatchingPath (pair.second);
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
			struct inotify_event *event = reinterpret_cast<inotify_event*> (&buffer [i]);

			const QString path = WatchedPathes2Descriptors_.right.at (event->wd);
			const QString fullPath = path + "/" + QString (event->name);

			i += EventSize_ + event->len;

			if (!(event->mask & IN_ISDIR) &&
					IsInExceptionList (fullPath))
				continue;

			if (event->mask & IN_CREATE)
			{
				if (event->mask & IN_ISDIR)
					AddPathWithNotify (fullPath);
				else
					emit fileWasCreated (fullPath);
			}
			else if (event->mask & IN_DELETE)
			{
				if (!(event->mask & IN_ISDIR))
					emit fileWasRemoved (fullPath);
			}
			else if (event->mask & IN_MODIFY)
			{
				if (!(event->mask & IN_ISDIR))
					emit fileWasUpdated (fullPath);
			}
			else if (event->mask & IN_MOVED_FROM)
			{
				eventsBuffer << event;
			}
			else if (event->mask & IN_MOVED_TO)
			{
				if (!eventsBuffer.isEmpty ())
				{
					for (auto e : eventsBuffer)
						if (e->cookie == event->cookie &&
								e->wd == event->wd)
						{
							emit entryWasRenamed (path + "/" + QString (e->name),
									fullPath);
							eventsBuffer.removeAll (e);
							break;
						}
						else if (e->cookie == event->cookie)
						{
							const QString oldPrePath = WatchedPathes2Descriptors_.right.at (e->wd);
							emit entryWasMoved (oldPrePath + "/" + QString (e->name),
									fullPath);
							eventsBuffer.removeAll (e);
							break;
						}
				}
				else
					AddPathWithNotify (fullPath);
			}
			else if (event->mask & IN_DELETE_SELF)
			{
				emit dirWasRemoved (path);
			}

			if (event->mask & IN_IGNORED)
			{
				RemoveWatchingPath (event->wd);
			}
		}

		for (auto e : eventsBuffer)
		{
			const QString path = WatchedPathes2Descriptors_.right.at (e->wd);
			const QString fullPath = path + "/" + QString (e->name);

			if (e->mask & IN_ISDIR)
				emit dirWasRemoved (fullPath);
			else
				emit fileWasRemoved (fullPath);
		}
	}

	void FilesWatcher::AddPathWithNotify (const QString& path)
	{
		if (!AddPath (path))
			return;

		emit dirWasCreated (path);
		auto pathes = Utils::ScanDir (QDir::Dirs | QDir::NoDotAndDotDot,
				path,
				true);
		for (const auto& p : pathes)
		{
			if (!AddPath (p))
				continue;

			emit dirWasCreated (p);
		}

		pathes = Utils::ScanDir (QDir::AllEntries | QDir::NoDotAndDotDot,
				path,
				true);
		for (const auto& p : pathes)
			if (!QFileInfo (p).isDir ())
				emit fileWasCreated (p);
	}

	bool FilesWatcher::IsInExceptionList (const QString& path) const
	{
		if (ExceptionMasks_.isEmpty ())
			return false;

		for (const auto& mask : ExceptionMasks_)
		{
			QRegExp rx (mask, Qt::CaseInsensitive, QRegExp::WildcardUnix);
			if (rx.exactMatch (path))
			{
				qDebug () << "entry with name"
						<< QFileInfo (path).fileName ()
						<< "was ignored by "
						<< mask;
				return true;
			}
		}

		return false;
	}

	void FilesWatcher::RemoveWatchingPath (int descriptor)
	{
		inotify_rm_watch (INotifyDescriptor_, descriptor);
		WatchedPathes2Descriptors_.right.erase (descriptor);
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