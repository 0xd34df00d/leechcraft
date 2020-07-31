/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "riexintegrator.h"
#include "clientconnection.h"
#include "riexintegrator.h"
#include "glooxaccount.h"
#include "xeps/riexmanager.h"

namespace LC::Azoth::Xoox
{
	RIEXIntegrator::RIEXIntegrator (RIEXManager& mgr, GlooxAccount& acc, QObject *parent)
	: QObject { parent }
	, Mgr_ { mgr }
	, Acc_ { acc }
	{
		connect (&mgr,
				&RIEXManager::gotItems,
				this,
				&RIEXIntegrator::HandleRIEX);
	}

	void RIEXIntegrator::HandleRIEX (const QString& msgFrom, const QList<RIEXItem>& items, const QString& body)
	{
		auto [jid, resource] = ClientConnection::Split (msgFrom);
		if (!items.isEmpty ())
			Acc_.riexItemsSuggested (items, Acc_.GetClientConnection ()->GetCLEntry (jid), body);
	}

	void RIEXIntegrator::SuggestItems (const QList<RIEXItem>& items, QObject *to, const QString& message)
	{
		const auto entry = qobject_cast<EntryBase*> (to);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< to
					<< "to EntryBase";
			return;
		}

		QList<RIEXItem> add;
		QList<RIEXItem> del;
		QList<RIEXItem> modify;
		for (const auto& item : items)
		{
			switch (item.Action_)
			{
			case RIEXItem::AAdd:
				add << RIEXItem { RIEXItem::AAdd, item.ID_, item.Nick_, item.Groups_ };
				break;
			case RIEXItem::ADelete:
				del << RIEXItem { RIEXItem::ADelete, item.ID_, item.Nick_, item.Groups_ };
				break;
			case RIEXItem::AModify:
				modify << RIEXItem { RIEXItem::AModify, item.ID_, item.Nick_, item.Groups_ };
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown action"
						<< item.Action_
						<< "for item"
						<< item.ID_;
				break;
			}
		}

		if (!add.isEmpty ())
			Mgr_.SuggestItems (entry, add, message);
		if (!modify.isEmpty ())
			Mgr_.SuggestItems (entry, modify, message);
		if (!del.isEmpty ())
			Mgr_.SuggestItems (entry, del, message);
	}

}
