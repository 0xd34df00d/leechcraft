/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace LMP
{
	class RecursiveDirWatcherImpl;

	class RecursiveDirWatcher : public QObject
	{
		Q_OBJECT

		RecursiveDirWatcherImpl * const Impl_;
	public:
		RecursiveDirWatcher (QObject* = nullptr);

		void AddRoot (const QString&);
		void RemoveRoot (const QString&);
	signals:
		void directoryChanged (const QString&);
	};
}
}
