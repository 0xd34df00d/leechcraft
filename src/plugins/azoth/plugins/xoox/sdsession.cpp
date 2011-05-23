/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "sdsession.h"
#include <boost/bind.hpp>
#include <QStandardItemModel>
#include <QtDebug>
#include <QXmppDiscoveryIq.h>
#include "glooxaccount.h"
#include "clientconnection.h"
#include "sdmodel.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	SDSession::SDSession (GlooxAccount *account)
	: Model_ (new SDModel (this))
	, Account_ (account)
	{
	}
	
	namespace
	{
		template<typename T>
		QList<QStandardItem*> AppendRow (T *parent,
				const QStringList& strings,
				const QString& jid,
				const QString& node)
		{
			QList<QStandardItem*> items;
			Q_FOREACH (const QString& string, strings)
			{
				QStandardItem *item = new QStandardItem (string);
				items << item;
				item->setEditable (false);
			}
			items.at (0)->setData (jid, SDSession::DRJID);
			items.at (0)->setData (node, SDSession::DRNode);
			parent->appendRow (items);
			return items;
		}
	}
	
	void SDSession::SetQuery (const QString& query)
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("JID") << tr ("Node"));

		QList<QStandardItem*> items = AppendRow (Model_,
				QStringList (query) << query << "",
				query,
				"");
		JID2Node2Item_ [query] [""] = items.at (0);
		
		items.at (0)->setData (true, DRFetchedMore);

		Account_->GetClientConnection ()->RequestItems (query,
				boost::bind (&SDSession::HandleItems, this, _1));
	}
	
	QAbstractItemModel* SDSession::GetRepresentationModel () const
	{
		return Model_;
	}
	
	void SDSession::HandleInfo (const QXmppDiscoveryIq& iq)
	{
		QStandardItem *item = JID2Node2Item_ [iq.from ()] [iq.queryNode ()];;
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no parent node for"
					<< iq.from ();
			return;
		}
		
		if (iq.identities ().size () != 1)
			return;

		const QString& text = iq.identities ().at (0).name ();
		if (text.isEmpty ())
			return;

		const QModelIndex& index = item->index ();
		const QModelIndex& sibling = index.sibling (index.row (), CName);
		Model_->itemFromIndex (sibling)->setText (text);
	}
	
	void SDSession::HandleItems (const QXmppDiscoveryIq& iq)
	{
		QStandardItem *parentItem = JID2Node2Item_ [iq.from ()] [iq.queryNode ()];
		if (!parentItem)
		{
			qWarning () << Q_FUNC_INFO
					<< "no parent node for"
					<< iq.from ();
			return;
		}
		
		Q_FOREACH (const QXmppDiscoveryIq::Item& item, iq.items ())
		{
			QList<QStandardItem*> items = AppendRow (parentItem,
					QStringList (item.name ()) << item.jid () << item.node (),
					item.jid (),
					item.node ());
			JID2Node2Item_ [item.jid ()] [item.node ()] = items.at (0);
			
			Account_->GetClientConnection ()->RequestInfo (item.jid (),
					boost::bind (&SDSession::HandleInfo, this, _1), item.node ());
		}
	}
	
	void SDSession::QueryItem (QStandardItem *item)
	{
		item->setData (true, DRFetchedMore);

		const QString& jid = item->data (DRJID).toString ();
		const QString& node = item->data (DRNode).toString ();
		Account_->GetClientConnection ()->RequestItems (jid,
				boost::bind (&SDSession::HandleItems, this, _1), node);
	}
}
}
}
