/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsdatabase.h"
#include <QFileSystemWatcher>
#include <QSet>
#include <QStringList>
#include <util/sll/delayedexecutor.h>
#include "itemtypes.h"

namespace LC
{
namespace Util
{
namespace XDG
{
	ItemsDatabase::ItemsDatabase (ICoreProxy_ptr proxy, const QList<Type>& types, QObject *parent)
	: ItemsFinder { proxy, types, parent }
	, Watcher_ { new QFileSystemWatcher { this } }
	{
		Watcher_->addPaths (ToPaths (types));
		connect (Watcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (scheduleUpdate ()));
	}

	void ItemsDatabase::scheduleUpdate ()
	{
		if (UpdateScheduled_)
			return;

		UpdateScheduled_ = true;
		Util::ExecuteLater ([this]
				{
					UpdateScheduled_ = false;
					update ();
				},
				10000);
	}
}
}
}
