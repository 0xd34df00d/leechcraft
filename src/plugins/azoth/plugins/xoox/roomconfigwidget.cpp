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

#include "roomconfigwidget.h"
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QXmppMucManager.h>
#include "roomclentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"
#include "formbuilder.h"
#include "affiliationselectordialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomConfigWidget::RoomConfigWidget (RoomCLEntry *room, QWidget *widget)
	: QWidget (widget)
	, FormWidget_ (0)
	, FB_ (new FormBuilder)
	, Room_ (room)
	, JID_ (room->GetRoomHandler ()->GetRoomJID ())
	, RoomHandler_ (0)
	, PermsModel_ (new QStandardItemModel)
	, Aff2Cat_ (InitModel ())
	{
		Ui_.setupUi (this);
		Ui_.PermsTree_->setModel (PermsModel_);

		GlooxAccount *acc = qobject_cast<GlooxAccount*> (room->GetParentAccount ());
		QXmppMucManager *mgr = acc->GetClientConnection ()->GetMUCManager ();

		RoomHandler_ = mgr->addRoom (JID_);
		connect (RoomHandler_,
				SIGNAL (configurationReceived (const QXmppDataForm&)),
				this,
				SLOT (handleConfigurationReceived (const QXmppDataForm&)));
		connect (RoomHandler_,
				SIGNAL (permissionsReceived (const QList<QXmppMucItem>&)),
				this,
				SLOT (handlePermsReceived (const QList<QXmppMucItem>&)));
		RoomHandler_->requestConfiguration ();
		RoomHandler_->requestPermissions ();
	}

	QMap<QXmppMucItem::Affiliation, QStandardItem*> RoomConfigWidget::InitModel () const
	{
		PermsModel_->clear ();
		PermsModel_->setHorizontalHeaderLabels (QStringList ("JID") << tr ("Reason"));
		QMap<QXmppMucItem::Affiliation, QStandardItem*> aff2cat;
		aff2cat [QXmppMucItem::OutcastAffiliation] = new QStandardItem (tr ("Banned"));
		aff2cat [QXmppMucItem::MemberAffiliation] = new QStandardItem (tr ("Members"));
		aff2cat [QXmppMucItem::AdminAffiliation] = new QStandardItem (tr ("Admins"));
		aff2cat [QXmppMucItem::OwnerAffiliation] = new QStandardItem (tr ("Owners"));
		Q_FOREACH (QStandardItem *item, aff2cat.values ())
		{
			QList<QStandardItem*> rootItems;
			rootItems << item;
			rootItems << new QStandardItem (tr ("Reason"));
			Q_FOREACH (QStandardItem *t, rootItems)
				t->setEditable (false);
			PermsModel_->appendRow (rootItems);
		}
		return aff2cat;
	}

	void RoomConfigWidget::SendItem (const QXmppMucItem& item)
	{
		QList<QXmppMucItem> items;
		items << item;
		QXmppMucAdminIq iq;
		iq.setTo (JID_);
		iq.setType (QXmppIq::Set);
		iq.setItems (items);

		GlooxAccount *account = qobject_cast<GlooxAccount*> (Room_->GetParentAccount ());
		account->GetClientConnection ()->GetClient ()->sendPacket (iq);
	}

	QStandardItem* RoomConfigWidget::GetCurrentItem () const
	{
		const QModelIndex& index = Ui_.PermsTree_->currentIndex ();
		if (!index.isValid ())
			return 0;

		const QModelIndex& sibling = index.sibling (index.row (), 0);
		return PermsModel_->itemFromIndex (sibling);
	}

	void RoomConfigWidget::accept ()
	{
		QXmppDataForm form = FB_->GetForm ();
		form.setType (QXmppDataForm::Submit);
		RoomHandler_->setConfiguration (form);
	}

	void RoomConfigWidget::on_AddPerm__released ()
	{
		std::auto_ptr<AffiliationSelectorDialog> dia (new AffiliationSelectorDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;

		const QString& jid = dia->GetJID ();
		if (jid.isEmpty ())
			return;

		QXmppMucItem item;
		item.setJid (jid);
		item.setAffiliation (dia->GetAffiliation ());
		SendItem (item);

		handlePermsReceived (QList<QXmppMucItem> () << item);
	}

	void RoomConfigWidget::on_ModifyPerm__released ()
	{
		QStandardItem *stdItem = GetCurrentItem ();
		if (!stdItem)
			return;

		QStandardItem *parent = stdItem->parent ();
		if (!Aff2Cat_.values ().contains (parent))
		{
			qWarning () << Q_FUNC_INFO
					<< "bad parent"
					<< parent
					<< "for"
					<< stdItem;
			return;
		}

		const QXmppMucItem::Affiliation aff = Aff2Cat_.key (parent);
		const QString& jid = stdItem->text ();

		std::unique_ptr<AffiliationSelectorDialog> dia (new AffiliationSelectorDialog (this));
		dia->SetJID (jid);
		dia->SetAffiliation (aff);
		if (dia->exec () != QDialog::Accepted)
			return;

		const QString& newJid = dia->GetJID ();
		if (newJid.isEmpty ())
			return;

		parent->removeRow (stdItem->row ());

		QXmppMucItem item;
		item.setJid (newJid);
		item.setAffiliation (dia->GetAffiliation ());
		SendItem (item);

		if (item.affiliation () != QXmppMucItem::NoAffiliation)
			handlePermsReceived (QList<QXmppMucItem> () << item);
	}

	void RoomConfigWidget::on_RemovePerm__released ()
	{
		QStandardItem *stdItem = GetCurrentItem ();
		if (!stdItem)
			return;

		const QString& jid = stdItem->text ();
		if (jid.isEmpty ())
			return;

		QStandardItem *parent = stdItem->parent ();
		if (!parent)
			return;

		parent->removeRow (stdItem->row ());

		QXmppMucItem item;
		item.setJid (jid);
		item.setAffiliation (QXmppMucItem::NoAffiliation);
		SendItem (item);
	}

	void RoomConfigWidget::handleConfigurationReceived (const QXmppDataForm& form)
	{
		if (sender () != RoomHandler_)
			return;

		FormWidget_ = FB_->CreateForm (form);
		Ui_.ScrollArea_->setWidget (FormWidget_);
		emit dataReady ();
	}

	void RoomConfigWidget::handlePermsReceived (const QList<QXmppMucItem>& perms)
	{
		if (qobject_cast<QXmppMucRoom*> (sender ()) &&
				sender () != RoomHandler_)
			return;

		Q_FOREACH (const QXmppMucItem& perm, perms)
		{
			QStandardItem *parentItem = Aff2Cat_ [perm.affiliation ()];
			if (!parentItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "no parent item for"
						<< perm.affiliation ();
				continue;
			}

			QList<QStandardItem*> items;
			items << new QStandardItem (perm.jid ());
			items << new QStandardItem (perm.reason ());
			Q_FOREACH (QStandardItem *item, items)
				item->setEditable (false);
			parentItem->appendRow (items);
		}

		Q_FOREACH (QStandardItem *item, Aff2Cat_.values ())
		{
			item->sortChildren (0);
			Ui_.PermsTree_->expand (item->index ());
		}
	}
}
}
}
