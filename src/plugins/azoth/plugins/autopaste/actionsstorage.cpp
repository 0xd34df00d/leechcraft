/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionsstorage.h"
#include <QStringList>
#include <QAction>

namespace LC::Azoth::Autopaste
{
	QList<QAction*> ActionsStorage::GetEntryActions (QObject *entry)
	{
		if (Entry2Actions_.contains (entry))
			return Entry2Actions_.value (entry);

		connect (entry,
				&QObject::destroyed,
				this,
				[this, entry] { Entry2Actions_.remove (entry); });

		const auto paste = new QAction { tr ("Paste to pastebin..."), entry };
		paste->setProperty ("ActionIcon", "edit-paste");
		paste->setProperty ("Azoth/Autopaste/Areas", QStringList { "toolbar" });

		connect (paste,
				&QAction::triggered,
				this,
				[this, entry] { emit pasteRequested (entry); });

		const QList<QAction*> list { paste };
		Entry2Actions_ [entry] = list;
		return list;
	}

	QStringList ActionsStorage::GetActionAreas (const QObject *action) const
	{
		return action->property ("Azoth/Autopaste/Areas").toStringList ();
	}
}
