/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012	Georg Rudoy
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace NetStoreManager
{
	class FilesWatcherBase : public QObject
	{
		Q_OBJECT
	public:
		FilesWatcherBase (QObject* = 0);

	public slots:
		virtual void updatePaths (const QStringList& paths) = 0;
		virtual void checkNotifications () = 0;
		virtual void release () = 0;
		virtual void updateExceptions (QStringList masks) = 0;

	signals:
		void dirWasCreated (const QString& path);
		void fileWasCreated (const QString& path);
		void dirWasRemoved (const QString& path);
		void fileWasRemoved (const QString& path);
		void fileWasUpdated (const QString& path);
		void entryWasRenamed (const QString& oldName, const QString& newName);
		void entryWasMoved (const QString& oldPath, const QString& newPath);
	};
}
}
