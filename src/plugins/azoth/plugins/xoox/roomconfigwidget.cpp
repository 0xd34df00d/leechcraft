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
	, PermsModel_ (new QStandardItemModel)
	, Aff2Cat_ (InitModel ())
	, MUCManager_ (0)
	{
		Ui_.setupUi (this);
		Ui_.PermsTree_->setModel (PermsModel_);

		GlooxAccount *acc = qobject_cast<GlooxAccount*> (room->GetParentAccount ());
		MUCManager_ = acc->GetClientConnection ()->GetMUCManager ();
		connect (MUCManager_,
				SIGNAL (roomConfigurationReceived (const QString&, const QXmppDataForm&)),
				this,
				SLOT (handleConfigurationReceived (const QString&, const QXmppDataForm&)));
		connect (MUCManager_,
				SIGNAL (roomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)),
				this,
				SLOT (handlePermsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)));
		MUCManager_->requestRoomConfiguration (JID_);
		MUCManager_->requestRoomPermissions (JID_);
	}
	
	QMap<QXmppMucAdminIq::Item::Affiliation, QStandardItem*> RoomConfigWidget::InitModel () const
	{
		PermsModel_->clear ();
		PermsModel_->setHorizontalHeaderLabels (QStringList ("JID") << tr ("Reason"));
		QMap<QXmppMucAdminIq::Item::Affiliation, QStandardItem*> aff2cat;
		aff2cat [QXmppMucAdminIq::Item::OutcastAffiliation] = new QStandardItem (tr ("Banned"));
		aff2cat [QXmppMucAdminIq::Item::MemberAffiliation] = new QStandardItem (tr ("Members"));
		aff2cat [QXmppMucAdminIq::Item::AdminAffiliation] = new QStandardItem (tr ("Admins"));
		aff2cat [QXmppMucAdminIq::Item::OwnerAffiliation] = new QStandardItem (tr ("Owners"));
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
	
	void RoomConfigWidget::SendItem (const QXmppMucAdminIq::Item& item)
	{
		QList<QXmppMucAdminIq::Item> items;
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
		MUCManager_->setRoomConfiguration (JID_, form);
	}
	
	void RoomConfigWidget::on_AddPerm__released ()
	{
		std::auto_ptr<AffiliationSelectorDialog> dia (new AffiliationSelectorDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;
		
		const QString& jid = dia->GetJID ();
		if (jid.isEmpty ())
			return;

		QXmppMucAdminIq::Item item;
		item.setJid (jid);
		item.setAffiliation (dia->GetAffiliation ());
		SendItem (item);
		
		handlePermsReceived (JID_, QList<QXmppMucAdminIq::Item> () << item);
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
		
		const QXmppMucAdminIq::Item::Affiliation aff = Aff2Cat_.key (parent);		
		const QString& jid = stdItem->text ();

		std::auto_ptr<AffiliationSelectorDialog> dia (new AffiliationSelectorDialog (this));
		dia->SetJID (jid);
		dia->SetAffiliation (aff);
		if (dia->exec () != QDialog::Accepted)
			return;
		
		const QString& newJid = dia->GetJID ();
		if (newJid.isEmpty ())
			return;
		
		parent->removeRow (stdItem->row ());
		
		QXmppMucAdminIq::Item item;
		item.setJid (newJid);
		item.setAffiliation (dia->GetAffiliation ());
		SendItem (item);
		
		handlePermsReceived (JID_, QList<QXmppMucAdminIq::Item> () << item);
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
		
		QXmppMucAdminIq::Item item;
		item.setJid (jid);
		item.setAffiliation (QXmppMucAdminIq::Item::NoAffiliation);
		SendItem (item);
	}
	
	void RoomConfigWidget::handleConfigurationReceived (const QString& jid,
			const QXmppDataForm& form)
	{
		if (jid != JID_)
			return;

		FormWidget_ = FB_->CreateForm (form);
		Ui_.ScrollArea_->setWidget (FormWidget_);
		emit dataReady ();
	}
	
	void RoomConfigWidget::handlePermsReceived (const QString& jid,
			const QList<QXmppMucAdminIq::Item>& perms)
	{
		if (jid != JID_)
			return;

		Q_FOREACH (const QXmppMucAdminIq::Item& perm, perms)
		{
			QStandardItem *parentItem = Aff2Cat_ [perm.affiliation ()];
			if (!parentItem)
				continue;
			
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
