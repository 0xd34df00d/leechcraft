/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012	Georg Rudoy
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "fileswatcherbase.h"

namespace LC
{
namespace NetStoreManager
{
	class FilesWatcherDummy : public FilesWatcherBase
	{
		Q_OBJECT
	public:
		FilesWatcherDummy (QObject* = 0);
	public slots:
		void updatePaths (const QStringList& paths);
		void checkNotifications ();
		void release ();
		void updateExceptions (QStringList masks);
	};

	typedef FilesWatcherDummy FilesWatcher;
}
}
