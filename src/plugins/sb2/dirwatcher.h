/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDir>
#include <QSet>

class QFileSystemWatcher;
class QUrl;

namespace LC
{
namespace SB2
{
	class DirWatcher : public QObject
	{
		Q_OBJECT

		const QDir Watched_;
		QFileSystemWatcher * const Watcher_;

		bool NotifyScheduled_ = false;

		QSet<QString> LastQuarksList_;
	public:
		DirWatcher (const QDir&, QObject* = 0);
	private slots:
		void handleDirectoryChanged ();
		void notifyChanges ();
	signals:
		void quarksAdded (const QList<QUrl>&);
		void quarksRemoved (const QList<QUrl>&);
	};
}
}
