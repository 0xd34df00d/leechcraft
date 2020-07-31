/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtoblockedrunner.h"
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include "clientconnection.h"
#include "xeps/privacylistsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	AddToBlockedRunner::AddToBlockedRunner (const QStringList& ids,
			const ClientConnection_ptr& conn, QObject *parent)
	: QObject { parent }
	, Ids_ { ids }
	, Conn_ { conn }
	{
		Conn_->GetPrivacyListsManager ()->QueryLists ({
					[this] (const QXmppIq&) { deleteLater (); },
					Util::BindMemFn (&AddToBlockedRunner::HandleGotLists, this)
				});
	}

	void AddToBlockedRunner::HandleGotLists (const QStringList&,
			const QString& active, const QString& def)
	{
		bool activate = false;
		QString listName;
		if (!active.isEmpty ())
			listName = active;
		else if (!def.isEmpty ())
			listName = def;
		else
		{
			listName = "default";
			activate = true;
		}

		FetchList (listName, activate);
	}

	void AddToBlockedRunner::FetchList (const QString& listName, bool activate)
	{
		Conn_->GetPrivacyListsManager ()->QueryList (listName,
				{
					[=] (const QXmppIq&) { AddToList (listName, {}, activate); },
					[=] (const PrivacyList& list) { AddToList (listName, list, activate); }
				});
	}

	void AddToBlockedRunner::AddToList (const QString& name, PrivacyList list, bool activate)
	{
		deleteLater ();

		if (list.GetName ().isEmpty ())
			list.SetName (name);

		auto items = list.GetItems ();

		const auto& presentIds = Util::MapAs<QSet> (items, &PrivacyListItem::GetValue);

		bool modified = false;
		for (const auto& id : Ids_)
		{
			if (presentIds.contains (id))
				continue;

			items.prepend ({ id, PrivacyListItem::Type::Jid });
			modified = true;
		}

		if (!modified)
			return;

		list.SetItems (items);

		const auto plm = Conn_->GetPrivacyListsManager ();
		plm->SetList (list);
		if (activate)
			plm->ActivateList (list.GetName (), PrivacyListsManager::ListType::Default);
	}
}
}
}
