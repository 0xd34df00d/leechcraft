/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dirwatcher.h"
#include <QFileSystemWatcher>
#include <QTimer>
#include <QUrl>

namespace LC::SB2
{
	namespace
	{
		QSet<QString> ScanQuarks (const QDir& dir)
		{
			QSet<QString> result;
			for (const auto& entry : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
			{
				QDir quarkDir { dir };
				quarkDir.cd (entry);
				if (quarkDir.exists (entry + ".qml"))
					result << entry;
			}
			return result;
		}
	}

	DirWatcher::DirWatcher (const QDir& dir, QObject *parent)
	: QObject { parent }
	, Watched_ { dir }
	, Watcher_ { new QFileSystemWatcher (parent) }
	, LastQuarksList_ { ScanQuarks (dir) }
	{
		Watcher_->addPath (Watched_.absolutePath ());

		connect (Watcher_,
				&QFileSystemWatcher::directoryChanged,
				this,
				[this]
				{
					if (NotifyScheduled_)
						return;

					QTimer::singleShot (1000,
							this,
							&DirWatcher::NotifyChanges);
					NotifyScheduled_ = true;
				});
	}

	void DirWatcher::NotifyChanges ()
	{
		NotifyScheduled_ = false;

		auto current = ScanQuarks (Watched_);
		const auto& newQuarks = current - LastQuarksList_;
		const auto& removedQuarks = LastQuarksList_ - current;
		LastQuarksList_.swap (current);

		auto toUrlList = [this] (const QSet<QString>& filenames) -> QList<QUrl>
		{
			QList<QUrl> result;
			for (const auto& name : filenames)
				result << QUrl::fromLocalFile (Watched_.absoluteFilePath (name + '/' + name + ".qml"));
			return result;
		};

		if (!newQuarks.isEmpty ())
			emit quarksAdded (toUrlList (newQuarks));
		if (!removedQuarks.isEmpty ())
			emit quarksRemoved (toUrlList (removedQuarks));
	}
}
