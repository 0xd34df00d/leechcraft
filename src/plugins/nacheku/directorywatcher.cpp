/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "directorywatcher.h"
#include <QDir>
#include <QTimer>
#include <QUrl>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Nacheku
{
	DirectoryWatcher::DirectoryWatcher (IEntityManager *iem, QObject *parent)
	: QObject { parent }
	, IEM_ { iem }
	, Watcher_ { std::make_unique<QFileSystemWatcher> () }
	{
		XmlSettingsManager::Instance ().RegisterObject ("WatchDirectory",
				this,
				"settingsChanged");

		QTimer::singleShot (5000,
				this,
				SLOT (settingsChanged ()));

		connect (Watcher_.get (),
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (handleDirectoryChanged (QString)),
				Qt::QueuedConnection);
	}

	void DirectoryWatcher::settingsChanged ()
	{
		const QString& path = XmlSettingsManager::Instance ()
				.property ("WatchDirectory").toString ();
		const QStringList& dirs = Watcher_->directories ();
		if (dirs.size () == 1 &&
				dirs.at (0) == path)
			return;

		if (!dirs.isEmpty ())
			Watcher_->removePaths (dirs);

		if (!path.isEmpty ())
		{
			QDir dir (path);
			Olds_ = dir.entryInfoList (QDir::Files);

			Watcher_->addPath (path);
			handleDirectoryChanged (path);
		}
	}

	void DirectoryWatcher::handleDirectoryChanged (const QString& path)
	{
		const auto& cur = QDir { path }.entryInfoList (QDir::Files);
		auto nl = cur;

		for (const auto& oldFi : Olds_)
		{
			const auto& fname = oldFi.absoluteFilePath ();
			for (const auto& newFi : nl)
				if (newFi.absoluteFilePath () == fname)
				{
					nl.removeOne (newFi);
					break;
				}
		}

		Olds_ = cur;

		for (const auto& newFi : nl)
			IEM_->HandleEntity (Util::MakeEntity (QUrl::fromLocalFile (newFi.absoluteFilePath ()),
						path,
						FromUserInitiated));
	}
}
}

