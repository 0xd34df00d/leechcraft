/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recursivedirwatcher.h"

#ifdef Q_OS_MAC
#include "recursivedirwatcher_mac.h"
#else
#include "recursivedirwatcher_generic.h"
#endif

namespace LC
{
namespace LMP
{
	RecursiveDirWatcher::RecursiveDirWatcher (QObject *parent)
	: QObject { parent }
	, Impl_ { new RecursiveDirWatcherImpl { this } }
	{
		connect (Impl_,
				SIGNAL (directoryChanged (QString)),
				this,
				SIGNAL (directoryChanged (QString)));
	}

	void RecursiveDirWatcher::AddRoot (const QString& root)
	{
		Impl_->AddRoot (root);
	}

	void RecursiveDirWatcher::RemoveRoot (const QString& root)
	{
		Impl_->RemoveRoot (root);
	}
}
}
