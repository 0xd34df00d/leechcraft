/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QStringList>

class QFileSystemWatcher;
class QTimer;

namespace LC
{
namespace LMP
{
	class RecursiveDirWatcher;

	class LocalCollectionWatcher : public QObject
	{
		Q_OBJECT

		RecursiveDirWatcher * const Watcher_;

		QList<QString> ScheduledDirs_;
		QTimer * const ScanTimer_;
	public:
		LocalCollectionWatcher (QObject* = nullptr);

		void AddPath (const QString&);
		void RemovePath (const QString&);
	private:
		void ScheduleDir (const QString&);
	private slots:
		void handleDirectoryChanged (const QString&);
		void rescanQueue ();
	};
}
}
