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

class QFileSystemWatcher;

namespace LC
{
namespace LMP
{
	class RecursiveDirWatcherImpl : public QObject
	{
		Q_OBJECT

		QFileSystemWatcher * const Watcher_;
		QHash<QString, QStringList> Dir2Subdirs_;
	public:
		RecursiveDirWatcherImpl (QObject*);

		void AddRoot (const QString&);
		void RemoveRoot (const QString&);
	signals:
		void directoryChanged (const QString&);
	};
}
}
