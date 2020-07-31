/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recentmanager.h"
#include <QCoreApplication>
#include <QSettings>

namespace LC
{
namespace Launchy
{
	RecentManager::RecentManager (QObject *parent)
	: QObject (parent)
	{
		Load ();
	}

	bool RecentManager::HasRecents () const
	{
		return !RecentList_.isEmpty ();
	}

	bool RecentManager::IsRecent (const QString& item) const
	{
		return RecentList_.contains (item);
	}

	int RecentManager::GetRecentOrder (const QString& item) const
	{
		return RecentList_.indexOf (item);
	}

	void RecentManager::AddRecent (const QString& item)
	{
		RecentList_.removeAll (item);
		RecentList_.prepend (item);

		const auto maxSize = 32;
		if (RecentList_.size () > maxSize)
			RecentList_.erase (RecentList_.begin () + 32, RecentList_.end ());

		Save ();

		emit recentListChanged ();
	}

	void RecentManager::Save () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Launchy");
		settings.beginGroup ("Recent");
		settings.setValue ("IDs", RecentList_);
		settings.endGroup ();
	}

	void RecentManager::Load ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Launchy");
		settings.beginGroup ("Recent");
		RecentList_ = settings.value ("IDs").toStringList ();
		settings.endGroup ();
	}
}
}
