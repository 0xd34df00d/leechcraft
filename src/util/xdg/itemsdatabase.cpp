/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsdatabase.h"
#include <QFileSystemWatcher>
#include <QTimer>
#include "itemtypes.h"

namespace LC
{
namespace Util
{
namespace XDG
{
	ItemsDatabase::ItemsDatabase (const ICoreProxy_ptr& proxy, const QList<Type>& types, QObject *parent)
	: ItemsFinder { proxy, types, parent }
	{
		const auto watcher = new QFileSystemWatcher { this };
		watcher->addPaths (ToPaths (types));
		connect (watcher,
				&QFileSystemWatcher::directoryChanged,
				this,
				[this]
				{
					if (UpdateScheduled_)
						return;

					UpdateScheduled_ = true;
					QTimer::singleShot (10000,
							this,
							[this]
							{
								UpdateScheduled_ = false;
								Update ();
							});
				});
	}
}
}
}
