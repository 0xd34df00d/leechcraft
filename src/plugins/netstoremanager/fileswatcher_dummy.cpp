/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fileswatcher_dummy.h"
#include <QStringList>

namespace LC
{
namespace NetStoreManager
{
	FilesWatcherDummy::FilesWatcherDummy (QObject *parent)
	: FilesWatcherBase (parent)
	{
	}

	void FilesWatcherDummy::updatePaths(const QStringList &paths)
	{

	}
	void FilesWatcherDummy::checkNotifications ()
	{
	}

	void FilesWatcherDummy::release ()
	{
	}

	void FilesWatcherDummy::updateExceptions (QStringList)
	{
	}
}
}
