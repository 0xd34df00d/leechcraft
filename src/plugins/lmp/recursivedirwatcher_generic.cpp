/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recursivedirwatcher_generic.h"
#include <QFileSystemWatcher>
#include <QStringList>
#include <QDir>
#include <QtConcurrentRun>
#include <util/threads/futures.h>

namespace LC
{
namespace LMP
{
	namespace
	{
		QStringList CollectSubdirs (const QString& path)
		{
			QStringList result { path };

			QDir dir { path };
			for (const auto& item : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
				result += CollectSubdirs (dir.filePath (item));

			return result;
		}
	}

	RecursiveDirWatcherImpl::RecursiveDirWatcherImpl (QObject *parent)
	: QObject { parent }
	, Watcher_ { new QFileSystemWatcher { this } }
	{
		connect (Watcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SIGNAL (directoryChanged (QString)));
	}

	void RecursiveDirWatcherImpl::AddRoot (const QString& root)
	{
		qDebug () << Q_FUNC_INFO << "scanning" << root;
		Util::Sequence (this, QtConcurrent::run (CollectSubdirs, root)) >>
				[this, root] (const QStringList& paths)
				{
					Dir2Subdirs_ [root] = paths;
					Watcher_->addPaths (paths);
				};
	}

	void RecursiveDirWatcherImpl::RemoveRoot (const QString& root)
	{
		Watcher_->removePaths (Dir2Subdirs_.take (root));
	}
}
}
