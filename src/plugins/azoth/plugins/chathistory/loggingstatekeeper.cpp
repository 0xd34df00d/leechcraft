/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loggingstatekeeper.h"
#include <QSettings>
#include <QCoreApplication>
#include <QStringList>
#include <QtDebug>
#include <util/sll/containerconversions.h>
#include <interfaces/azoth/iclentry.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	LoggingStateKeeper::LoggingStateKeeper ()
	{
		LoadDisabled ();
	}

	bool LoggingStateKeeper::IsLoggingEnabled (ICLEntry *entry) const
	{
		return !DisabledIDs_.contains (entry->GetEntryID ());
	}

	void LoggingStateKeeper::SetLoggingEnabled (ICLEntry *entry, bool enable)
	{
		const auto& id = entry->GetEntryID ();
		if (enable)
			DisabledIDs_.remove (id);
		else
			DisabledIDs_ << id;

		SaveDisabled ();
	}

	void LoggingStateKeeper::LoadDisabled ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_ChatHistory");
		DisabledIDs_ = Util::AsSet (settings.value ("DisabledIDs").toStringList ());
	}

	void LoggingStateKeeper::SaveDisabled ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_ChatHistory");
		settings.setValue ("DisabledIDs", QStringList (DisabledIDs_.values ()));
	}
}
}
}
